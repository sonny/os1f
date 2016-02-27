#ifndef __EVENT_H__
#define __EVENT_H__

#include "task.h"
#include "bitfield.h"

struct event {
  //uint8_t waiting[TASK_COUNT];
  bitfield_t waiting;
};

void event_init(struct event *e);
void event_add_wait(struct event *e);
void event_wait(struct event *e);
void event_notify(struct event *e);

#endif  /* __EVENT_H__ */
