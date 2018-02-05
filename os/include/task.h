#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include <stdbool.h>
#include "syscall.h"

struct task {
  void * stackp;
  int32_t  id;
  uint32_t state;
  uint32_t sleep_until;  
};

struct task * task_create(int stack_size);
struct task * task_init(struct task *t, void (*func)(void*), void *context);


__attribute__((always_inline)) static inline
void task_schedule(struct task *task)
{
  syscall_task_start(task);
}

__attribute__((always_inline)) static inline
struct task * task_create_schedule(void (*func)(void*), int stack_size, void *context)
{
  struct task * t = task_create(stack_size);
  task_init(t, func, context);
  task_schedule(t);
  return t;
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

struct stacked_regs {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr; // r14
  uint32_t pc; // r15
  uint32_t xpsr;
};

struct manual_regs {
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
  uint32_t r8;
  uint32_t r9;
  uint32_t r10;
  uint32_t r11;
};

struct regs {
  struct manual_regs manual;
  struct stacked_regs stacked;
};



#endif /* __TASK_H__ */
