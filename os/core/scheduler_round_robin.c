/*
 * scheduler_round_robin.c
 *
 *  Created on: May 12, 2018
 *      Author: sonny
 */

#include "scheduler.h"
#include "defs.h"
#include "error.h"
#include "board.h"
#include "assertions.h"
#include "task_control.h"

#if OS_SCHEDULER == SCHEDULER_ROUND_ROBIN

static list_t ready_list = LIST_STATIC_INIT(ready_list);

void scheduler_idle(void * ctx)
{
	(void)ctx;
	while(1) {
		__WFI();
	}
}

int scheduler_init(void)
{
	task_control_init();
	return OS_OK;
}

int scheduler_reschedule_task(task_t * task)
{
	assert_protected();
	if (task->state != TASK_ACTIVE || task->id < 0) return OSERR_VALUE;
	list_addAtRear(&ready_list, task_to_list(task));
	return OS_OK;
}

int scheduler_unschedule_task(task_t * task)
{
	assert_protected();
	list_remove(task_to_list(task));
	return OS_OK;
}

task_t * scheduler_get_next_ready(void)
{
	assert_protected();
	if (list_empty(&ready_list))
		return task_control_get(IDLE_TASK_ID);
	else
		return list_to_task(list_removeFront(&ready_list));
}

bool scheduler_task_ready(task_t * t)
{
	return list_element_of(task_to_list(t), &ready_list);
}

#endif
