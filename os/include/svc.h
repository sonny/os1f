#ifndef __SVC_H__
#define __SVC_H__

#include "defs.h"

typedef void (*svcall_t)(void*);

void service_call(void (*call)(void*), void *cxt);
void service_start(void);
void service_yield(void);
void service_task_start(struct task *t);
void service_task_sleep(uint32_t ms);
void service_event_wait(struct event *e);
void service_event_notify(struct event *e);

#endif  /* __SVC_H__ */
