#include <string.h>
#include <stdlib.h>
#include "task_type.h"
#include "defs.h"
#include "kernel_task.h"
#include "memory.h"
#include "svc.h"
#include "event.h"
#include "systimer.h"
#include "task_control.h"

// implemented in kernel_task
extern void kernel_task_end(void);

task_t * task_alloc(int stack_size)
{
	// ensure that eventual sp is 8 byte aligned
	size_t size = sizeof(task_t) + stack_size + (stack_size % 8);
	task_t * t = malloc_aligned(size, 8);
	memset(t, 0, size);
	t->sp = (uint8_t*) t + size;
	t->stack_top = t->sp;
	return t;
}

task_t * task_init(task_t *t, const char * name, int id)
{
	*(uint32_t*) &t->signature = TASK_SIGNATURE;
	//t->id = id;
	t->name = name;
	t->exc_return = 0xfffffffd;
	list_init(&t->node);
	event_init(&t->join, "Join");
	return t;
}

task_t * task_create(int stack_size, const char * name)
{
	task_t * t = task_alloc(stack_size);
	task_init(t, name, -32);
	task_control_add(t);
	return t;
}

task_t * task_frame_init(task_t *t, void (*func)(void*), void *context)
{
  hw_stack_frame_t *frame = (hw_stack_frame_t*)(t->sp - sizeof(hw_stack_frame_t));
  frame->r0 = (uint32_t)context;
  frame->pc = (uint32_t)func & 0xfffffffe;
  frame->lr = (uint32_t)&kernel_task_end;
  frame->xpsr = 0x01000000;   // thumb mode enabled (required);

  t->sp = (uint8_t*)frame;
  return t;
}

static void __task_create_schedule(void *ctx)
{
	task_init_t *ti = ctx;
	ti->task = task_create(ti->stack_size, ti->name);
	task_frame_init(ti->task, ti->func, ti->context);
	kernel_task_start_task(ti->task);
}

task_t * task_create_schedule(void (*func)(void*), int stack_size, void *context, const char * name)
{
	task_init_t ti =
	{ .func = func, .stack_size = stack_size, .context = context, .name = name,
			.task = 0 };
	service_call(__task_create_schedule, &ti, true);
	return ti.task;
}

static void protected_task_start(void * cxt)
{
  task_t * new = cxt;
  __disable_irq();
  kernel_task_start_task(new);
  __enable_irq();
}

//inline
void task_schedule(task_t *task)
{
  service_call(protected_task_start, task, false);
}

void task_delay(uint32_t ms)
{
	event_t event = {0};
	event_init(&event, "Delay Event");
	systimer_t * timer = systimer_create_event_onetime(ms, &event);
	event_wait(&event);
	systimer_destroy(timer);
	service_call(kernel_task_event_unregister, &event, true);
}

//inline
void task_yield(void) 
{
  kernel_context_switch();
}

void task_free(task_t * t)
{
	assert(t->id > 0 && "Cannot free idle or main task.");
	service_call((svcall_t) kernel_task_destroy_task, t, true);
}

void task_join(task_t * t)
{
	event_wait(&t->join);
	task_free(t);
}
