/*
 * watchdog.c
 *
 *  Created on: Apr 1, 2018
 *      Author: sonny
 */

#include "stm32746g_discovery.h"
#include "task.h"
#include "display.h"

#define WATCHDOG_STACK_SIZE 128
static void watchdog_task(void *ctx);
static void watchdog_enable(void);
static void watchdog_refresh(void);
static uint32_t GetLSIFrequency(void);

static IWDG_HandleTypeDef IwdgHandle;

void watchdog_init(void)
{
	//uint32_t lsifreq = GetLSIFrequency();
	//lcd_printf_line(10, "LSI Freq %d", lsifreq);
	watchdog_enable();
	task_create_schedule(watchdog_task, WATCHDOG_STACK_SIZE, NULL, "Watchdog");
}

static void watchdog_task(void *ctx)
{
	(void)ctx;
	while (1)
	{
		watchdog_refresh();
		task_sleep(100);
	}
}

static void watchdog_enable(void)
{
	IWDG->KR = IWDG_KEY_ENABLE; // enable device (WO)
	IWDG->KR = IWDG_KEY_WRITE_ACCESS_ENABLE; // enable register write access (WO)
	IWDG->PR = IWDG_PRESCALER_32; // set prescaler
	IWDG->RLR = 250; // set reload register ms
	IWDG->KR = IWDG_KEY_RELOAD; // reload counter with reload value (WO)
}

static void watchdog_refresh(void)
{
	IWDG->KR = IWDG_KEY_RELOAD;
}

static volatile uint32_t uwPeriodValue = 0;
static volatile uint32_t uwCaptureNumber = 0;
static volatile uint32_t uwMeasurementDone = 0;
static TIM_HandleTypeDef  TimInputCaptureHandle;
static uint32_t GetLSIFrequency(void)
{
	uint32_t pclk1 = 0, latency = 0;
	TIM_IC_InitTypeDef timinputconfig =
	{ 0 };
	RCC_OscInitTypeDef oscinit =
	{ 0 };
	RCC_ClkInitTypeDef clkinit =
	{ 0 };

	/* Enable LSI Oscillator */
	oscinit.OscillatorType = RCC_OSCILLATORTYPE_LSI;
	oscinit.LSIState = RCC_LSI_ON;
	oscinit.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&oscinit) != HAL_OK)
	{
		//Error_Handler();
	}

	/* Configure the TIM peripheral */
	/* Set TIMx instance */
	TimInputCaptureHandle.Instance = TIM5;

	/* TIMx configuration: Input Capture mode ---------------------
	 The LSI clock is connected to TIM5 CH4.
	 The Rising edge is used as active edge.
	 The TIM5 CCR4 is used to compute the frequency value.
	 ------------------------------------------------------------ */
	TimInputCaptureHandle.Init.Prescaler = 0;
	TimInputCaptureHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
	TimInputCaptureHandle.Init.Period = 0xFFFF;
	TimInputCaptureHandle.Init.ClockDivision = 0;
	TimInputCaptureHandle.Init.RepetitionCounter = 0;
//	TimInputCaptureHandle.Init.AutoReloadPreload =
//			TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_IC_Init(&TimInputCaptureHandle) != HAL_OK)
	{
		/* Initialization Error */
		//Error_Handler();
	}
	/* Connect internally the  TIM5 CH4 Input Capture to the LSI clock output */
	HAL_TIMEx_RemapConfig(&TimInputCaptureHandle, TIM_TIM5_LSI);

	/* Configure the Input Capture of channel 4 */
	timinputconfig.ICPolarity = TIM_ICPOLARITY_RISING;
	timinputconfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
	timinputconfig.ICPrescaler = TIM_ICPSC_DIV8;
	timinputconfig.ICFilter = 0;

	if (HAL_TIM_IC_ConfigChannel(&TimInputCaptureHandle, &timinputconfig,
			TIM_CHANNEL_4) != HAL_OK)
	{
		/* Initialization Error */
		//Error_Handler();
	}
	if (HAL_TIM_IC_ConfigChannel(&TimInputCaptureHandle, &timinputconfig,
			TIM_CHANNEL_4) != HAL_OK)
	{
		/* Initialization Error */
		//Error_Handler();
	}

	/* Reset the flags */
	TimInputCaptureHandle.Instance->SR = 0;

	/* Start the TIM Input Capture measurement in interrupt mode */
	if (HAL_TIM_IC_Start_IT(&TimInputCaptureHandle, TIM_CHANNEL_4) != HAL_OK)
	{
		/* Starting Error */
		//Error_Handler();
	}

	/* Wait until the TIM5 get 2 LSI edges (refer to TIM5_IRQHandler() in
	 stm32f7xx_it.c file) */
	while (uwMeasurementDone == 0)
	{
	}
	uwCaptureNumber = 0;

	/* Deinitialize the TIM5 peripheral registers to their default reset values */
	HAL_TIM_IC_DeInit(&TimInputCaptureHandle);

	/* Compute the LSI frequency, depending on TIM5 input clock frequency (PCLK1)*/
	/* Get PCLK1 frequency */
	pclk1 = HAL_RCC_GetPCLK1Freq();
	HAL_RCC_GetClockConfig(&clkinit, &latency);
	HAL_RCC_GetClockConfig(&clkinit, &latency);

	/* Get PCLK1 prescaler */
	if ((clkinit.APB1CLKDivider) == RCC_HCLK_DIV1)
	{
		/* PCLK1 prescaler equal to 1 => TIMCLK = PCLK1 */
		return ((pclk1 / uwPeriodValue) * 8);
	}
	else
	{
		/* PCLK1 prescaler different from 1 => TIMCLK = 2 * PCLK1 */
		return (((2 * pclk1) / uwPeriodValue) * 8);
	}
}

static void tim_nvic_init(__attribute__((unused)) void *p)
{
	HAL_NVIC_SetPriority(TIM5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM5_IRQn);
}

void HAL_TIM_IC_MspInit(TIM_HandleTypeDef *htim)
{
  /* TIMx Peripheral clock enable */
	__HAL_RCC_TIM5_CLK_ENABLE();
	service_call(tim_nvic_init, NULL, false);
}

void TIM5_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&TimInputCaptureHandle);
}

static volatile uint16_t tmpCC4[2] = {0, 0};
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  /* Get the Input Capture value */
  tmpCC4[uwCaptureNumber++] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);

  if (uwCaptureNumber >= 2)
  {
    /* Compute the period length */
    uwPeriodValue = (uint16_t)(0xFFFF - tmpCC4[0] + tmpCC4[1] + 1);
    uwMeasurementDone = 1;
    uwCaptureNumber = 0;
  }
}
