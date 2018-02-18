#ifndef __KERNEL_TASK_H__
#define __KERNEL_TASK_H__

#include <stdint.h>
#include "defs.h"

void kernel_task_init(void);
void kernel_task_schedule(void);
void kernel_task_active_next(void);
void kernel_task_wakeup(void);
void kernel_task_start(task_t *);
void kernel_task_sleep(uint32_t);
void kernel_task_wait(uint32_t);
void kernel_task_update_global_SP(void);
void kernel_task_update_local_SP(void);
void kernel_task_event_wait(event_t *);
void kernel_task_event_notify(event_t *);
void kernel_task_save_context(int);
uint32_t kernel_task_load_context(void);

uint32_t current_task_id(void);

#endif  /* __KERNEL_TASK_H__ */
