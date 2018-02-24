#include <stdint.h>
#include "defs.h"
#include "kernel.h"
#include "kernel_task.h"
#include "display.h"
#include "task.h"
#include "usec_timer.h"

void os_start(void)
{
  HAL_Init();
  display_init();
  usec_timer_init();
  
  #ifdef ENABLE_FPU
  kernel_FPU_enable();
  #endif /* ENABLE_FP */
  
  kernel_task_init();
  //NOTE: after here we are in user mode
}

void SysTick_Handler(void)
{
  HAL_IncTick();
  static int counter = 0;

  if (++counter > TIME_SLICE) {
    counter = 0;
    protected_kernel_context_switch(NULL);
  }
}


