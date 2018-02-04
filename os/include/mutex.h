#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "stm32f746xx.h"
#include "task.h"
#include "spinlock.h"

struct mutex {
  volatile uint32_t lock;
  uint32_t depth;
  struct event waiting;
};

static inline
void mutex_init(struct mutex* m) {
  m->depth = 0;
  event_init(m);
}

static inline
void mutex_lock(struct mutex *m) {
  int tid = current()->id;

  if (spinlock_locked_as(&m->lock, tid)) {
    m->depth++;
  }
  else {
    while (!spinlock_try_lock(&m->lock, tid)) {
      event_wait(&m->waiting);
    }
  }

  /* got lock -- waiting cleared by notify */
}


static inline
void mutex_unlock(struct mutex *m) {
  if (m->depth == 0) {
    spinlock_unlock(&m->lock);
    event_notify(&m->waiting);
  }
  else {
    m->depth--;
  }
}

#endif /* __MUTEX_H__ */
