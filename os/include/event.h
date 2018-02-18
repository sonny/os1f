#ifndef __EVENT_H__
#define __EVENT_H__

#include "defs.h"
#include "svc.h"
#include "list.h"
#include "kernel.h"
#include "kernel_task.h"

struct event {
  list_t waiting;
};

#define EVENT_STATIC_INIT(name) { LIST_STATIC_INIT( (name).waiting ) }


static void protected_event_wait(void * cxt);
static void protected_event_notify(void *cxt);

static inline
void event_init(event_t *e)
{
  list_init(&e->waiting);
}

static inline
void event_notify(event_t *e)
{
  service_call(protected_event_notify, e);
}


static inline
void event_wait(event_t *e)
{
  service_call(protected_event_wait, e);
}

static inline
bool event_task_waiting(event_t *e)
{
  return !list_empty(&e->waiting);
}

static inline
void protected_event_wait(void * cxt)
{
  event_t *e = cxt;
  kernel_critical_begin();
  kernel_task_event_wait(e);
  kernel_critical_end();
  protected_kernel_context_switch(NULL);
}

static inline
void protected_event_notify(void *cxt)
{
  event_t *e = cxt;
  kernel_critical_begin();
  kernel_task_event_notify(e);
  kernel_critical_end();
}

#endif  /* __EVENT_H__ */
