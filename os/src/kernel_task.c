#include <stdint.h>
#include "kernel.h"
#include "task.h"
#include "defs.h"
#include <string.h>
#include <assert.h>

static struct task *TCB[TASK_COUNT];
static struct task *current_task = NULL;
static struct task *idle_task = NULL;
static int task_insert_idx = 0;
static int current_task_idx = -1;

static void kernel_task_idle_func(void *c) { while (1); }

void kernel_task_init(void)
{
  memset(TCB, 0, TASK_COUNT * sizeof(struct task*));

  // start up idle task
  current_task_idx = 0;
  current_task = TCB[0];
  idle_task = task_create(kernel_task_idle_func, 128, NULL);
  idle_task->state = TASK_ACTIVE;
}

void kernel_task_active_next(void)
{
  /* int i; */
  /* struct task * t = NULL; */
  /* for (i = 1; i < TASK_COUNT; ++i) { */
  /*   t = TCB[i]; */
  /*   if (t && t->state == TASK_ACTIVE) */
  /*     break; */
  /* } */

  /* if (!t) t = idle_task; */
  /* current_task = t; */

  
  // Round-Robin Scheduler
  int next_task_idx = current_task_idx;
  int last = current_task_idx == -1
    ? task_insert_idx -1
    : current_task_idx;

  /* This loop should have 2 SUBTLE features
   * 1. if the current task is -1 (IDLE), then it starts at 0 index
   *    and examines all tasks
   * 2. otherwise, it looks at all tasks starting at current+1 mod
   *    the number of tasks, and should look at current LAST before
   *    giving up.
   */
  current_task = idle_task; // the default
  do {
    next_task_idx = (next_task_idx + 1) % task_insert_idx;
    assert(TCB[next_task_idx] && "Invalid TCB entry");
    if (TCB[next_task_idx]->state == TASK_ACTIVE) {
      current_task_idx = next_task_idx;
      current_task = TCB[current_task_idx];
      break;
    }
  } while (next_task_idx != last);
}


void kernel_task_wakeup(void)
{
  int i;
  /* TODO: handle rollover */
  uint32_t tick = HAL_GetTick();
  for (i = 0; i < TASK_COUNT; ++i) {
    struct task *t = TCB[i];
    if (t->state == TASK_SLEEP) {
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

void kernel_task_update_global_SP(void)
{
  kernel_PSP_set((uint32_t)current_task->psp);
}

void kernel_task_update_local_SP(void)
{
 current_task->psp = (void*)kernel_PSP_get();
}


