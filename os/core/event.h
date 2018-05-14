#ifndef __EVENT_H__
#define __EVENT_H__

#include "defs.h"
#include "service.h"
#include "list.h"
#include "kernel.h"
#include "event_type.h"
#include "event_control.h"
#include "task_control.h"
#include "task_type.h"
#include "assertions.h"
#include "scheduler.h"
#include "task.h"

static void event_wait_irq(void * cxt);
static void event_notify_irq(void *cxt);

static inline
void event_init(event_t * e, const char * name)
{
	e->id = -1;
	e->signal = 0;
	e->name = name;
	list_init(&e->waiting);
	e->signature = EVENT_SIGNATURE;
}

static inline
void event_notify(event_t *e) {
	service_call(event_notify_irq, e, true);
}

static inline
void event_wait(event_t *e) {
	service_call((svcall_t)event_control_add, e, true);
	service_call(event_wait_irq, e, true);
	e->signal = 0;
}

static inline
bool event_task_waiting(event_t *e) {
	return !list_empty(&e->waiting);
}

static inline
void event_wait_irq(void * cxt) {
	assert_protected();
	event_t *e = cxt;
	if (e->signal == 0) {
		task_t * current = get_current_task();
		task_state_transition(current, TA_WAIT);
		list_addAtRear(&e->waiting, task_to_list(current));
		kernel_context_switch_irq(NULL);
	}
}

static inline
void event_notify_irq(void * cxt) {
	assert_protected();
	event_t *e = cxt;
	e->signal = 1;

	while (!list_empty(&e->waiting))
	{
		task_t * task = list_to_task(list_removeFront(&e->waiting));
		assert_task_sig(task);
		assert(task->state == TASK_WAIT && "Tasks in waiting queue must be waiting.");
		task_state_transition(task, TA_NOTIFY);
		scheduler_reschedule_task(task);
	}
}

#endif  /* __EVENT_H__ */
