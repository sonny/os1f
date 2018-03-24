#include "stm32f7xx_hal.h"

#include "usec_timer.h"

static volatile uint64_t usec_timer = 0;
static volatile TIM_HandleTypeDef TimHandle;
static const int period = 0xffff;

void usec_timer_init(void) {

	TimHandle.Instance = TIM7;
	TimHandle.Init.Period = period;
	TimHandle.Init.Prescaler = ((SystemCoreClock / 2) / ONE_MILLION) - 1;
	TimHandle.Init.ClockDivision = 0;
	TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;

	HAL_TIM_Base_Init(&TimHandle);

	HAL_TIM_Base_Start_IT(&TimHandle);
}

void HAL_TIM_PeriodElapsedCallback( __attribute__((unused)) TIM_HandleTypeDef *htim) {
	usec_timer += period;
}

uint64_t usec_time(void) {
	uint16_t tim_counter = __HAL_TIM_GetCounter(&TimHandle);
	return usec_timer + tim_counter;
}

void HAL_TIM_Base_MspInit( __attribute__((unused)) TIM_HandleTypeDef *htim) {
	/*##-1- Enable peripheral clock #################################*/
	/* TIMx Peripheral clock enable */
	__HAL_RCC_TIM7_CLK_ENABLE()
	;

	/*##-2- Configure the NVIC for TIMx ########################################*/
	/* Set the TIMx priority */
	//HAL_NVIC_SetPriority(TIM7_IRQn, 15, 0);

	/* Enable the TIMx global Interrupt */
	HAL_NVIC_EnableIRQ(TIM7_IRQn);
}

void TIM7_IRQHandler(void) {
	HAL_TIM_IRQHandler(&TimHandle);
}
