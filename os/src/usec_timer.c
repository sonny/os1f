#include "stm32f7xx_hal.h"

static uint32_t usec_timer = 0;
static TIM_HandleTypeDef    TimHandle;
static int period = 0xffff;

void usec_timer_init(void)
{

  TimHandle.Instance = TIM7;
  TimHandle.Init.Period            = period;
  TimHandle.Init.Prescaler         = ((SystemCoreClock / 2) / 1000000) - 1;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;

  HAL_TIM_Base_Init(&TimHandle);
  
  HAL_TIM_Base_Start_IT(&TimHandle);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  usec_timer++;
}

uint64_t usec_time(void)
{
  return (uint64_t)usec_timer * period + __HAL_TIM_GetCounter(&TimHandle);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  /*##-1- Enable peripheral clock #################################*/
  /* TIMx Peripheral clock enable */
  __HAL_RCC_TIM7_CLK_ENABLE();
  
  /*##-2- Configure the NVIC for TIMx ########################################*/
  /* Set the TIMx priority */
  HAL_NVIC_SetPriority(TIM7_IRQn, 3, 0);

  /* Enable the TIMx global Interrupt */
  HAL_NVIC_EnableIRQ(TIM7_IRQn);
}

void TIM7_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&TimHandle);
}
