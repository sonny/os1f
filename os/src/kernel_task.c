#include <stdint.h>
#include "kernel.h"
#include "task.h"
#include "event.h"
#include "defs.h"
#include <string.h>
#include <assert.h>

#define IDLE_STACK_SIZE 128
#define MAIN_STACK_SIZE 1024

static struct task *TCB[TASK_COUNT];
static struct task *current_task = NULL;
static int task_insert_idx = 1;
static int current_task_idx = -1;

/* idle task */
static uint8_t idle_task_stack[IDLE_STACK_SIZE];
static struct task idle_task = {0};

/* main task */
static uint8_t main_task_stack[MAIN_STACK_SIZE]; // default size of main stack
static struct task main_task = {0};

static void kernel_task_idle_func(void *c) { while (1); }

// NOTE: Do Not Call from inside an IRQ
// This function switches modes from privileged to user
// Handle with care
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
  TCB[0] = &main_task;

  current_task_idx = 0;
  current_task = &main_task;
  
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

void kernel_task_init(void)
{
  // Init TCB
  memset(TCB, 0, TASK_COUNT * sizeof(struct task*));
  // Init Idle Task
  idle_task.stackp = &idle_task_stack[0] + IDLE_STACK_SIZE;
  idle_task.id = -1;
  idle_task.state = TASK_ACTIVE;
  idle_task.sleep_until = 0;
  task_init(&idle_task, kernel_task_idle_func, NULL);
  
  kernel_task_main_hoist();
}

void kernel_task_active_next(void)
{
  // Round-Robin Scheduler
  // Checks each task, starting with the current_task + 1
  // and (possibly) wrapping around the array to check
  // the current_task last
  int next_task_idx = current_task_idx;
  int last = (current_task == &idle_task)
    ? (task_insert_idx - 1)
    : current_task_idx;

  /* This loop should have 2 SUBTLE features
   * 1. if the current task is -1 (IDLE), then it starts at 0 index
   *    and examines all tasks
   * 2. otherwise, it looks at all tasks starting at current+1 mod
   *    the number of tasks, and should look at current LAST before
   *    giving up.
   */
  current_task = &idle_task; // the default
  current_task_idx = -1;
  do {
    next_task_idx = (next_task_idx + 1) % task_insert_idx;
    struct task * task = TCB[next_task_idx];
    if (task && task->state == TASK_ACTIVE) {
      current_task_idx = next_task_idx;
      current_task = task;
      break;
    }
  } while (next_task_idx != last);
}


void kernel_task_wakeup(void)
{
  int i;
  /* TODO: handle rollover */
  uint32_t tick = HAL_GetTick();
  for (i = 0; i < task_insert_idx; ++i) {
    struct task *t = TCB[i];
    if (t && t->state == TASK_SLEEP) {
      if (t->sleep_until < tick) {
        t->sleep_until = 0;
        t->state = TASK_ACTIVE;
      }
    }
  }
}

void kernel_task_start(struct task * new)
{
  new->id = task_insert_idx;
  TCB[task_insert_idx++] = new;
  new->state = TASK_ACTIVE;
}

void kernel_task_sleep(uint32_t ms)
{
  current_task->state = TASK_SLEEP;
  current_task->sleep_until = HAL_GetTick() + ms;
}

void kernel_task_wait(uint32_t wait_state)
{
  current_task->state = wait_state;
}

void kernel_task_event_wait(struct event * e)
{
  e->waiting |= 1 << current_task->id;
  current_task->state = TASK_WAIT;
}

void kernel_task_event_notify(struct event * e)
{
  uint32_t waiting = e->waiting;
  int n = 31;
  
  while(waiting) {
    uint32_t z = __CLZ(waiting);
    int id = n - z;
    // notify task
    TCB[id]->state = TASK_ACTIVE;
    waiting <<= (z+1);
    n -= (z+1);
  }
  e->waiting = 0;

}

void kernel_task_update_global_SP(void)
{
  kernel_PSP_set((uint32_t)current_task->stackp);
}

void kernel_task_update_local_SP(void)
{
 current_task->stackp = (void*)kernel_PSP_get();
}

uint32_t current_task_id(void)
{
  return current_task->id;
}
