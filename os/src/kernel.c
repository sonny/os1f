#include <stdint.h>
#include "kernel.h"
#include "kernel_task.h"
#include "defs.h"
#include "task.h"
#include "syscall.h"
#include "memory.h"

void osInit(void)
{
  HAL_Init();

  // lowest priority 
  //NVIC_SetPriority(PendSV_IRQn, 255);
  //NVIC_SetPriority(SVC_IRQn, 254);
  
#ifdef ENABLE_FP
  set_FPCCR( get_FPCCR() | FPCCR_LSPEN | FPCCR_ASPEN );
  set_CONTROL( get_CONTROL() | CONTROL_FPCA );
#endif /* ENABLE_FP */

  mem_init();
  kernel_task_init();
}

void osStart(void)
{
  syscall_start();
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
void PendSV_Handler_C(void)
{
  kernel_critical_begin();

  kernel_task_update_local_SP();
  kernel_task_wakeup();
  kernel_task_active_next();
  kernel_task_update_global_SP();
    
  kernel_critical_end();
}


