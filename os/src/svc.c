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


//void SVC_Handler_C(void)
void SVC_Handler(void)
{
  register uint32_t * frame;
  __asm volatile("mrs %0, psp \n" : "=r" (frame) ::);

  uint32_t call = frame[0],
    arg0 = frame[1],
    arg1 = frame[2],
    arg2 = frame[3];
  
  switch (call) {
  case SVC_YIELD:
    svc_yield();
    break;
  case SVC_START:
    svc_start();
    break;
  case SVC_TASK_START:
    svc_task_start((struct task *)arg0);
    break;
  case SVC_TASK_SLEEP:
    svc_task_sleep(arg0);
    break;
  case SVC_EVENT_WAIT:
    svc_event_wait((struct event *)arg0);
    break;
  case SVC_EVENT_NOTIFY:
    svc_event_notify((struct event *)arg0);
    break;
  default:
    kernel_break();
    break;
  }
}

static inline
void svc_start(void)
{
  kernel_PendSV_set();

  /* kernel_critical_begin(); */
  /* kernel_task_active_next(); */
  /* kernel_task_update_global_SP(); */
  /* kernel_critical_end(); */
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

