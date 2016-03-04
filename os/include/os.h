#ifndef __OS_H__
#define __OS_H__

#include "os_config.h"
//#include "system_stm32f7xx.h"

#define TIMESLICE (5)
//#define MS        (SystemCoreClock/1000)

void osInit(void);

#endif /* __OS_H__ */
