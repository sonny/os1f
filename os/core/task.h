#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "defs.h"
#include "list.h"
#include "task_type.h"

void task_main_hoist(void);

task_t * task_alloc(int stack_size);
task_t * task_create(int stack_size, const char * name);
task_t * task_frame_init(task_t *t, void (*func)(void*), void *context);
task_t * task_create_schedule(void (*)(void*), int, void*, const char*);

void task_free(task_t * t);
void task_join(task_t * t);
void task_schedule(task_t *task);
void task_delay(uint32_t ms);
void task_yield(void);
void task_start(int id);
void task_stop(int id);
void task_display(task_t * task);

void task_state_transition(task_t *t, task_action_e action);

uintptr_t task_get_sp(int id);
uintptr_t task_get_saved_pc(int id);
hw_stack_frame_t task_get_saved_hw_frame(int id);
sw_stack_frame_t task_get_saved_sw_frame(int id);

void assert_task_valid(task_t *t);
void assert_all_tasks_valid(void);


#endif /* __TASK_H__ */
