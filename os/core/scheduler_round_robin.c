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
#include "task_type.h"
#include "kernel.h"
#include "systimer.h"
#include "list.h"

#if OS_SCHEDULER == SCHEDULER_ROUND_ROBIN

static list_t ready_list = LIST_STATIC_INIT(ready_list);

int scheduler_init(void)
{
	systimer_create_exec(5, kernel_context_switch_irq, NULL);
	return OS_OK;
}

int scheduler_reschedule_task(task_t * task)
{
	assert_protected();
	if (task->state != TASK_READY || task->id < 0) return OSERR_VALUE;
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
	task_t * next = list_to_task(list_removeFront(&ready_list));
	return next;
}

bool scheduler_task_ready(task_t * t)
{
	bool is_idle = (t == task_control_get_idle_task());
	bool in_list = list_contains(&ready_list, task_to_list(t));

	return (t->state == TASK_READY && (is_idle || in_list));
}

#endif
