#ifndef __OS_H__
#define __OS_H__

#include "board.h"
#include "os_config.h"
#include "stm32f7xx_hal.h"

//#include "system_stm32f7xx.h"

#define TIMESLICE (5)
//#define MS        (SystemCoreClock/1000)



void osInit(void);

/* wrap some of the HAL functions */
static inline uint32_t osGetTick(void) { return HAL_GetTick(); }

#endif /* __OS_H__ */
