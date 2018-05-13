/*
 * event_control.h
 *
 *  Created on: May 13, 2018
 *      Author: sonny
 */

#ifndef OS_CORE_EVENT_CONTROL_H_
#define OS_CORE_EVENT_CONTROL_H_

#include "event_type.h"
#include "task_type.h"

void event_control_init(void);
int event_control_add(event_t * event);
int event_control_remove(event_t * event);
void event_control_each(void (*f)(event_t*));
int event_control_task_waiting(task_t *t);

#endif /* OS_CORE_EVENT_CONTROL_H_ */
