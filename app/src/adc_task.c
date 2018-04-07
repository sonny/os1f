#include "os.h"

static event_t adc_event = EVENT_STATIC_INIT(adc_event);
static ADC_HandleTypeDef AdcHandle;
static DMA_HandleTypeDef hdma_adc;
static volatile uint32_t adc_values[2] =
{ 0 };

static void adc_task_init(void);

void adc_task(__attribute__((unused)) void *p)
{
//	const uint16_t V25 = 943;// V25 = 0.76V, Vref = 3.3V
	//const uint16_t Avg_Slope = 3; // Avg_Slope = 2.5mV/C
//	const float Avg_Slope = 25.0F;
	float V, T;
	char rot[] = ".oOo";
	int rot_idx = 0;

	const uint16_t const ts_cal1 = *((uint16_t*) (0x1FF0F44C)); // TS_CAL1
	const uint16_t const ts_cal2 = *((uint16_t*) (0x1FF0F44E)); // TS_CAL2
	// I'm off by 10 somewhere, the multiply by 10 is to correct for that
	const float t_avg_slope = (((float) ts_cal2 - ts_cal1) / 80.0F) * 10.0F; // 110 - 30
	const float ambient_offset = 16.0F;

	adc_task_init();
	int tid = kernel_task_id_current();

	while (1)
	{
		HAL_ADC_Start_IT(&AdcHandle);
		event_wait(&adc_event);
		// NOTE: T can actually differ up to 45 degrees from one chip to another;
		// it may be useful for relative temp, but not absolute
		// Calculate T at 25C using preset values, or Interpolate T from 30 to 110C using
		// registers preset at the factory. They work out to about the same.
		//T = ((adc_values[0] - V25) / Avg_Slope) + 25.0;
		T = (adc_values[0] - ts_cal1) / t_avg_slope + 30 - ambient_offset;

		// reference value is related to a 1.2V internal band gap (I don't know what this is)
		// the relation to the Vref is:
		V = (1.2 * 4095.0) / adc_values[1];
		// NOTE: this should be close to 3.0V for the F4-discovery

		lcd_printf_line(tid, "[%d] Temp: %.1f C, Vref: %.2f V %c", tid, T, V,
				rot[rot_idx]);
		rot_idx = (rot_idx + 1) % 4;
		task_sleep(250);
	}

}

static void adc_task_init(void)
{
	ADC_ChannelConfTypeDef sConfig[2];

	AdcHandle.Instance = ADC1;

	AdcHandle.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV4;
	AdcHandle.Init.Resolution = ADC_RESOLUTION_12B;
	AdcHandle.Init.ScanConvMode = ENABLE;
	AdcHandle.Init.ContinuousConvMode = DISABLE;
	AdcHandle.Init.DiscontinuousConvMode = DISABLE;
	AdcHandle.Init.NbrOfDiscConversion = 0;
	AdcHandle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; /* Conversion start trigged at each external event */
	AdcHandle.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
	AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	AdcHandle.Init.NbrOfConversion = 2;
	AdcHandle.Init.DMAContinuousRequests = DISABLE;
	AdcHandle.Init.EOCSelection = DISABLE;

	HAL_ADC_Init(&AdcHandle);

	sConfig[0].Channel = ADC_CHANNEL_TEMPSENSOR;
	sConfig[0].Rank = 1;
	sConfig[0].SamplingTime = ADC_SAMPLETIME_480CYCLES;
	sConfig[0].Offset = 0;

	sConfig[1].Channel = ADC_CHANNEL_VREFINT;
	sConfig[1].Rank = 2;
	sConfig[1].SamplingTime = ADC_SAMPLETIME_480CYCLES;
	sConfig[1].Offset = 0;

	HAL_ADC_ConfigChannel(&AdcHandle, &sConfig[0]);
	HAL_ADC_ConfigChannel(&AdcHandle, &sConfig[1]);
}

static void adc_nvic_init(__attribute__((unused)) void *p)
{
	HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ADC_IRQn);

}

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
	(void) hadc;
	/*## Enable peripherals and GPIO Clocks #################################*/
	__HAL_RCC_ADC1_CLK_ENABLE()	;
	service_call(adc_nvic_init, NULL, false);
}

void ADC_IRQHandler(void)
{
	HAL_ADC_IRQHandler(&AdcHandle);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	static int val_idx = 0;
	if (hadc->Instance == ADC1)
	{
		adc_values[val_idx] = HAL_ADC_GetValue(hadc);
		event_notify_irq(&adc_event);

		val_idx = (val_idx + 1) % 2;
	}
}
