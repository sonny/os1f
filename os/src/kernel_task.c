#include <stdint.h>
#include "kernel.h"
#include "task.h"
#include "event.h"
#include "defs.h"
#include "list.h"
#include "svc.h"
#include <string.h>
#include <assert.h>
#include "os_printf.h"

#define MAX_TASK_COUNT  32
#define IDLE_STACK_SIZE 128
#define MAIN_STACK_SIZE 1024  // default size of main stack
#define IDLE_TASK_ID    -1

static task_t *task_list[MAX_TASK_COUNT] = {0};

static task_t * current_task = NULL;
static list_t task_active = LIST_STATIC_INIT(task_active);
static list_t task_sleeping = LIST_STATIC_INIT(task_sleeping);

/* idle task */
static uint8_t idle_task_stack[IDLE_STACK_SIZE] __attribute__((aligned (8)));
static task_t idle_task = {
  .node = LIST_STATIC_INIT(idle_task.node),
  .name = "Idle",
  .sp = &idle_task_stack[0] + IDLE_STACK_SIZE,
  .stack_free = 0,
  .id = -1,
  .state = TASK_ACTIVE,
  .sleep_until = 0,
  .uses_fpu = false,
  .join = EVENT_STATIC_INIT(idle_task.join),
  .exc_return = 0xfffffffd
};

/* main task */
static uint8_t main_task_stack[MAIN_STACK_SIZE] __attribute__((aligned (8)));
static task_t main_task = {
  .node = LIST_STATIC_INIT(main_task.node),
  .name = "Main",
  .sp = &main_task_stack[0] + MAIN_STACK_SIZE,
  .stack_free = 0,
  .id = 0,
  .state = TASK_ACTIVE,
  .sleep_until = 0,
  .uses_fpu = false,
  .join = EVENT_STATIC_INIT(main_task.join),
  .exc_return = 0xfffffffd
};

static void kernel_task_main_hoist(void);
static void kernel_task_idle_func(void *c) { while (1); }

inline
void kernel_task_save_context_current(int exc_return)
{
  __asm volatile ("stmia %0, {r4-r11} \n" :: "r" (&current_task->sw_context) :);
  current_task->exc_return = exc_return;
  
  #ifdef ENABLE_FPU

  // check to see if task used FPU
  if ( !(exc_return & (1<<4)) ) {
    current_task->uses_fpu = true;
    __asm volatile ( "vstmia %0, {s16-s31} \n" :: "r" (&current_task->sw_fp_context) : );
  }
  else
    current_task->uses_fpu = false;

  #endif
}

inline
uint32_t kernel_task_load_context_current(void)
{
  __asm volatile ("ldmia %0, {r4-r11} \n" :: "r" (&current_task->sw_context) :);

  #ifdef ENABLE_FPU

  if ( current_task->uses_fpu ) {
    __asm volatile ( "vldmia %0, {s16-s31} " :: "r" (&current_task->sw_fp_context) : );
  }

  #endif

  return current_task->exc_return;
}

void kernel_task_init(void)
{
  task_stack_init(&idle_task, kernel_task_idle_func, NULL);
  kernel_task_main_hoist();
}

// NOTE: Do Not Call from inside an IRQ
// This function switches modes from privileged to user
// Handle with care
// NOTE: NO MALLOC (or any other syscall) until after syscall_start
extern uint32_t main_return_point;
static void kernel_task_main_hoist(void)
{
  // Copy current stack to new main stack
  uint32_t stack_base = *((uint32_t*)SCB->VTOR);
  uint32_t stack_ptr  = kernel_SP_get();
  uint32_t stack_size = stack_base - stack_ptr;
  void *main_task_sp = &main_task_stack[0] + MAIN_STACK_SIZE - stack_size;
  memcpy(main_task_sp, (void*)stack_ptr, stack_size);

  // Note: this is where the stack pointer will be once
  // we enter the context switching handler.
  // Advance main_task_sp to account for stacked/pushed regs
  void * adjusted_main_task_sp = main_task_sp - sizeof(hw_stack_frame_t);
  // Ensure that result stack is aligned on 8 byte boundary
  if ((uint32_t)adjusted_main_task_sp % 8) {
    adjusted_main_task_sp -= 4;
  }
  
  // Init main task object
  main_task.sp = adjusted_main_task_sp;

  // Setup return HW frame
  hw_stack_frame_t * frame = adjusted_main_task_sp;
  frame->pc = main_return_point;
  frame->xpsr = 0x01000000;

  current_task = &main_task;
  task_list[0] = &main_task;
  
  // Set PSP to our new stack and Change mode to unprivileged
  kernel_sync_barrier();
  kernel_PSP_set((uint32_t)main_task_sp);
  // Recover MSP for interrupt handles -- has to happen before mode change
  kernel_MSP_set(stack_base);
  kernel_sync_barrier();
  kernel_CONTROL_set(0x01 << 1 | 0x01 << 0); // use PSP with unprivileged thread mode
  kernel_sync_barrier();

  // need to call start here in order to keep the SP valid
  kernel_context_switch();
  __asm volatile("main_return_point: \n");
}

void kernel_task_schedule_current(void)
{
  switch(current_task->state) {
  case TASK_END:
    // do not reschedule
    break;
  case TASK_SLEEP:
    // Task is scheduled by task_sleep
    break;
  case TASK_WAIT:
    // Task is scheduled by task_event_wait
    break;
  case TASK_ACTIVE:
    list_addAtRear(&task_active, task_to_list(current_task)); 
    break;
  default:
    // invalid state
    kernel_break();
    break;
  }
}

void kernel_task_active_next_current(void)
{
  if (!list_empty(&task_active))
    current_task = list_to_task(list_removeFront(&task_active));
  else
    current_task = &idle_task;
}

void kernel_task_start_id(int id)
{
  assert(id >= 0 && task_list[id] && "Invalid task id.");
  kernel_task_start_task(task_list[id]);
}

void kernel_task_start_task(task_t * new)
{
  assert(task_list[new->id] == NULL && "Invalid task list id.");
  task_list[new->id] = new;
  new->state = TASK_ACTIVE; 
  list_addAtRear(&task_active, task_to_list(new));
}

void kernel_task_sleep_current(uint32_t ms)
{
  assert(current_task && "Current Task is NULL");
  current_task->state = TASK_SLEEP;
  current_task->sleep_until = HAL_GetTick() + ms;
  list_addAtRear(&task_sleeping, task_to_list(current_task)); 
}

void kernel_task_event_wait_current(event_t * e)
{
  assert(current_task && "Cannot Task is NULL");
  current_task->state = TASK_WAIT;
  list_addAtRear(&e->waiting, task_to_list(current_task));
}

static inline
void kernel_task_wakeup_task(list_t *node, const void * context)
{
  task_t *t = list_to_task(node);
  const uint32_t tick = (uint32_t)context;
  assert(t->state == TASK_SLEEP && "Tasks in sleeping queue must be asleep.");

  if (t->sleep_until < tick) {
    t->sleep_until = 0;
    t->state = TASK_ACTIVE;

    list_remove(node);
    list_addAtRear(&task_active, node);
  }
}

void kernel_task_wakeup_all(void)
{
  uint32_t tick = HAL_GetTick(); 
  list_each_do(&task_sleeping, kernel_task_wakeup_task, (void*)tick);
}

void kernel_task_event_notify_all(event_t * e)
{
  while(!list_empty(&e->waiting)) {
    task_t * task = list_to_task(list_removeFront(&e->waiting));
    assert(task->state == TASK_WAIT && "Tasks in waiting queue must be waiting.");
    task->state = TASK_ACTIVE;
    list_addAtRear(&task_active, task_to_list(task));
  }
}

void kernel_task_load_PSP_current(void)
{
  assert(current_task && "Current task cannot be null");
  kernel_PSP_set((uint32_t)current_task->sp);
}

void kernel_task_save_PSP_current(void)
{
  assert(current_task && "Current task cannot be null");
  current_task->sp = (void*)kernel_PSP_get();
}

uint32_t kernel_task_id_current(void)
{
  return current_task->id;
}

void kernel_task_end(void)
{
  current_task->state = TASK_END;
  event_notify(&current_task->join);
  task_yield();
}

int32_t kernel_task_next_id(void)
{
  int i = 1;
  while (i < MAX_TASK_COUNT) {
    if (task_list[i] == NULL) return i;
    ++i;
  }
  return -1;
}

void kernel_task_destroy_task(task_t *t)
{
  assert(task_list[t->id] == t && "Invalid Task Entry");
  task_list[t->id] = NULL;
}

static const char default_name[] = "unnamed";
static const char *state_names[] = {
  "Inactive",
  "Active",
  "Sleep",
  "Wait",
  "End"
};

void kernel_task_display_task_stats(void)
{
  int i;
  char fmt[] = "%d: %s - %s\n";
  os_iprintf(fmt, idle_task.id, idle_task.name, state_names[idle_task.state]);
  for (i = 0; i < MAX_TASK_COUNT; ++i) {
    task_t * t = task_list[i];
    if (t) {
      const char * name = default_name;
      if (t->name) name = t->name;
      os_iprintf(fmt, t->id, name, state_names[t->state]);
    }
  }
}
