//#include "devices/osSysTick.h"
#include "stm32f7xx.h"
#include "stm32f7xx_hal_rcc.h"
#include "system.h"
#include "memory.h"
#include "task.h"
#include "registers.h"

void osInit()
{
  SysInit();

#ifdef ENABLE_FP
  set_FPCCR( get_FPCCR() | FPCCR_LSPEN | FPCCR_ASPEN );
  set_CONTROL( get_CONTROL() | CONTROL_FPCA );
#endif /* ENABLE_FP */
  mem_init();
  task_init();
  //SystickConfig(1*MS);
  SysTick_Config(HAL_RCC_GetHCLKFreq()/1000);
}

