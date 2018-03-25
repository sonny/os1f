#include <stdint.h>
#include "defs.h"
#include "kernel.h"
#include "kernel_task.h"
#include "display.h"
#include "task.h"
#include "usec_timer.h"

static volatile bool __os_started = false;

static IWDG_HandleTypeDef IwdgHandle;
static void kernel_watchdog_enable(void);


void os_start(void) {
	HAL_Init();
	display_init();
	usec_timer_init();

	HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

#ifdef ENABLE_FPU
	kernel_FPU_enable();
#endif /* ENABLE_FP */

	kernel_watchdog_enable();

	kernel_task_init();
	//NOTE: after here we are in user mode

	__os_started = true;
}

static void kernel_watchdog_enable(void)
{
	IwdgHandle.Init.Prescaler = IWDG_PRESCALER_32;
	IwdgHandle.Init.Reload    = 250; // 250 ms
	IwdgHandle.Init.Window    = IWDG_WINDOW_DISABLE;
	HAL_IWDG_Init(&IwdgHandle);
}

void kernel_watchdog_refresh(void)
{
	HAL_IWDG_Refresh(&IwdgHandle);
}

inline
bool os_started(void)
{
	return __os_started;
}

void assert_os_started(void)
{
	assert(__os_started && "OS has not started quite yet.");
}

void _exit( __attribute__((unused)) int code)
{
#if defined(DEBUG)
	__asm volatile ("bkpt 0");
#endif
	while(1) ;
}
