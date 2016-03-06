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
  uint32_t waiting;
};

static inline void mutex_init(struct mutex* m) {
  m->waiting = 0;
}

static inline void mutex_lock(struct mutex *m) {
  int tid = task_current()->id;
  
  while (!spinlock_try_lock(&m->lock)) {
    atomic_fetch_or(&m->waiting, (1<<tid));
    task_wait(TASK_STATE_WAIT_MUTEX);     /* this yields */
  }

  /* got lock */
  atomic_fetch_and(&m->waiting, ~(1<<tid));
}

static inline void mutex_unlock(struct mutex *m) {
  spinlock_unlock(&m->lock);
  int n = 31;
  
  uint32_t waiting = m->waiting;
  while(waiting) {
    int z = __CLZ(waiting);
    int id = n - z;
    atomic_fetch_and(&m->waiting, ~(1<<id));
    task_notify(id, TASK_STATE_WAIT_MUTEX);
    waiting <<= (z+1);
    n -= (z+1);
  }
}

#endif /* __MUTEX_H__ */
