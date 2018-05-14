#include <stdint.h>
#include "os.h"

static volatile bool __os_started = false;

static void os_services_start();

void os_start(void)
{
#ifdef DEBUG
	HAL_DBGMCU_EnableDBGStandbyMode();
	__HAL_DBGMCU_FREEZE_IWDG();
	__HAL_DBGMCU_FREEZE_TIM4();
	__HAL_DBGMCU_FREEZE_TIM9();
	__HAL_DBGMCU_FREEZE_TIM10();
#endif

	HAL_Init();
	display_init();

#ifdef SYSTIMER_ENABLE
	systimer_init();
#endif

	HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

#ifdef ENABLE_FPU
	kernel_FPU_enable();
#endif /* ENABLE_FP */

	task_control_init();
	task_main_hoist(); // sets the PSP
	// NOTE: NO service calls before this point
	// NOTE: after here we are in user mode

	__os_started = true;
	scheduler_init(); // scheduler uses service calls

	os_services_start();
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

static void os_services_start(void)
{

#ifdef SYSTIMER_ENABLE
	systimer_enable();
#endif

#ifdef WATCHDOG_ENABLE
	watchdog_init();
#endif /* WATCHDOG_ENABLE */

#ifdef SHELL_ENABLE
	shell_init();
#endif /* SHELL_ENABLE */

#ifdef VIRTLED_ENABLE
	virtled_init();
#endif

}

void _exit(__attribute__((unused)) int code)
{
#if defined(DEBUG)
	__asm volatile ("bkpt 0");
#endif
	while (1)
		;
}
