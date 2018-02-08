#ifndef __EVENT_H__
#define __EVENT_H__

#include "svc.h"
#include "list.h"

struct event {
  struct list waiting;
};

#define EVENT_STATIC_INIT(name) { LIST_STATIC_INIT( (name).waiting ) }

static inline
void event_init(struct event *e)
{
  list_init(&e->waiting);
}

static inline
void event_notify(struct event *e)
{
  service_event_notify(e);
}


static inline
void event_wait(struct event *e)
{
  service_event_wait(e);
}

static inline
bool event_task_waiting(struct event *e)
{
  return !list_empty(&e->waiting);
}

#endif  /* __EVENT_H__ */
