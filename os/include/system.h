#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stdint.h>
#include "system_stm32f7xx.h"

#define TIMESLICE (5)
#define MS        (SystemCoreClock/1000)

//extern volatile uint32_t systick;


void SysInit(void);
/* void SystickConfig(uint32_t ticks); */
/* void SystemClock_Config(void); */
/* void Error_Handler(void); */
/* void CPU_CACHE_Enable(void); */

#endif  /* __SYSTEM_H__ */
