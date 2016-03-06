#ifndef __EVENT_H__
#define __EVENT_H__

#include <stdatomic.h>
#include "stm32f746xx.h"
#include "task.h"
#include "bitfield.h"

struct event {
  //  bitfield_t waiting;
  uint32_t subscriptions;
  uint32_t notifications;
};

static inline void event_init(struct event *e) {
  e->subscriptions = 0;
  e->notifications = 0;
}
//void event_add_wait(struct event *e);

//void event_wait(struct event *e);
//void event_notify(struct event *e);

static inline void event_subscribe(struct event *e) {
  uint32_t val = 1 << task_current()->id;
  atomic_fetch_or(&e->subscriptions, val);
}

static inline void event_unsubscribe(struct event *e) {
  uint32_t val = 1 << task_current()->id;
  atomic_fetch_and(&e->subscriptions, ~val);
}

static inline bool event_is_subscribed(struct event *e, int tid) {
  return e->subscriptions & (1<<tid);
}

static inline void event_notify(struct event *e) {
  uint32_t subbed = e->subscriptions;
  int n = 31;

  while(subbed) {
    int z = __CLZ(subbed);
    int id = n - z;
    atomic_fetch_or(&e->notifications, 1<<id);
    task_notify(id, TASK_STATE_WAIT_EVENT);
    subbed <<= (z+1);
    n -= (z+1);
  }
}

static inline bool event_is_notified(struct event *e, int tid) {
  return e->notifications & (1<<tid);
}

static inline void event_acknowledge(struct event *e) {
  uint32_t val = 1 << task_current()->id;
  atomic_fetch_and(&e->notifications, ~val);
}

static inline void event_wait(struct event *e) {
  int tid = task_current()->id;
  if (!event_is_notified(e, tid))
    task_wait(TASK_STATE_WAIT_EVENT);

  event_acknowledge(e);
  event_unsubscribe(e);
}

#endif  /* __EVENT_H__ */
