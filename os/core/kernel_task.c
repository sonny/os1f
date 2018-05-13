#include <stdint.h>
#include "kernel.h"
#include "task.h"
#include "defs.h"
#include "list.h"
#include "service.h"
#include <string.h>
#include <assert.h>
#include "os_printf.h"
#include "heap.h"
#include "assertions.h"
#include "memory.h"
#include "systimer.h"
#include "task_control.h"
#include "scheduler.h"

static event_t *event_list[MAX_EVENT_COUNT] = {0};

// XXX: move to event
void kernel_task_event_wait_current(event_t * e)
{
	extern task_t * current_task;
	assert(current_task && "Current Task is NULL");
	assert_protected();
	current_task->state = TASK_WAIT;
	list_addAtRear(&e->waiting, task_to_list(current_task));
}

// XXX: move to event control
void kernel_task_event_notify_all(event_t * e)
{
	assert_protected();
	while (!list_empty(&e->waiting))
	{
		task_t * task = list_to_task(list_removeFront(&e->waiting));
		assert_task_sig(task);
		assert(task->state == TASK_WAIT && "Tasks in waiting queue must be waiting.");
		task->state = TASK_ACTIVE;
		scheduler_reschedule_task(task);
	}
}

// XXX: move to event control
void kernel_task_remove_join_event(task_t * t)
{
	for (int i = 0; i < MAX_EVENT_COUNT; ++i) {
		if (event_list[i] == &t->join) {
			assert(!event_task_waiting(&t->join) && "Someone is waiting on the join.");
			event_list[i] = NULL;
			break;
		}
	}
}


// XXX: move to event control
int kernel_task_in_wait(task_t * t)
{
	int i;
	int result = 0;
	for (i = 0; i < MAX_EVENT_COUNT; ++i) {
		event_t * e = event_list[i];
		if (e == NULL) continue;
		assert_event_sig(e);
		if (list_element_of(task_to_list(t), &e->waiting)) result++;
	}
	return result;
}


// XXX: move to event control
void kernel_task_event_register(void * ctx)
{
	event_t * new = ctx;

	assert_protected();
	assert_event_sig(new);
	int i;
	// ensure event is not in list
	for (i = 0; i < MAX_EVENT_COUNT; ++i) {
		if (event_list[i] == new) return; // nothing to do
	}

	// find spot for event
	for (i = 0; i < MAX_EVENT_COUNT; ++i) {
		if (event_list[i] == NULL) break; // found a spot
	}

	// if we get here, i is the new index
	assert(i < MAX_EVENT_COUNT && "Too many events.");
	event_list[i] = new;
}

// XXX: move to event control
void kernel_task_event_unregister(void * ctx)
{
	event_t * event = ctx;
	assert_protected();
	assert_event_sig(event);
	// find registration spot
	int i;
	for (i = 0; i < MAX_EVENT_COUNT; ++i) {
		if (event_list[i] == event) {
			event_list[i] = NULL;
			return;
		}
	}
}

