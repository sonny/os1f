#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "defs.h"
#include "task_type.h"
#include "event.h"
#include "spinlock.h"
#include "mutex_type.h"


static inline
void mutex_init(mutex_t* m) {
	m->depth = 0;
	event_init(&m->tasks, "Mutex");
}

static inline
void mutex_lock(mutex_t *m) {
	int tid = get_current_task_id();

	if (spinlock_locked_as(&m->lock, tid)) {
		m->depth++;
	} else {
		while (!spinlock_try_lock_value(&m->lock, tid)) {
			event_wait(&m->tasks);
		}
	}

	/* got lock -- waiting cleared by notify */
}

static inline
int mutex_lock_try(mutex_t *m) {
	int tid = get_current_task_id();

	if (spinlock_locked_as(&m->lock, tid)) {
		m->depth++;
		return 1;
	} else
		return spinlock_try_lock_value(&m->lock, tid);

}

static inline
void mutex_unlock(mutex_t *m) {
	if (m->depth == 0) {
		spinlock_unlock(&m->lock);
		if (event_task_waiting(&m->tasks))
			event_notify(&m->tasks);
	} else {
		m->depth--;
	}
}

#endif /* __MUTEX_H__ */
