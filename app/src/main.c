#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "os.h"

static void task_func(void *);
static void task_once(void *);
static void task_greedy(void *);
static void task_lcd_led(void *);
static void timer_counter(void *);
static void display_system_clock(void);

struct func_data {
	char * name;
	uint32_t sleep;
};

static struct func_data fdata[4] = {
		{ .name = "Simple 1", .sleep = 4000 },
		{ .name = "Simple 2", .sleep = 2000 },
		{ .name = "Simple 3", .sleep = 1000 },
		{ .name = "Simple 4", .sleep = 500 }, };

static uint32_t counters[] = {0, 0, 0, 0};

static volatile uint64_t USECtime = 0;
static volatile uint32_t MSECtime = 0;

extern void adc_task(void*);

int main(void) {
	// switch modes and make main a normal user task
	os_start();

	task_create_schedule(task_func, DEFAULT_STACK_SIZE, (void*) &fdata[2], fdata[2].name);
	task_create_schedule(task_func, DEFAULT_STACK_SIZE, (void*) &fdata[3], fdata[3].name);
	/* //  task_create_schedule(task_greedy, DEFAULT_STACK_SIZE, NULL, "Greedy 0"); */
	/* //  task_create_schedule(task_greedy, DEFAULT_STACK_SIZE, NULL, "Greedy 1"); */

	task_create_schedule(task_lcd_led, DEFAULT_STACK_SIZE, (void*) NULL, "LED LCD");

	systimer_create_exec(1000, (timer_callback)virtled_toggle, (void*)VLED0);
	systimer_create_exec(500, (timer_callback)virtled_toggle, (void*)VLED1);
	systimer_create_exec(250, (timer_callback)virtled_toggle, (void*)VLED2);
	systimer_create_exec(125, (timer_callback)virtled_toggle, (void*)VLED3);
	systimer_create_exec(65, (timer_callback)virtled_toggle, (void*)VLED4);
	systimer_create_exec(32, (timer_callback)virtled_toggle, (void*)VLED5);


	task_create_schedule(adc_task, 512, NULL, "ADC");
        
	display_system_clock();
	if (RCC->CSR & RCC_CSR_IWDGRSTF) {
		// Watchdog reset occurred
		lcd_printf_line(10, "Watchdog reset occurred.");
		__HAL_RCC_CLEAR_RESET_FLAGS();
	}
	else {
		// Watchdog reset did not occur
		lcd_printf_line(10, "Watchdog reset DID NOT occur.");
	}


	int tid = get_current_task_id();

	while (1) {
		uint32_t tick = HAL_GetTick();
		uint32_t rt_hours = tick / 3600000;
		uint32_t rt_hours_rem  = tick % 3600000;
		uint32_t rt_mins = rt_hours_rem / 60000;
		uint32_t rt_mins_rem = rt_hours_rem % 60000;
		uint32_t rt_secs = rt_mins_rem / 1000;
		uint32_t rt_msecs = rt_mins_rem % 1000;

		lcd_printf_line(tid, "[%d] Main Task Runtime - %d:%d:%d:%d", tid, rt_hours, rt_mins, rt_secs, rt_msecs);
		//lcd_printf_line(1, "Counters [%d] [%d] [%d] [%d]", counters[0], counters[1], counters[2], counters[3]);

		task_t * tonce = task_create_schedule(task_once, DEFAULT_STACK_SIZE, NULL, "Once");
		task_join(tonce);

		task_delay(500);
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
	lcd_printf_line(15, "System Clock : %d %s", clk, clk_str);
}


void task_func(void *context) {
	int k = 0;
	int tid = get_current_task_id();
	struct func_data * fdata = context;
	while (1) {
		++k;
		lcd_printf_line(tid, "[%d] Simple Task counter (%d delay) : %d", tid, fdata->sleep, k);
		task_delay(fdata->sleep);

	};
}

void task_once(void *context) {
	(void) context;
	uint32_t tick = HAL_GetTick();
	int tid = get_current_task_id();

	lcd_printf_line(9, "[%d] ONCE Task at %d ms", tid, tick);
}

static void task_greedy(void *ctx) {
	(void) ctx;
	volatile int k = 0;
	int tid = get_current_task_id();

	while (1) {
		++k;
		if ((k % 10000000) == 0) {
			lcd_printf_line(tid, "[%d] Simple Greedy counter [%d]", tid, k);
		}
	}
}

static void task_lcd_led(void *ctx)
{
	(void)ctx;
	int tid = get_current_task_id();

	while(1) {
		uint64_t tstart = usec_time();
		virtled_toggle(VLED11);
		uint64_t tend = usec_time();
		uint32_t tdiff = tend - tstart;

		lcd_printf_line(tid, "[%d] VLED toggle time: %dus", tid, tdiff);

		task_delay(1000);
	}
}

static void timer_counter(void *ctx)
{
	uint32_t * i = ctx;
	(*i)++;
}
