/*
 * scheduler_trivial.h
 *
 *  Created on: May 12, 2018
 *      Author: sonny
 */

#ifndef OS_CORE_SCHEDULER_H_
#define OS_CORE_SCHEDULER_H_

#include "task_type.h"

int scheduler_init(void);
int scheduler_reschedule_task(task_t * task);
int scheduler_unschedule_task(task_t * task);
task_t * scheduler_get_next_ready(void);
bool scheduler_task_ready(task_t * t);


#define SCHEDULER_TRIVIAL     1
#define SCHEDULER_ROUND_ROBIN 2
#define SCHEDULER_PRIORITY    3

//#define OS_SCHEDULER SCHEDULER_ROUND_ROBIN
//#define OS_SCHEDULER SCHEDULER_TRIVIAL
#define OS_SCHEDULER SCHEDULER_PRIORITY

#endif /* OS_CORE_SCHEDULER_H_ */
