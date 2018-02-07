#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "stm32f746xx.h"
#include "kernel_task.h"
#include "task.h"
#include "event.h"
#include "spinlock.h"

struct mutex {
  volatile uint32_t lock;
  uint32_t depth;
  struct event waiting;
};

#define MUTEX_STATIC_INIT(name) { 0, 0, EVENT_STATIC_INIT( (name).waiting ) }

static inline
void mutex_init(struct mutex* m) {
  m->depth = 0;
  event_init(&m->waiting);
}

static inline
void mutex_lock(struct mutex *m) {
  int tid = current_task_id();

  if (spinlock_locked_as(&m->lock, tid)) {
    m->depth++;
  }
  else {
    while (!spinlock_try_lock_value(&m->lock, tid)) {
      event_wait(&m->waiting);
    }
  }

  /* got lock -- waiting cleared by notify */
}

static inline
int mutex_lock_try(struct mutex *m) {
  int tid = current_task_id();

  if (spinlock_locked_as(&m->lock, tid)) {
    m->depth++;
    return 1;
  }
  else 
    return spinlock_try_lock_value(&m->lock, tid);

}


static inline
void mutex_unlock(struct mutex *m) {
  if (m->depth == 0) {
    spinlock_unlock(&m->lock);
    if (event_task_waiting(&m->waiting))
      event_notify(&m->waiting);
  }
  else {
    m->depth--;
  }
}

#endif /* __MUTEX_H__ */
