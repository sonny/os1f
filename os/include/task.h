#ifndef __TASK_H__
#define __TASK_H__

#include <stdatomic.h>
#include <stdint.h>
#include <stdbool.h>
#include "kernel.h"
#include "defs.h"

struct task {
  void * psp;
  uint32_t flags;
  uint32_t sleep_until;  
};

extern struct task* current_task;
void taskNext(void);
void taskStart(void (*)(void*), int, struct task *); 

void * taskGet(int i);


__attribute__((always_inline)) static inline
bool task_change_from_state(struct task *t, uint32_t from, uint32_t to)
{
  return atomic_compare_exchange_strong(&(t->flags), &from, to);
}

__attribute__((always_inline)) static inline
bool task_change_from_state_current(uint32_t from, uint32_t to)
{
  return task_change_from_state(current_task, from, to);
}

__attribute__((always_inline)) static inline
void taskSleepUntil(uint32_t ms) 
{
  if (task_change_from_state_current(TASK_ACTIVE, TASK_SLEEP)) {
    current_task->sleep_until = ms;
    syscall_yield();
  }
}

__attribute__((always_inline)) static inline
void taskSleep(uint32_t ms) 
{
  taskSleepUntil(HAL_GetTick() + ms);
}

__attribute__((always_inline)) static inline
void taskWait(uint32_t state)
{
  if (task_change_from_state_current(TASK_ACTIVE, state)) {
    syscall_yield();
  }
}

// NOTE: cannot nest mutexes with this implementation
__attribute__((always_inline)) static inline
bool task_notify(uint32_t id, int state)
{
  return task_change_from_state(TCB[id], state, TASK_ACTIVE);
}

#endif /* __TASK_H__ */
