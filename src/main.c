#include <string.h>
//#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "stm32f7xx_hal.h"
#include "kernel.h"
#include "kernel_task.h"
#include "defs.h"
#include "task.h"
#include "display.h"
#include "shell.h"

//void printmsg(char *m);
static void task_func(void *);
static void task_once(void *);
static void task_greedy(void *);
static void display_system_clock(void);

struct func_data {
	char * name;
	uint32_t sleep;
};

static struct func_data fdata[4] = { { .name = "Task1", .sleep = 4000 }, {
		.name = "Task2", .sleep = 2000 }, { .name = "Task3", .sleep = 1000 }, {
		.name = "Task4", .sleep = 500 }, };

extern void adc_task(void*);
//extern void memory_thread_test(void);
//extern uint32_t heap_size_get(void);
//extern char _Heap_Begin, _Heap_Limit,_estack;

int main(void) {
	// switch modes and make main a normal user task
	os_start();

	task_create_schedule(task_func, DEFAULT_STACK_SIZE, (void*) &fdata[0],
			"Simple 0");
	task_create_schedule(task_func, DEFAULT_STACK_SIZE, (void*) &fdata[1],
			"Simple 1");
	//  task_create_schedule(task_greedy, DEFAULT_STACK_SIZE, NULL, "Greedy 0");
	//  task_create_schedule(task_greedy, DEFAULT_STACK_SIZE, NULL, "Greedy 1");

	/* // Unocmment to test memory allocation syncronization */
	/* // memory_thread_test(); */
	task_create_schedule(adc_task, 512, NULL, "ADC");

	shell_init();

	display_system_clock();

	int tid = kernel_task_id_current();
	uint32_t z = 0;
	while (1) {
		++z;
		lcd_printf_at(0, tid, "[%d] Main Task counter : %d", tid, z);

		task_t * tonce = task_create_schedule(task_once, DEFAULT_STACK_SIZE,
				NULL, "Once");

		task_join(tonce);

		task_sleep(500);
	}

	return 0;
}

extern uint32_t SystemCoreClock;
static void display_system_clock(void)
{
	uint32_t clk = SystemCoreClock;
	char * clk_str = "Hz";
	if (clk > 1000000) {
		clk /= 1000000;
		clk_str = "MHz";
	}
	else if (clk > 1000) {
		clk /= 1000;
		clk_str = "KHz";
	}
	lcd_printf_at(0, 15, "System Clock : %d %s", clk, clk_str);
}


void task_func(void *context) {
	int k = 0;
	int tid = kernel_task_id_current();
	struct func_data * fdata = context;
	while (1) {
		++k;
		lcd_printf_at(0, tid, "[%d] Simple Task counter : %d", tid, k);
		task_sleep(fdata->sleep);

	};
}

void task_once(void *context) {
	(void) context;
	uint32_t tick = HAL_GetTick();
	int tid = kernel_task_id_current();

	lcd_printf_at(0, 9, "[%d] ONCE Task at %d ms", tid, tick);
}

static void task_greedy(void *ctx) {
	(void) ctx;
	volatile int k = 0;
	int tid = kernel_task_id_current();

	while (1) {
		++k;
		if ((k % 10000000) == 0) {
			lcd_printf_at(0, tid, "[%d] Simple Greedy counter [%d]\n", tid, k);
		}
	}
}
