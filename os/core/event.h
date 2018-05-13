#ifndef __EVENT_H__
#define __EVENT_H__

#include "defs.h"
#include "service.h"
#include "list.h"
#include "kernel.h"
#include "event_type.h"
#include "event_control.h"
#include "task_control.h"
#include "task.h"
#include "assertions.h"
#include "scheduler.h"

static void event_wait_irq(void * cxt);
static void event_notify_irq(void *cxt);

static inline
void event_init(event_t * e, const char * name)
{
	e->name = name;
	list_init(&e->waiting);
	e->signature = EVENT_SIGNATURE;
}

static inline
void event_notify(event_t *e) {
	service_call(event_notify_irq, e, false);
}

static inline
void event_wait(event_t *e) {
	service_call((svcall_t)event_control_add, e, true);
	service_call(event_wait_irq, e, false);
	e->signal = 0;
}

static inline
bool event_task_waiting(event_t *e) {
	return !list_empty(&e->waiting);
}

static inline
void event_wait_irq(void * cxt) {
	event_t *e = cxt;
	__disable_irq();
	if (e->signal == 0) {
		task_t * current = task_control_get(get_task_id());
		current->state = TASK_WAIT;
		list_addAtRear(&e->waiting, task_to_list(current));
		protected_kernel_context_switch(NULL);
	}
	__enable_irq();
}

static inline
void event_notify_irq(void * cxt) {
	event_t *e = cxt;
	__disable_irq();
	e->signal = 1;

	while (!list_empty(&e->waiting))
	{
		task_t * task = list_to_task(list_removeFront(&e->waiting));
		assert_task_sig(task);
		assert(task->state == TASK_WAIT && "Tasks in waiting queue must be waiting.");
		task->state = TASK_ACTIVE;
		scheduler_reschedule_task(task);
	}
	__enable_irq();
}

#endif  /* __EVENT_H__ */
