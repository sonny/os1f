/*
 * assertions.h
 *
 *  Created on: Mar 14, 2018
 *      Author: sonny
 */

#ifndef OS_INCLUDE_ASSERTIONS_H_
#define OS_INCLUDE_ASSERTIONS_H_

#include <assert.h>
#include "task_type.h"
#include "event_type.h"
#include "mutex_type.h"

void assert_os_started(void); // implemented in kernel.c


static inline
void assert_task_sig(task_t *t)
{
	assert(t->signature == TASK_SIGNATURE && "Invalid task signature.");
}

static inline
void assert_event_sig(event_t *e)
{
	assert(e->signature == EVENT_SIGNATURE && "Invalid signature.");
}

static inline
void assert_mutex_sig(mutex_t *m)
{
	assert(m->signature == TASK_SIGNATURE && "Invalid signature.");
}


__attribute__ ((always_inline)) static inline
void assert_protected(void)
{
	uint32_t primask;
	__asm volatile("mrs %0, PRIMASK\n" : "=r"(primask));
	assert(primask & 1 && "Not in protected section.");
}

#endif /* OS_INCLUDE_ASSERTIONS_H_ */
