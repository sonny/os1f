#include "stm32f746xx.h"
#include "mutex.h"
#include "spinlock.h"
#include "task.h"
//#include "sched.h"

#define MUTEX_LOCKED   1
#define MUTEX_UNLOCKED 0

void mutex_init(struct mutex *m)
{
  bitfield_init(&m->waiting);
}

void mutex_lock(struct mutex *m)
{
  const struct task *t = task_current();
  
  while (!spinlock_try_lock(&m->lock)) {
    bitfield_set(&m->waiting, t->id);
    __DMB();
    task_wait(WAIT_MUTEX);     /* this yields */
  }

  /* got lock */
  bitfield_clear(&m->waiting, t->id);
  return;
}

void mutex_unlock(struct mutex *m)
{
  int i;
  spinlock_unlock(&m->lock);

  /* notify all waiting tasks */
  for (i = 0; i < TASK_COUNT; ++i) {
    if (bitfield_is_set(m->waiting, i) &&
        task_get(i)->state == TASK_STATE_WAIT_MUTEX)
      {
        // avoid race condition
        bitfield_clear(&m->waiting, i);
        __DMB();
        task_notify(i);
      }
  }
  
}

