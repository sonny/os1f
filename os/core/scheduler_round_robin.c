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

#if OS_SCHEDULER == SCHEDULER_ROUND_ROBIN

#define IDLE_TASK_ID    -1

static list_t ready_list = LIST_STATIC_INIT(ready_list);

static TASK_STATIC_CREATE(idle_task, "Idle", IDLE_STACK_SIZE, IDLE_TASK_ID);

static void scheduler_idle(void * ctx)
{
	(void)ctx;
	while(1) __WFI();
}

int scheduler_init(void)
{
	task_frame_init(&idle_task.task, scheduler_idle, NULL);
	return OS_OK;
}

int scheduler_reschedule_task(task_t * task)
{
	assert_protected();
	if (task->state != TASK_ACTIVE) return OSERR_VALUE;
	list_addAtRear(&ready_list, task_to_list(task));
	return OS_OK;
}

task_t * scheduler_get_next_ready(void)
{
	assert_protected();
	if (list_empty(&ready_list))
		return &idle_task.task;
	else
		return list_to_task(list_removeFront(&ready_list));
}

bool scheduler_task_ready(task_t * t)
{
	return list_element_of(task_to_list(t), &ready_list);
}

#endif
