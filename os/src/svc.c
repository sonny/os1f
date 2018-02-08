#include <stdint.h>
#include "svc.h"
#include "kernel.h"
#include "kernel_task.h"
#include "defs.h"

static void svc_start(void*);
static void svc_yield(void*);
static void svc_task_start(void*);
static void svc_task_sleep(void*);
static void svc_task_wait(void*);
static void svc_event_wait(void*);
static void svc_event_notify(void*);

void service_call(void (*call)(void*), void *cxt)
{
  __asm volatile("svc 0\n");
}

void service_start(void)
{
  service_call(svc_start, NULL);
}

void service_yield(void)
{
  service_call(svc_yield, NULL);
}

void service_task_start(struct task *t)
{
  service_call(svc_task_start, t);
}

void service_task_sleep(uint32_t ms)
{
  service_call(svc_task_sleep, (void*)ms);
}

void service_event_wait(struct event *e)
{
  service_call(svc_event_wait, e);
}

void service_event_notify(struct event *e)
{
  service_call(svc_event_notify, e);
}

void SVC_Handler(void)
{
  register uint32_t * frame;
  __asm volatile("mrs %0, psp \n" : "=r" (frame) ::);

  svcall_t call = (svcall_t)frame[0];
  void *args = (void*)frame[1];
  
  call(args);
}

static inline
void svc_start(void * cxt)
{
  kernel_PendSV_set();
}

static inline
void svc_yield(void *cxt)
{
  kernel_PendSV_set();
}

void svc_task_start(void * cxt)
{
  struct task * new = cxt;
  kernel_critical_begin();
  kernel_task_start(new);
  kernel_critical_end();
}

void svc_task_sleep(void *cxt)
{
  uint32_t ms = (uint32_t)cxt;
  kernel_critical_begin();
  kernel_task_sleep(ms);
  kernel_critical_end();
  kernel_PendSV_set();
}

void svc_event_wait(void * cxt)
{
  struct event *e = cxt;
  kernel_critical_begin();
  kernel_task_event_wait(e);
  kernel_critical_end();
  kernel_PendSV_set();
}

void svc_event_notify(void *cxt)
{
  struct event *e = cxt;
  kernel_critical_begin();
  kernel_task_event_notify(e);
  kernel_critical_end();
}

