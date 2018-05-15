#include <string.h>
#include <stdlib.h>
#include "task_type.h"
#include "defs.h"
#include "memory.h"
#include "service.h"
#include "event.h"
#include "systimer.h"
#include "task_control.h"
#include "scheduler.h"
#include "assertions.h"
#include "os_printf.h"


static void task_end(void);
static TASK_STATIC_CREATE(main_task, "Main", MAIN_STACK_SIZE, 0);

static task_t * task_alloc(int stack_size)
{
	// ensure that eventual sp is 8 byte aligned
	size_t size = sizeof(task_t) + stack_size + (stack_size % 8);
	task_t * t = malloc_aligned(size, 8);
	memset(t, 0, size);
	t->sp = (uint8_t*) t + size;
	t->stack_top = t->sp;
	return t;
}

static task_t * task_init(task_t *t, const char * name)
{
	*(uint32_t*) &t->signature = TASK_SIGNATURE;
	t->priority = DEFAULT_TASK_PRIORITY;
	t->state = TASK_INACTIVE;
	t->name = name;
	t->exc_return = 0xfffffffd;
	list_init(&t->node);
	event_init(&t->join, "Join");
	return t;
}

task_t * task_frame_init(task_t *t, void (*func)(void*), const void *ctx)
{
  hw_stack_frame_t *frame = (hw_stack_frame_t*)(t->sp - sizeof(hw_stack_frame_t));
  frame->r0 = (uint32_t)ctx;
  frame->pc = (uint32_t)func & 0xfffffffe;
  frame->lr = (uint32_t)&task_end;
  frame->xpsr = 0x01000000;   // thumb mode enabled (required);

  t->sp = (uint8_t*)frame;
  return t;
}

static task_t * task_create(void (*func)(void*), int stack_size, const void *ctx, const char * name)
{
	task_t * t = task_alloc(stack_size);
	task_init(t, name);
	task_frame_init(t, func, ctx);
	return t;
}

task_t * task_create_schedule(void (*func)(void*), int stack_size, void *ctx, const char * name)
{
	task_t * task = task_create(func, stack_size, ctx, name);
	task->state = TASK_READY;
	service_call((svcall_t)task_control_add, task, true);
	service_call((svcall_t)scheduler_reschedule_task, task, true);
	return task;
}

void task_start(task_t * task)
{
	assert_protected();
	assert(task && "No task");
	task_state_transition(task, TA_START);
	scheduler_reschedule_task(task);
}

void task_stop(task_t * task)
{
	assert_protected();
	assert(task && "No task");
	scheduler_unschedule_task(task);
	task_state_transition(task, TA_STOP);
}

void task_delay(uint32_t ms)
{
	event_t event = {0};
	event_init(&event, "Delay Event");
	systimer_t * timer = systimer_create_event_onetime(ms, &event);
	event_wait(&event);
	systimer_destroy(timer);
	service_call((svcall_t)event_control_remove, &event, true);
}

void task_yield(void) 
{
  kernel_context_switch();
}

static void task_end_irq(void * ctx)
{
	assert_protected();
	task_t * task = ctx;
	event_notify_irq(&task->join);
	task_state_transition(task, TA_EXIT);
	kernel_context_switch_irq(NULL);
}

static void task_end(void)
{
	task_t * current = get_current_task();
	service_call(task_end_irq, current, true);
}

static void task_remove(task_t *t)
{
	assert_protected();
	assert(!list_element(task_to_list(t)) && "Node still in some list.");
	task_control_remove(t);
	event_control_remove(&t->join);
}

void task_free(task_t * t)
{
	assert(t->id > 0 && "Cannot free idle or main task.");
	service_call((svcall_t) task_remove, (void*)t, true);

	free_aligned(t);
}

void task_join(task_t * t)
{
	event_wait(&t->join);
	task_free(t);
}

uintptr_t task_get_sp(int id)
{
  task_t * t = task_control_get(id);
  if (t) return (uintptr_t)t->sp;
  return 0;
}

uintptr_t task_get_saved_pc(int id)
{
	task_t * t = task_control_get(id);
	if (t) return ((hw_stack_frame_t*)t->sp)->pc;
	return 0;
}

hw_stack_frame_t task_get_saved_hw_frame(int id)
{
	task_t * t = task_control_get(id);
	if (t) return *((hw_stack_frame_t*)t->sp);
	return (hw_stack_frame_t){0,0,0,0,0,0,0,0};
}

sw_stack_frame_t task_get_saved_sw_frame(int id)
{
	task_t * t = task_control_get(id);
	if (t) return t->sw_context;
	return (sw_stack_frame_t){0,0,0,0,0,0,0,0};
}

// NOTE: Do Not Call from inside an IRQ
// This function switches modes from privileged to user
// Handle with care
// NOTE: NO MALLOC (or any other syscall) until after context_switch
extern uint32_t main_return_point;
void task_main_hoist(void)
{
	__disable_irq();
	// Copy current stack to new main stack
	uint32_t stack_base = *((uint32_t*) SCB->VTOR);
	uint32_t stack_ptr = kernel_SP_get();
	uint32_t stack_size = stack_base - stack_ptr;
	uint8_t *main_task_sp = &main_task.stack[0] + sizeof(main_task.stack) - stack_size;
	memcpy(main_task_sp, (void*) stack_ptr, stack_size);


	// Note: this is where the stack pointer will be once
	// we enter the context switching handler.
	// Advance main_task_sp to account for stacked/pushed regs
	uint8_t * adjusted_main_task_sp = main_task_sp - sizeof(hw_stack_frame_t);
	// Ensure that result stack is aligned on 8 byte boundary
	if ((uint32_t) adjusted_main_task_sp % 8)
	{
		adjusted_main_task_sp = adjusted_main_task_sp - 4;
	}

	// Init main task object
	main_task.task.sp = adjusted_main_task_sp;

	// Setup return HW frame
	hw_stack_frame_t * frame = (hw_stack_frame_t*) adjusted_main_task_sp;
	frame->pc = main_return_point;
	frame->xpsr = 0x01000000;

	extern task_t * current_task;
	current_task = &main_task.task;
	current_task->state = TASK_ACTIVE;
	task_control_add(&main_task.task);
	__enable_irq();


	// Set PSP to our new stack and Change mode to unprivileged
	__ISB();
	__set_PSP((uint32_t) main_task_sp);
	// Recover MSP for interrupt handles -- has to happen before mode change
	__set_MSP(stack_base);
	__ISB();
	__set_CONTROL(CONTROL_SPSEL_Msk | CONTROL_nPRIV_Msk);
	__ISB();

	// need to call start here in order to keep the SP valid
	kernel_context_switch();
	__asm volatile("main_return_point: \n");
}

void task_state_transition(task_t *t, task_action_e action)
{
	assert_protected();
	switch(t->state) {
	case TASK_INACTIVE:
		switch(action) {
		case TA_START:
			t->state = TASK_READY;
			break;
		case TA_STOP:
		case TA_CONTEXT_SWITCH:
		case TA_WAIT:
		case TA_NOTIFY:
		case TA_EXIT:
		default:
			assert(0 && "Invalid Transition");
			break;
		}

		break;
	case TASK_ACTIVE:
		switch(action) {
		case TA_STOP:
			t->state = TASK_INACTIVE;
			break;
		case TA_CONTEXT_SWITCH:
			t->state = TASK_READY;
			break;
		case TA_WAIT:
			t->state = TASK_WAIT;
			break;
		case TA_NOTIFY:
			assert(0 && "Invalid Transition");
			break;
		case TA_EXIT:
			t->state = TASK_END;
			break;
		case TA_START:
		default:
			assert(0 && "Invalid Transition");
			break;
		}

		break;
	case TASK_READY:
		switch(action) {
		case TA_START:
			assert(0 && "Invalid Transition");
			break;
		case TA_STOP:
			t->state = TASK_INACTIVE;
			break;
		case TA_CONTEXT_SWITCH:
			t->state = TASK_ACTIVE;
			break;
		case TA_WAIT:
		case TA_NOTIFY:
		case TA_EXIT:
		default:
			assert(0 && "Invalid Transition");
			break;
		}

		break;
	case TASK_WAIT:
		switch(action) {
		case TA_STOP:
			t->state = TASK_INACTIVE;
			break;
		case TA_CONTEXT_SWITCH:
			// do nothing
			break;
		case TA_NOTIFY:
			t->state = TASK_READY;
			break;
		case TA_START:
		case TA_WAIT:
		case TA_EXIT:
		default:
			assert(0 && "Invalid Transition");
			break;
		}

		break;
	case TASK_END:
		switch(action) {
		case TA_CONTEXT_SWITCH:
			// do nothing
			break;
		case TA_START:
		case TA_STOP:
		case TA_WAIT:
		case TA_NOTIFY:
		case TA_EXIT:
		default:
			assert(0 && "Invalid Transition");
			break;
		}

		break;
	default:
		assert(0 && "Invalid Transition");

	}
}

void assert_task_valid(task_t *t)
{
	task_t * current = get_current_task();

	// task must have valid signature
	assert(t->signature == TASK_SIGNATURE && "Invalid task signature.");

	bool in_active = (t->state == TASK_ACTIVE);
	bool in_ready = scheduler_task_ready(t);
	bool in_wait = event_control_task_waiting(t);

	// task state must be one of Inactive, Active, Sleep, Wait, or end
	switch(t->state) {
	case TASK_ACTIVE:
		// task must be current or in active queue
		assert((t == current && in_active) && "Invalid TASK_ACTIVE");
		// task must not be in sleep, or wait queue
		assert(!(in_ready || in_wait) && "Invalid TASK_ACTIVE");
		break;
	case TASK_READY:
		// task must be in ready
		assert(in_ready && "Invalid TASK_READY");
		// task must not be current
		assert((t != current) && "Invalid TASK_READY");
		// task must not be in active, or wait
		assert(!(in_active || in_wait) && "Invalid TASK_READY");
		break;
	case TASK_WAIT:
		// task must not be current
		assert((t != current) && "Invalid TASK_WAIT");
		// task must not be in active, or sleep queue
		assert(!(in_active || in_ready) && "Invalid TASK_WAIT");
		// task must be in ONE wait queue
		assert(in_wait == 1 && "Invalid TASK_WAIT");
		break;
	case TASK_INACTIVE:
		// task must not be current
		// task must not be in active, ready, or wait queue
	case TASK_END:
		// task must not be current
		assert((t != current) && "Invalid TASK_INACTIVE or TASK_END");
		// task must not be in active, ready, or wait queue
		assert(!(in_active || in_ready || in_wait) && "Invalid TASK_INACTIVE or TASK_END");
		break;
	default:
		assert(0 && "Invalid task state.");
		break;
	}
}

void assert_all_tasks_valid(void)
{
	task_control_each(assert_task_valid);
}

static const char default_name[] = "unnamed";
static const char *state_names[] =
{ "Inactive", "Active", "Ready", "Wait", "End" };

void task_display(task_t * task)
{
	char fmt[] = "%d\t%s\t%d/%d\t%.2f\t%s\r\n";
	const char * name = default_name;
	if (task->name)
		name = task->name;

	uint32_t usecs = usec_time();
	int stack_size = (char*) (task->stack_top)	- ((char*) task + sizeof(task_t));
	int stack_usage = (char*) task->stack_top - (char*) task->sp;
	uint32_t runtime = task->runtime;
	float runper = (((float) runtime / (float) usecs) * 100);

	os_iprintf(fmt, task->id, state_names[task->state], stack_usage,
				stack_size, runper, name);
}

