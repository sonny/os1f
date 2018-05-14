/*
 * context_switch.c
 *
 *  Created on: May 12, 2018
 *      Author: sonny
 */

#include "board.h"
#include "defs.h"
#include "task_type.h"
#include "task.h"
#include "scheduler.h"
#include "systimer.h"

task_t * current_task = NULL;

static void save_PSP(void);
static void load_PSP(void);
static void update_runtime(void);
static void update_lasttime(void);
static uint32_t load_context(void);
static void save_context(int exc_return);

uint32_t get_current_task_id(void)
{
	return current_task->id;
}

task_t * get_current_task(void)
{
	return current_task;
}

#define FPU_USED(VAL) (!(VAL & (1 << 4)))

__attribute__ ((naked))
void PendSV_Handler(void)
{
	uint32_t exc_return;
	__asm volatile("mov %0, lr \n" : "=r"(exc_return));

	__disable_irq();
	save_context(exc_return);
	update_runtime();

	save_PSP();
	scheduler_reschedule_task(current_task);
	current_task = scheduler_get_next_ready();

	assert_all_tasks_valid();

	load_PSP();

	update_lasttime();
	exc_return = load_context();
	__enable_irq();
	__asm volatile("bx %0 \n" :: "r"(exc_return));
}

__attribute__ ((always_inline)) static inline
void save_PSP(void)
{
	assert(current_task && "Current task cannot be null");
	current_task->sp = (uint8_t*) __get_PSP();
}

__attribute__ ((always_inline)) static inline
void load_PSP(void)
{
	assert(current_task && "Current task cannot be null");
	__set_PSP((uint32_t) current_task->sp);
}

__attribute__ ((always_inline)) static inline
void update_runtime(void)
{
	uint64_t diff = usec_time() - current_task->lasttime;
	current_task->runtime += diff;
}

__attribute__ ((always_inline)) static inline
void update_lasttime(void)
{
	current_task->lasttime = usec_time();
}

__attribute__ ((always_inline)) static inline
uint32_t load_context(void)
{
	__asm volatile ("ldmia %0, {r4-r11} \n" :: "r" (&current_task->sw_context) :);

#ifdef ENABLE_FPU

	if (FPU_USED(current_task->exc_return))
	{
		__asm volatile ( "vldmia %0, {s16-s31} " :: "r" (&current_task->sw_fp_context) : );
	}

#endif

	return current_task->exc_return;
}

__attribute__ ((always_inline)) static inline
void save_context(int exc_return)
{
	__asm volatile ("stmia %0, {r4-r11} \n" :: "r" (&current_task->sw_context) :);
	current_task->exc_return = exc_return;

#ifdef ENABLE_FPU

	// check to see if task used FPU
	if (FPU_USED(exc_return))
	{
		__asm volatile ( "vstmia %0, {s16-s31} \n" :: "r" (&current_task->sw_fp_context) : );
	}

#endif
}
