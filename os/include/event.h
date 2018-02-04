#ifndef __EVENT_H__
#define __EVENT_H__

#include <stdatomic.h>
#include "stm32f746xx.h"
#include "task.h"

struct event {
  uint32_t waiting;
};

static inline
void event_init(struct event *e)
{
  e->waiting = 0;
}

static inline
void event_notify(struct event *e)
{
  syscall_event_notify(e);
}


static inline
void event_wait(struct event *e)
{
  syscall_event_wait(e);
}

#endif  /* __EVENT_H__ */
