/*
 * scheduler_trivial.c
 *
 *  Created on: May 12, 2018
 *      Author: sonny
 */

#include "scheduler.h"

#if OS_SCHEDULER == SCHEDULER_TRIVIAL

#include "defs.h"
#include "assertions.h"
#include "error.h"
#include "task_type.h"
#include "task_control.h"
#include "task.h"
#include "kernel.h"
#include "systimer.h"

int scheduler_init(void)
{
	systimer_create_exec(5, kernel_context_switch_irq, NULL);
	return OS_OK;
}

int scheduler_reschedule_task(task_t * task)
{
	assert_protected();
	return OS_OK;
}

int scheduler_unschedule_task(task_t * task)
{
	assert_protected();
	// list from ANY list
	list_remove(task_to_list(task));
	return OS_OK;
}

task_t * scheduler_get_next_ready(void)
{
	static int current_id = 0;
	assert_protected();
	int prev_id = current_id;
	// find next active task
	task_t * next = NULL;
	do {
		current_id = (current_id + 1) % MAX_TASK_COUNT;
		next = task_control_get(current_id);
	} while ((next == NULL || next->state != TASK_READY) && current_id != prev_id);

	if (!next || next->state != TASK_READY) next = task_control_get(IDLE_TASK_ID);
	return next;
}

bool scheduler_task_ready(task_t *t)
{
	return t->state == TASK_READY;
}

#endif
