#ifndef __EVENT_H__
#define __EVENT_H__

#include "defs.h"
#include "svc.h"
#include "list.h"
#include "kernel.h"
#include "kernel_task.h"
#include "event_type.h"

static void event_wait_irq(void * cxt);
static void event_notify_irq(void *cxt);

static inline
void event_init(const char * name, event_t *e) {
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
	service_call(kernel_task_event_register, e, true);
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
		kernel_task_event_wait_current(e);
		protected_kernel_context_switch(NULL);
	}
	__enable_irq();
}

static inline
void event_notify_irq(void * cxt) {
	event_t *e = cxt;
	__disable_irq();
	e->signal = 1;
	kernel_task_event_notify_all(e);
	__enable_irq();
}

#endif  /* __EVENT_H__ */
