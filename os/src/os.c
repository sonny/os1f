#include "stm32f7xx_hal.h"
#include "memory.h"
#include "task.h"
#include "display.h"

void osInit()
{
  HAL_Init();
  displayInit();

  // lowest priority 
  NVIC_SetPriority(PendSV_IRQn, 255);
  
#ifdef ENABLE_FP
  set_FPCCR( get_FPCCR() | FPCCR_LSPEN | FPCCR_ASPEN );
  set_CONTROL( get_CONTROL() | CONTROL_FPCA );
#endif /* ENABLE_FP */

  mem_init();
  task_init();

  SysTick_Config(HAL_RCC_GetHCLKFreq()/1000);
}

void SysTick_Handler(void)
{
  HAL_IncTick();

  // is it time for the current task to turn over its timeslice?
  const struct task *t = task_current();
  if (t && t->timeout_at <= HAL_GetTick())
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}
