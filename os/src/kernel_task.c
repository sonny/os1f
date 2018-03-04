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
#include "heap.h"

#define MAX_TASK_COUNT  32
#define IDLE_STACK_SIZE 128
#define MAIN_STACK_SIZE 1024  // default size of main stack
#define IDLE_TASK_ID    -1

static task_t *task_list[MAX_TASK_COUNT] = {0};

static task_t * current_task = NULL;
static list_t task_active = LIST_STATIC_INIT(task_active);
//static list_t task_sleeping = LIST_STATIC_INIT(task_sleeping);

static __attribute__((const))
heap_key_t sleeping_heap_key(const void * cxt);

static HEAP_MIN_STATIC_CREATE(sleeping, MAX_TASK_COUNT, sleeping_heap_key);

static TASK_STATIC_CREATE(idle_task, "Idle", IDLE_STACK_SIZE, IDLE_TASK_ID);
static TASK_STATIC_CREATE(main_task, "Main", MAIN_STACK_SIZE, 0);

static void kernel_task_main_hoist(void);
static void kernel_task_idle_func(void *c) { (void)c; while (1); }

inline
void kernel_task_save_context_current(int exc_return)
{
  __asm volatile ("stmia %0, {r4-r11} \n" :: "r" (&current_task->sw_context) :);
  current_task->exc_return = exc_return;
  
  #ifdef ENABLE_FPU

  // check to see if task used FPU
  if ( !(exc_return & (1<<4)) ) {
    current_task->flags |= TASK_FLAG_FPU;
    __asm volatile ( "vstmia %0, {s16-s31} \n" :: "r" (&current_task->sw_fp_context) : );
  }
  else
    current_task->flags &= ~TASK_FLAG_FPU;

  #endif
}

inline
uint32_t kernel_task_load_context_current(void)
{
  __asm volatile ("ldmia %0, {r4-r11} \n" :: "r" (&current_task->sw_context) :);

  #ifdef ENABLE_FPU

  if ( current_task->flags & TASK_FLAG_FPU ) {
    __asm volatile ( "vldmia %0, {s16-s31} " :: "r" (&current_task->sw_fp_context) : );
  }

  #endif

  return current_task->exc_return;
}

void kernel_task_update_lasttime_current(void)
{
  current_task->lasttime = usec_time();
}

void kernel_task_update_runtime_current(void)
{
  uint64_t diff = usec_time() - current_task->lasttime;
  current_task->runtime += diff;
}

void kernel_task_init(void)
{
  task_frame_init(&idle_task.task, kernel_task_idle_func, NULL);
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
  //  void *main_task_sp = &main_task_stack[0] + MAIN_STACK_SIZE - stack_size;
  uint8_t *main_task_sp = &main_task.stack[0] + sizeof(main_task.stack) - stack_size;
  memcpy(main_task_sp, (void*)stack_ptr, stack_size);

  // Note: this is where the stack pointer will be once
  // we enter the context switching handler.
  // Advance main_task_sp to account for stacked/pushed regs
  uint8_t * adjusted_main_task_sp = main_task_sp - sizeof(hw_stack_frame_t);
  // Ensure that result stack is aligned on 8 byte boundary
  if ((uint32_t)adjusted_main_task_sp % 8) {
    adjusted_main_task_sp = adjusted_main_task_sp - 4;
  }
  
  // Init main task object
  main_task.task.sp = adjusted_main_task_sp;

  // Setup return HW frame
  hw_stack_frame_t * frame = (hw_stack_frame_t*)adjusted_main_task_sp;
  frame->pc = main_return_point;
  frame->xpsr = 0x01000000;

  current_task = &main_task.task;
  task_list[0] = &main_task.task;
  
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
    current_task = &idle_task.task;
}

void kernel_task_start_id(int id)
{
  if ( id < 0 || id >= MAX_TASK_COUNT ) return;
  kernel_task_start_task(task_list[id]);
}

void kernel_task_start_task(task_t * t)
{
  if (!task_list[t->id]) task_list[t->id] = t;
  t->state = TASK_ACTIVE; 
  list_addAtRear(&task_active, task_to_list(t));
}

void kernel_task_stop_id(int id)
{
  if ( id < 0 || id >= MAX_TASK_COUNT || !task_list[id] ) return;
  kernel_task_stop_task(task_list[id]);
}

void kernel_task_stop_task(task_t * t)
{
  list_remove(task_to_list(t)); // remove from any list it might be in
  t->state = TASK_INACTIVE; 
}

void kernel_task_sleep_current(uint32_t ms)
{
  assert(current_task && "Current Task is NULL");
  current_task->state = TASK_SLEEP;
  current_task->sleep_until = HAL_GetTick() + ms;
  //list_addAtRear(&task_sleeping, task_to_list(current_task));
  Heap.insert(&sleeping.heap, current_task);
}

void kernel_task_event_wait_current(event_t * e)
{
  assert(current_task && "Cannot Task is NULL");
  current_task->state = TASK_WAIT;
  list_addAtRear(&e->waiting, task_to_list(current_task));
}

/* static inline */
/* void kernel_task_wakeup_task(list_t *node, const void * context) */
/* { */
/*   task_t *t = list_to_task(node); */
/*   const uint32_t tick = (uint32_t)context; */
/*   assert(t->state == TASK_SLEEP && "Tasks in sleeping queue must be asleep."); */

/*   if (t->sleep_until < tick) { */
/*     t->sleep_until = 0; */
/*     t->state = TASK_ACTIVE; */

/*     list_remove(node); */
/*     list_addAtRear(&task_active, node); */
/*   } */
/* } */

void kernel_task_wakeup_all(void)
{
  uint32_t tick = HAL_GetTick(); 
  //list_each_do(&task_sleeping, kernel_task_wakeup_task, (void*)tick);
  task_t * t = Heap.head(&sleeping.heap);
  while (t && t->sleep_until <= tick) {
    t = Heap.remove_head(&sleeping.heap);
    t->sleep_until = 0;
    t->state = TASK_ACTIVE;
    list_addAtRear(&task_active, task_to_list(t));

    t = Heap.head(&sleeping.heap);
  }
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
  current_task->sp = (uint8_t*)kernel_PSP_get();
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

static inline  __attribute__((const))
heap_key_t sleeping_heap_key(const void * cxt)
{
  const task_t * t = cxt;
  return t->sleep_until;
}

void kernel_task_display_task_stats(void)
{
  int i;
  char fmt[] = "%d\t%s\t%d/%d\t%.2f\t%s\n";
  os_iprintf("ID\tState\tStack\tTime\tName\n");
  int stack_size = IDLE_STACK_SIZE;
  int stack_usage = (char*)idle_task.task.stack_top - (char*)idle_task.task.sp;
  uint32_t usecs = usec_time();
  uint32_t runtime = idle_task.task.runtime;
  float runper = (((float)runtime / (float)usecs) * 100); 
  os_iprintf(fmt, idle_task.task.id,
             state_names[idle_task.task.state],
             stack_usage, stack_size,
             runper,
             idle_task.task.name
             );

  for (i = 0; i < MAX_TASK_COUNT; ++i) {
    task_t * t = task_list[i];
    if (t) {
      const char * name = default_name;
      if (t->name) name = t->name;
      if (i == 0) stack_size = MAIN_STACK_SIZE;
      else stack_size = (char*)(t->stack_top) - ((char*)t + sizeof(task_t));
      runtime = t->runtime;
      stack_usage = (char*)t->stack_top - (char*)t->sp;

      runtime = t->runtime;
      runper = (((float)runtime / (float)usecs) * 100); 

      os_iprintf(fmt, t->id,
                 state_names[t->state],
                 stack_usage, stack_size,
                 runper, name
                 );
    }
  }
}
