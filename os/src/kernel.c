#include <stdint.h>
#include "defs.h"
#include "kernel.h"
#include "kernel_task.h"
#include "serial.h"
#include "task.h"

extern void initialise_monitor_handles(void);

void os_start(void)
{
  HAL_Init();
  serialInit();
  initialise_monitor_handles();

/* #ifdef ENABLE_FP */
/*   set_FPCCR( get_FPCCR() | FPCCR_LSPEN | FPCCR_ASPEN ); */
/*   set_CONTROL( get_CONTROL() | CONTROL_FPCA ); */
/* #endif /\* ENABLE_FP *\/ */

  kernel_task_init();
  // after here we are in user mode
}

void SysTick_Handler(void)
{
  HAL_IncTick();
  static int counter = 0;

  if (++counter > TIME_SLICE) {
    counter = 0;
    kernel_PendSV_set();
  }
}


// called by ASM PendSV_Handler
//void PendSV_Handler_C(void)
void PendSV_Handler(void)
{
  kernel_critical_begin();
  kernel_task_save_context();

  kernel_task_update_local_SP();
  kernel_task_schedule();
  kernel_task_wakeup();
  kernel_task_active_next();
  kernel_task_update_global_SP();

  kernel_task_load_context();
  kernel_critical_end();
}


