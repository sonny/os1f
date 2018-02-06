#include <stdint.h>
#include "kernel.h"
#include "task.h"
#include "event.h"
#include "defs.h"
#include "list.h"
#include <string.h>
#include <assert.h>

#define IDLE_STACK_SIZE 128
#define MAIN_STACK_SIZE 1024
#define IDLE_TASK_ID    -1

struct task_node {
  struct list list;
  struct task *task;
};

static struct task_node * current_task_node = NULL;
static struct list task_active;
static struct list task_sleeping;
static struct list task_waiting;

/* idle task */
static uint8_t idle_task_stack[IDLE_STACK_SIZE];
static struct task idle_task = {0};

/* main task */
static uint8_t main_task_stack[MAIN_STACK_SIZE]; // default size of main stack
static struct task main_task = {0};
static struct task_node main_task_node;

static void kernel_task_main_hoist(void);

static void kernel_task_idle_func(void *c) { while (1); }

static inline
struct task *current_task(void)
{
  if (current_task_node)
    return current_task_node->task;
  else
    return &idle_task;
}

void kernel_task_init(void)
{
  // Init TCB
  list_init(&task_active);
  list_init(&task_sleeping);
  list_init(&task_waiting);
  
  // Init Idle Task
  idle_task.stackp = &idle_task_stack[0] + IDLE_STACK_SIZE;
  idle_task.id = IDLE_TASK_ID;
  idle_task.state = TASK_ACTIVE;
  idle_task.sleep_until = 0;
  task_init(&idle_task, kernel_task_idle_func, NULL);
  
  kernel_task_main_hoist();
}

static inline
struct task_node * kernel_task_create_task_node(struct task *task)
{
  struct task_node * result = malloc(sizeof(struct task_node));
  list_init((struct list *)result);
  result->task = task;
  return result;
}

// NOTE: Do Not Call from inside an IRQ
// This function switches modes from privileged to user
// Handle with care
// NOTE: NO MALLOC until after syscall_start
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
  void * adjusted_main_task_sp = main_task_sp - sizeof(struct regs);
  // Ensure that result stack is aligned on 8 byte boundary
  if ((uint32_t)adjusted_main_task_sp % 8) {
    adjusted_main_task_sp -= 4;
  }
  
  // Init main task object
  main_task.stackp = adjusted_main_task_sp;
  main_task.id = 0;
  main_task.state = TASK_ACTIVE;
  main_task.sleep_until = 0;

  list_init((struct list*)&main_task_node);
  main_task_node.task = &main_task;
  list_addAtRear(&task_active, (struct list *)&main_task_node);

  current_task_node = NULL;
  
  // Set PSP to our new stack and Change mode to unprivileged
  kernel_sync_barrier();
  kernel_PSP_set((uint32_t)main_task_sp);
  // Recover MSP for interrupt handles -- has to happen before mode change
  kernel_MSP_set(stack_base);
  kernel_sync_barrier();
  kernel_CONTROL_set(0x01 << 1 | 0x01 << 0); // use PSP with unprivileged thread mode
  kernel_sync_barrier();

  // need to call start here in order to keep the SP valid
  syscall_start();
}

void kernel_task_schedule(void)
{
  if (current_task_node) {
    switch(current_task_node->task->state) {
    case TASK_END:
      free(current_task_node);
      break;
    case TASK_SLEEP:
      list_addAtRear(&task_sleeping, (struct list *)current_task_node); 
      break;
    case TASK_WAIT:
      list_addAtRear(&task_waiting, (struct list *)current_task_node);
      break;
    case TASK_ACTIVE:
      list_addAtRear(&task_active, (struct list *)current_task_node); 
      break;
    default:
      // invalid state
      kernel_break();
      break;
    }
  }
  current_task_node = NULL;
}

void kernel_task_active_next(void)
{
  assert(current_task_node == NULL && "Something bad happened to the scheduler");
  if (!list_empty(&task_active))
    current_task_node = (struct task_node*)list_removeFront(&task_active);
}

static inline
void kernel_task_wakeup_task(struct list *l, const void * context)
{
  struct task *t = ((struct task_node *)l)->task;
  const uint32_t tick = (uint32_t)context;
  assert(t->state == TASK_SLEEP && "Tasks in sleeping queue must be asleep.");

  if (t->sleep_until < tick) {
    t->sleep_until = 0;
    t->state = TASK_ACTIVE;

    list_remove(l);
    list_addAtRear(&task_active, l);
  }
}

void kernel_task_wakeup(void)
{
  uint32_t tick = HAL_GetTick(); 
  list_each_do(&task_sleeping, kernel_task_wakeup_task, (void*)tick);
}

void kernel_task_start(struct task * new)
{
  struct task_node * tn = kernel_task_create_task_node(new);
  new->state = TASK_ACTIVE; 
  list_addAtRear(&task_active, (struct list*)tn);
}

void kernel_task_sleep(uint32_t ms)
{
  assert(current_task_node && "Cannot sleep idle task.");
  struct task * t = current_task();
  t->state = TASK_SLEEP;
  t->sleep_until = HAL_GetTick() + ms;
}

void kernel_task_event_wait(struct event * e)
{
  assert(current_task_node && "Cannot wait idle task.");
  struct task * t = current_task();
  e->waiting |= 1 << t->id;
  t->state = TASK_WAIT;
}

static inline
void kernel_task_event_notify_task(struct list *l, const void * ctx)
{
  const struct event * e = ctx;
  struct task * t = ((struct task_node *)l)->task;
  assert(t->state == TASK_WAIT && "Tasks in waiting queue must be waiting.");
  if (((uint32_t)1 << t->id) & e->waiting) {
    t->state = TASK_ACTIVE;

    list_remove(l);
    list_addAtRear(&task_active, l);
  }
}

void kernel_task_event_notify(struct event * e)
{
  list_each_do(&task_waiting, kernel_task_event_notify_task, e);
}

/* void kernel_task_remove(struct task * t) */
/* { */
/*   //TCB[t->id] = NULL; */
/* } */


void kernel_task_update_global_SP(void)
{
  struct task * t = current_task();
  kernel_PSP_set((uint32_t)t->stackp);
}

void kernel_task_update_local_SP(void)
{
  struct task * t = current_task();
  t->stackp = (void*)kernel_PSP_get();
}

uint32_t current_task_id(void)
{
  return current_task()->id;
}

void kernel_task_end(void)
{
  struct task * t = current_task();
  t->state = TASK_END;
  event_notify(&t->join);
  task_yield();
}

