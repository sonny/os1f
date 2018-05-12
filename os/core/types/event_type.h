/*
 * event_type.h
 *
 *  Created on: Apr 9, 2018
 *      Author: sonny
 */

#ifndef OS_CORE_TYPES_EVENT_TYPE_H_
#define OS_CORE_TYPES_EVENT_TYPE_H_

#include <stdint.h>
#include "list.h"

typedef struct {
	const char const * name;
	uint32_t signal;
	list_t waiting;
	uint32_t signature;
} event_t;

#define EVENT_STATIC_INIT(NAME) { #NAME, 0, LIST_STATIC_INIT( (NAME).waiting ), EVENT_SIGNATURE }

#endif /* OS_CORE_TYPES_EVENT_TYPE_H_ */
