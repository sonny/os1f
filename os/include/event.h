#ifndef __EVENT_H__
#define __EVENT_H__

#include "svc.h"
#include "list.h"
#include "kernel_task.h"

struct event {
  struct list waiting;
};

#define EVENT_STATIC_INIT(name) { LIST_STATIC_INIT( (name).waiting ) }


static void protected_event_wait(void * cxt);
static void protected_event_notify(void *cxt);

static inline
void event_init(struct event *e)
{
  list_init(&e->waiting);
}

static inline
void event_notify(struct event *e)
{
  service_call(protected_event_notify, e);
}


static inline
void event_wait(struct event *e)
{
  service_call(protected_event_wait, e);
}

static inline
bool event_task_waiting(struct event *e)
{
  return !list_empty(&e->waiting);
}

static inline
void protected_event_wait(void * cxt)
{
  struct event *e = cxt;
  kernel_critical_begin();
  kernel_task_event_wait(e);
  kernel_critical_end();
  kernel_PendSV_set();
}

static inline
void protected_event_notify(void *cxt)
{
  struct event *e = cxt;
  kernel_critical_begin();
  kernel_task_event_notify(e);
  kernel_critical_end();
}

#endif  /* __EVENT_H__ */
