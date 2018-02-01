#include <stdint.h>
#include "kernel.h"
#include "task.h"
#include "defs.h"
#include <string.h>

static struct task *TCB[TASK_COUNT];
static struct task *current_task = NULL;
static struct task *idle_task = NULL;

static void kernel_task_idle_func(void *c) { while (1); }

void kernel_task_init(void)
{
  memset(TCB, 0, TASK_COUNT * sizeof(struct task*));
  current_task = TCB[0];
  idle_task = task_create(kernel_task_idle_func, 128, NULL);
  idle_task->state = TASK_ACTIVE;
}

void kernel_task_active_next(void)
{
  static int current_task_idx = 0;
  // find next active task
  // NOTE: this will spin unless there is at least one active task
  // Since PendSV is the lowest priority, another irq handler
  // can update the state of the TCB to activate an inactive task
  /* current_task_idx = (current_task_idx + 1) % TASK_COUNT; */
  /* struct task * t = TCB[current_task_idx]; */
  /* while(!t || t->state != TASK_ACTIVE) { */
  /*   current_task_idx = (current_task_idx + 1) % TASK_COUNT; */
  /*   struct task * t = TCB[current_task_idx]; */
  /* } */
  int i;
  struct task * t = NULL;
  for (i = 1; i < TASK_COUNT; ++i) {
    t = TCB[i];
    if (t && t->state == TASK_ACTIVE)
      break;
  }

  if (!t) t = idle_task;
  current_task = t;
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
  static int task_counter = 1;
  new->id = task_counter;
  TCB[task_counter++] = new;
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


