/*
 * task_control.c
 *
 *  Created on: May 12, 2018
 *      Author: sonny
 */

#include "defs.h"
#include "task_type.h"
#include "assertions.h"
#include "error.h"
#include "task_control.h"
#include "scheduler.h"

static TASK_STATIC_CREATE(idle_task, "Idle", IDLE_STACK_SIZE, IDLE_TASK_ID);

static task_t *TCB[MAX_TASK_COUNT] = {0};

void task_control_init(void)
{
	extern void scheduler_idle(void * ctx);
	task_frame_init(&idle_task.task, scheduler_idle, NULL);
}

int task_control_add(task_t * task)
{
	assert_protected();
	// find first available TCB slot
	int result = OSERR_SPACE;
	for (int i = 0; i < MAX_TASK_COUNT; ++i) {
		if (TCB[i] == NULL) {
			task->id = i;
			TCB[i] = task;
			result = i;
			break;
		}
	}
	return result;
}

int task_control_remove(task_t * task)
{
	assert_protected();
	TCB[task->id] = NULL;
	scheduler_unschedule_task(task);
	return OS_OK;
}

task_t * task_control_get(int id)
{
	if (id >= MAX_TASK_COUNT || id < IDLE_TASK_ID) return NULL;
	if (id == IDLE_TASK_ID) return &idle_task.task;
	return TCB[id];
}

void task_control_each(void (*f)(task_t*))
{
	for (int i = 0; i < MAX_TASK_COUNT; ++i) {
		if (TCB[i])	f(TCB[i]);
	}
}
