#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "defs.h"

static inline __attribute__ ((always_inline))
void __syscall4(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3)
{
  __asm volatile ("mov r0, %0\n"
                  "mov r1, %1\n"
                  "mov r2, %2\n"
                  "mov r3, %3\n"
                  "svc 0     \n"
                  : /* no output */
                  : "r"(r0),"r"(r1),"r"(r2),"r"(r3)
                  : "r0","r1","r2","r3" );
}

static inline __attribute__ ((always_inline))
void __syscall3(uint32_t r0, uint32_t r1, uint32_t r2)
{
  __syscall4(r0, r1, r2, 0);
}

static inline __attribute__ ((always_inline))
void __syscall2(uint32_t r0, uint32_t r1)
{
  __syscall4(r0, r1, 0, 0);
}

static inline __attribute__ ((always_inline))
void __syscall(uint32_t r0)
{
  __syscall4(r0, 0, 0, 0);
}

static inline __attribute__ ((always_inline))
void syscall_start(void)
{
  __syscall(SVC_START);
}

static inline __attribute__ ((always_inline))
void syscall_yield(void)
{
  __syscall(SVC_YIELD);
}

static inline __attribute__ ((always_inline))
void syscall_task_start(struct task * t)
{
  __syscall2(SVC_TASK_START, (uint32_t)t);
}

static inline __attribute__ ((always_inline))
void syscall_task_sleep(uint32_t ms)
{
  __syscall2(SVC_TASK_SLEEP, ms);
}

/* static inline __attribute__ ((always_inline)) */
/* void syscall_task_wait(uint32_t state) */
/* { */
/*   __syscall2(SVC_TASK_WAIT, state); */
/* } */

static inline __attribute__ ((always_inline))
void syscall_event_wait(struct event *e)
{
  __syscall2(SVC_EVENT_WAIT, (uint32_t)e);
}

static inline __attribute__ ((always_inline))
void syscall_event_notify(struct event *e)
{
  __syscall2(SVC_EVENT_NOTIFY, (uint32_t)e);
}

static inline __attribute__ ((always_inline))
void syscall_task_remove(struct task *t)
{
  __syscall2(SVC_TASK_REMOVE, (uint32_t)t);
}

#endif /* __SYSCALL_H__ */
