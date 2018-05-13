/*
 * event_control.c
 *
 *  Created on: May 13, 2018
 *      Author: sonny
 */

#include "defs.h"
#include "event_type.h"
#include "task_type.h"
#include "task.h"
#include "error.h"
#include "assertions.h"


static event_t *event_list[MAX_EVENT_COUNT] = {0};

void event_control_init(void);

int event_control_index(event_t * event)
{
	for (int i = 0; i < MAX_EVENT_COUNT; ++i) {
		if (event_list[i] == event)
			return i;
	}
	return -1;
}

int event_control_add(event_t * event)
{
	assert_protected();
	assert_event_sig(event);

	if (event_control_index(event) > -1) return OS_OK;
	int idx = event_control_index(NULL);
	if (idx == -1) return OSERR_SPACE;
	event_list[idx] = event;

//	int i;
//	// ensure event is not in list
//	for (i = 0; i < MAX_EVENT_COUNT; ++i) {
//		if (event_list[i] == event) return; // nothing to do
//	}

	// find spot for event
//	for (i = 0; i < MAX_EVENT_COUNT; ++i) {
//		if (event_list[i] == NULL) break; // found a spot
//	}

	// if we get here, i is the new index
//	assert(i < MAX_EVENT_COUNT && "Too many events.");
//	event_list[i] = new;
	return OS_OK;
}

int event_control_remove(event_t * event)
{
	assert_protected();
	assert_event_sig(event);
	// find registration spot
	int i;
	for (i = 0; i < MAX_EVENT_COUNT; ++i) {
		if (event_list[i] == event) {
			event_list[i] = NULL;
			return OS_OK;
		}
	}
	return OS_OK;
}

void event_control_each(void (*f)(event_t*));

int event_control_task_waiting(task_t *t)
{
	int result = 0;
	for (int i = 0; i < MAX_EVENT_COUNT; ++i) {
		event_t * e = event_list[i];
		if (e == NULL) continue;
		assert_event_sig(e);
		if (list_element_of(task_to_list(t), &e->waiting)) result++;
	}
	return result;

}

