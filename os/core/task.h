#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "defs.h"
#include "list.h"
#include "task_type.h"

task_t * task_alloc(int stack_size);
task_t * task_init(task_t *t, const char * name, int id);
task_t * task_create(int stack_size, const char * name);
task_t * task_frame_init(task_t *t, void (*func)(void*), void *context);
task_t * task_create_schedule(void (*)(void*), int, void*, const char*);

void task_free(task_t * t);
void task_join(task_t * t);
void task_schedule(task_t *task);
void task_sleep(uint32_t ms);
void task_yield(void);

static inline
task_t * list_to_task(list_t * list)
{
	return (task_t *) list;
}

static inline
list_t * task_to_list(task_t * task)
{
	return (list_t *) task;
}

#endif /* __TASK_H__ */
