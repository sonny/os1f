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

static task_t *TCB[MAX_TASK_COUNT] = {0};

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
	list_remove(task_to_list(task));
	return OS_OK;
}

task_t * task_control_get(int id)
{
	if (id >= MAX_TASK_COUNT || id < 0) return NULL;
	return TCB[id];
}

void task_control_each(void (*f)(task_t*))
{
	for (int i = 0; i < MAX_TASK_COUNT; ++i) {
		if (TCB[i])	f(TCB[i]);
	}
}
