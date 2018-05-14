/*
 * scheduler_trivial.c
 *
 *  Created on: May 12, 2018
 *      Author: sonny
 */

#include "scheduler.h"

#if OS_SCHEDULER == SCHEDULER_TRIVIAL

#include "assertions.h"

static task_t * TCB[MAX_TASK_COUNT] = {0};
static TASK_STATIC_CREATE(idle_task, "Idle", IDLE_STACK_SIZE, 0);

static void scheduler_idle(void *ctx)
{
	(void)ctx;
	while(1) {
		task_delay(5);
	}
}

int scheduler_init(void)
{
	task_frame_init(&idle_task.task, scheduler_idle, NULL);
	scheduler_add_task(&idle_task);
}

int scheduler_add_task(task_t * task)
{
	static int next_id = 0;
	assert_protected();
	if (next_id >= MAX_TASK_COUNT) return OSERR_SPACE;
	task->id = next_id++;
	TCB[task->id] = task;
	return OS_OK;
}

// NOTE: does not make space available for more tasks
int scheduler_remove_task(task_t * task)
{
	assert_protected();
	TCB[task->id] = NULL;
	return OS_OK;
}

int scheduler_get_task(int id, task_t **task)
{
	if (id >= MAX_TASK_COUNT) return OSERR_VALUE;
	*task = TCB[id];
	return OS_OK;
}

int scheduler_reschedule_task(task_t * task)
{
	(void)task;
	/* does nothing */
}

task_t * scheduler_get_next_ready(void)
{
	static int current_id = -1;
	assert_protected();
	// find next active task
	do {
		current_id = (current_id + 1) % MAX_TASK_COUNT;
	} while (TCB[current_id] == NULL || TCB[current_id]->state != TASK_ACTIVE);

	return TCB[current_id];
}

void scheduler_task_each(void (*f)(task_t*))
{
	for (int i = 0; i < MAX_TASK_COUNT; ++i) {
		if (TCB[i]) {
			f->(TCB[i]);
		}
	}
}

#endif
