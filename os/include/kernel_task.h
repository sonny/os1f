#ifndef __KERNEL_TASK_H__
#define __KERNEL_TASK_H__

#include <stdint.h>
#include "defs.h"

void kernel_task_init(void);
void kernel_task_active_next(void);
void kernel_task_wakeup(void);
void kernel_task_start(struct task * new);
void kernel_task_sleep(uint32_t ms);
void kernel_task_wait(uint32_t wait_state);
void kernel_task_update_global_SP(void);
void kernel_task_update_local_SP(void);
void kernel_task_event_wait(struct event * e);
void kernel_task_event_notify(struct event * e);
void kernel_task_remove(struct task *);

uint32_t current_task_id(void);

#endif  /* __KERNEL_TASK_H__ */
