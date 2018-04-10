/*
 * mutex_type.h
 *
 *  Created on: Apr 10, 2018
 *      Author: sonny
 */

#ifndef OS_CORE_TYPES_MUTEX_TYPE_H_
#define OS_CORE_TYPES_MUTEX_TYPE_H_

#include "spinlock.h"
#include "event_type.h"
#include <stdint.h>

typedef struct {
	volatile spinlock_t lock;
	uint32_t depth;
	event_t tasks;
	uint32_t signature;
} mutex_t;

#define MUTEX_STATIC_INIT(name) { 0, 0, EVENT_STATIC_INIT( (name).tasks ), MUTEX_SIGNATURE }


#endif /* OS_CORE_TYPES_MUTEX_TYPE_H_ */
