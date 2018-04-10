#ifndef __KERNEL_TASK_H__
#define __KERNEL_TASK_H__

#include <stdint.h>
#include "defs.h"
#include "event_type.h"
#include "task_type.h"

void kernel_task_init(void);
int32_t kernel_task_next_id(void);
void kernel_task_display_task_stats(void);
void kernel_task_event_notify_all(event_t *);
void kernel_task_wakeup_all(void);

void kernel_task_active_next_current(void);
void kernel_task_schedule_current(void);
void kernel_task_sleep_current(uint32_t);
void kernel_task_wait_current(uint32_t);
void kernel_task_event_wait_current(event_t *);
void kernel_task_save_context_current(int);
uint32_t kernel_task_load_context_current(void);
uint32_t kernel_task_id_current(void);
void kernel_task_update_lasttime_current(void);
void kernel_task_update_runtime_current(void);

void kernel_task_start_id(int);
void kernel_task_stop_id(int);

void kernel_task_start_task(task_t *);
void kernel_task_stop_task(task_t *);
void kernel_task_destroy_task(task_t *t);

void kernel_task_load_PSP_current(void);
void kernel_task_save_PSP_current(void);
void kernel_task_event_register(event_t *);
void assert_kernel_task_valid(void);

#endif  /* __KERNEL_TASK_H__ */
