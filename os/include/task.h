#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include <stdbool.h>
#include "syscall.h"

struct task {
  void * psp;
  uint32_t id;
  uint32_t state;
  uint32_t sleep_until;  
};

struct task_iter {
  uint32_t idx;
};

struct task * task_create(void (*)(void*), int, void *);
void task_iter_get(struct task_iter*);
struct task * task_iter_next(struct task_iter*);

__attribute__((always_inline)) static inline
void task_start(struct task *task)
{
  syscall_task_start(task);
}

__attribute__((always_inline)) static inline
void task_sleep(uint32_t ms) 
{
  syscall_task_sleep(ms);
}

__attribute__((always_inline)) static inline
void task_wait(uint32_t state)
{
  syscall_task_wait(state);
}

// NOTE: cannot nest mutexes with this implementation
/* __attribute__((always_inline)) static inline */
/* bool task_notify(uint32_t id, int state) */
/* { */
/*   return task_change_from_state(TCB[id], state, TASK_ACTIVE); */
/* } */

#endif /* __TASK_H__ */
