#include <malloc.h>
#include <stdio.h>
#include "task.h"
#include "stm32f7xx_hal.h"

/*
 This code is for testing malloc in the multitask
 environment.
 */

struct task_info {
	char * name;
	uint32_t alloc_size;
	uint32_t sleep_for;
};

static struct task_info task_info[] = { { .name = "Memory Thread 0",
		.alloc_size = 64, .sleep_for = 250 }, { .name = "Memory Thread 1",
		.alloc_size = 256, .sleep_for = 1500 }, { .name = "Memory Thread 2",
		.alloc_size = 1024, .sleep_for = 2000 }, { .name = "Memory Thread 3",
		.alloc_size = 4096, .sleep_for = 3000 }, { .name = "Memory Thread Fast",
		.alloc_size = 20, .sleep_for = 1000 }, };

extern uint32_t heap_size_get(void);
static void memt_func(void *context);
static void memt_func_fast(void *context);
static void report_heap(void * cxt);

void memory_thread_test(void) {
	task_create_schedule(report_heap, DEFAULT_STACK_SIZE, NULL, "Mem Heap");
	task_create_schedule(memt_func, DEFAULT_STACK_SIZE, (void*) &task_info[0],
			"Mem Func 0");
	task_create_schedule(memt_func, DEFAULT_STACK_SIZE, (void*) &task_info[1],
			"Mem Func 1");
	task_create_schedule(memt_func, DEFAULT_STACK_SIZE, (void*) &task_info[2],
			"Mem Func 2");
	task_create_schedule(memt_func, DEFAULT_STACK_SIZE, (void*) &task_info[3],
			"Mem Func 3");
	task_create_schedule(memt_func_fast, DEFAULT_STACK_SIZE,
			(void*) &task_info[4], "Mem Func Fast");
}

void report_heap(void * cxt) {
	(void) cxt;
	while (1) {
		uint32_t hsize = heap_size_get();
		printf("Heap Size: %u\n", hsize);
		task_sleep(2000);
	}
}

void memt_func(void *context) {
	struct task_info* ti = context;
	while (1) {
		uint32_t ticks = HAL_GetTick();
		void * allocated = malloc(ti->alloc_size);
		if (allocated) {
			printf("%s - allocated %u at %u ms\n", ti->name, ti->alloc_size,
					ticks);
		} else {
			printf("%s - FAILED to alloc %d at %u ms\n", ti->name,
					ti->alloc_size, ticks);
		}

		task_sleep(ti->sleep_for);
		if (allocated)
			free(allocated);
	}

}

void memt_func_fast(void *context) {
	struct task_info* ti = context;
	int counter = 0, repeat = 100;
	while (1) {
		void * allocated = malloc(ti->alloc_size);
		if (allocated)
			free(allocated);

		if (++counter == repeat) {
			counter = 0;
			uint32_t ticks = HAL_GetTick();

			if (allocated) {
				printf("%s - fast allocated %u, %d times at %u ms\n", ti->name,
						ti->alloc_size, repeat, ticks);
			} else {
				printf("%s - FAILED to allocate %d at %d ms\n", ti->name,
						ti->alloc_size, ticks);
			}

			task_sleep(ti->sleep_for);
		}
	}
}
