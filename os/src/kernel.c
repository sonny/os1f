#include <stdint.h>
#include "defs.h"
#include "kernel.h"
#include "kernel_task.h"
#include "display.h"
#include "task.h"
#include "usec_timer.h"

static bool __os_started = false;

void os_start(void) {
	HAL_Init();
	display_init();
	usec_timer_init();

#ifdef ENABLE_FPU
	kernel_FPU_enable();
#endif /* ENABLE_FP */

	kernel_task_init();
	//NOTE: after here we are in user mode

	__os_started = true;
}

inline
bool os_started(void)
{
	return __os_started;
}


