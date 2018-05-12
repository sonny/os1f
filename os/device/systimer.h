/*
 * systimer.h
 *
 *  Created on: Apr 22, 2018
 *      Author: sonny
 */

#ifndef OS_CORE_SYSTIMER_H_
#define OS_CORE_SYSTIMER_H_

#include <stddef.h>
#include "event_type.h"

typedef enum {
	TIMER_NONE,
	TIMER_TIMEOUT,
	TIMER_EVENT,
	TIMER_EXEC_IRQ,
	TIMER_EXEC_DEF,
} timer_type_e;

typedef void (*timer_callback)(void*);

typedef struct systimer_s systimer_t;

struct systimer_s {
	list_t node;
	uint16_t exec_at;
	uint16_t period; // set to 0 to make timer happen once
	timer_callback callback;
	void * cb_ctx;
	event_t *event;
	timer_type_e type;
};

void systimer_init(void);
systimer_t * systimer_create_exec(size_t, timer_callback, void*);
uint32_t msec_time(void);
uint64_t usec_time(void);

#endif /* OS_CORE_SYSTIMER_H_ */