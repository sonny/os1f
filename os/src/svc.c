#include <stdint.h>
#include "kernel.h"
#include "kernel_task.h"
#include "defs.h"

static void svc_start(void);
static void svc_yield(void);
static void svc_task_start(struct task *);
static void svc_task_sleep(uint32_t);
static void svc_task_wait(uint32_t);
static void svc_event_wait(struct event *e);
static void svc_event_notify(struct event *e);
static void svc_task_remove(struct task *t);


void SVC_Handler_C(void)
{
  uint32_t r0, r1, r2, r3;

  // account for push done by the wrapper
  __asm volatile("mrs r12, psp      \n"
                 "ldr %0, [r12, 32] \n"
                 "ldr %1, [r12, 36] \n"
                 "ldr %2, [r12, 40] \n"
                 "ldr %3, [r12, 44] \n"
                 : "=r" (r0), "=r" (r1), "=r" (r2), "=r" (r3)
                 : /* no input */
                 : "r12" );

  switch (r0) {
  case SVC_YIELD:
    svc_yield();
    break;
  case SVC_START:
    svc_start();
    break;
  case SVC_TASK_START:
    svc_task_start((struct task *)r1);
    break;
  case SVC_TASK_SLEEP:
    svc_task_sleep(r1);
    break;
  case SVC_EVENT_WAIT:
    svc_event_wait((struct event *)r1);
    break;
  case SVC_EVENT_NOTIFY:
    svc_event_notify((struct event *)r1);
    break;
  default:
    kernel_break();
    break;
  }
}

static inline
void svc_start(void)
{
  kernel_critical_begin();
  kernel_task_active_next();
  kernel_task_update_global_SP();
  kernel_critical_end();
}

static inline
void svc_yield(void)
{
  kernel_PendSV_set();
}

void svc_task_start(struct task * new)
{
  kernel_critical_begin();
  kernel_task_start(new);
  kernel_critical_end();
}

void svc_task_sleep(uint32_t ms)
{
  kernel_critical_begin();
  kernel_task_sleep(ms);
  kernel_critical_end();
  kernel_PendSV_set();
}

void svc_event_wait(struct event *e)
{
  kernel_critical_begin();
  kernel_task_event_wait(e);
  kernel_critical_end();
  kernel_PendSV_set();
}

void svc_event_notify(struct event *e)
{
  kernel_critical_begin();
  kernel_task_event_notify(e);
  kernel_critical_end();
}

void svc_task_remove(struct task *t)
{
  /*
  kernel_critical_begin();
  kernel_task_remove(t);
  kernel_critical_end();
  */
}
