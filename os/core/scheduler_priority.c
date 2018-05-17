/*
 * scheduler_priority.c
 *
 *  Created on: May 15, 2018
 *      Author: sonny
 */

#include "scheduler.h"

#if OS_SCHEDULER == SCHEDULER_PRIORITY

#include "kernel.h"
#include "error.h"
#include "task_control.h"
#include "assertions.h"
#include "systimer.h"

static list_t ready_list = LIST_STATIC_INIT(ready_list);
static systimer_t scheduler_timer = {
		.node = LIST_STATIC_INIT(scheduler_timer.node),
		.period = 5,
		.callback = kernel_context_switch_irq,
		.type = TIMER_EXEC_IRQ
};

int scheduler_init(void)
{
	return OS_OK;
}

static bool scheduler_insert_condition(list_t * node, list_t * new)
{
	// true if if the priority of the new node is higher
	const task_t * node_task = list_to_task(node);
	const task_t * ins_task = list_to_task(new);
	return ( ins_task->priority > node_task->priority );
}


int scheduler_reschedule_task(task_t * task)
{
	assert_protected();
	if (task->state != TASK_READY || task->id < 0) return OSERR_VALUE;
	list_insert_condition(&ready_list, task_to_list(task), scheduler_insert_condition);

	task_t * current = get_current_task();
	if (task != current) {
	// if new task has higher priority than current, switch immediately
		if (task->priority > current->priority)
			kernel_context_switch_irq(NULL);
	// if new task has the same priority than current, start the timer
		else if (task->priority == current->priority)
			systimer_start_irq(&scheduler_timer);
	// if new task has lower priority than current, just insert it
	// else do nothing
	}
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

	systimer_stop_irq(&scheduler_timer);

	task_t * task = list_to_task(list_removeFront(&ready_list));
	// if task priority is the same as the next one in the list
	// go ahead and set the timer
	if (task && !list_empty(&ready_list) &&
		list_to_task(list_head(&ready_list))->priority == task->priority ) {
		systimer_start_irq(&scheduler_timer);
	}
	return task;
}

bool scheduler_task_ready(task_t * t)
{
	return list_contains(&ready_list, task_to_list(t));
}

#endif
