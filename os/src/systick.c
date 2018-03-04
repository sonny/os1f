/*
 * systick.c
 *
 *  Created on: Mar 3, 2018
 *      Author: sonny
 */

#include "stm32f7xx_hal.h"
#include "kernel.h"
#include "task.h"

#define SYSTICK_RESOLUTION 5 // in ms

static uint32_t __systick;

/* override HAL_InitTick from stm32f7xx_hal.c */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
	/*Configure the SysTick to have interrupt in 5ms time basis*/
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / (1000 / SYSTICK_RESOLUTION));

	/*Configure the SysTick IRQ priority */
	HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority, 0);

	/* Return function status */
	return HAL_OK;
}

/* override HAL_InitTick from stm32f7xx_hal.c */
void HAL_IncTick(void) {
	__systick += SYSTICK_RESOLUTION;
}

/* override HAL_InitTick from stm32f7xx_hal.c */
inline
uint32_t HAL_GetTick(void) {
	return __systick;
}

/* override HAL_Delay from stm32f7xx_hal.c */
void HAL_Delay(__IO uint32_t Delay)
{
	uint32_t tickstart = HAL_GetTick();

	if (os_started())
	{
		task_sleep(Delay);
	}
	else
	{
		while ((HAL_GetTick() - tickstart) < Delay)
		{
		}
	}
}

void SysTick_Handler(void) {
	HAL_IncTick();

	if (os_started())
		protected_kernel_context_switch(NULL);
}

