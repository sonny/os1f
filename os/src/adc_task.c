#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_adc.h"
#include "sched.h"
#include "event.h"
#include "lcd.h"
#include "display.h"

static struct event adc_event;
static ADC_HandleTypeDef    AdcHandle;
static uint16_t adc_values[2];

void adc_task(void *p)
{
  int id = task_current()->id;
  int xpos = 10, ypos = (id*20 + 20);
  
  event_init(&adc_event);

  ADC_ChannelConfTypeDef sConfig[2];

  AdcHandle.Instance          = ADC1;
  
  AdcHandle.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV4;
  AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;
  AdcHandle.Init.ScanConvMode          = ENABLE;
  AdcHandle.Init.ContinuousConvMode    = DISABLE;
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;
  AdcHandle.Init.NbrOfDiscConversion   = 0;
  AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;        /* Conversion start trigged at each external event */
  AdcHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  AdcHandle.Init.NbrOfConversion       = 2;
  AdcHandle.Init.DMAContinuousRequests = DISABLE;
  AdcHandle.Init.EOCSelection          = ENABLE;

  HAL_ADC_Init(&AdcHandle);

  sConfig[0].Channel      = ADC_CHANNEL_TEMPSENSOR;
  sConfig[0].Rank         = 1;
  sConfig[0].SamplingTime = ADC_SAMPLETIME_480CYCLES;
  sConfig[0].Offset       = 0;

  sConfig[1].Channel      = ADC_CHANNEL_VREFINT;
  sConfig[1].Rank         = 2;
  sConfig[1].SamplingTime = ADC_SAMPLETIME_480CYCLES;
  sConfig[1].Offset       = 0;

  HAL_ADC_ConfigChannel(&AdcHandle, &sConfig[0]);
  HAL_ADC_ConfigChannel(&AdcHandle, &sConfig[1]);

  const uint16_t V25 = 943; // V25 = 0.76V, Vref = 3.3V
  const uint16_t Avg_Slope = 3; // Avg_Slope = 2.5mV/C
  uint16_t value;
  int V, T;
  while(1) {
    event_add_wait(&adc_event);
    HAL_ADC_Start_IT(&AdcHandle);
    yield();
    //value = HAL_ADC_GetValue(&AdcHandle);
    // NOTE: T can actually differ up to 45 degrees from one chip to another;
    // it may be useful for relative temp, but not absolute
    T = ((adc_values[0]-V25)/Avg_Slope) + 25;

    // reference value is related to a 1.2V internal band gap (I don't know what this is)
    // the relation to the Vref is:
    V = (1200*4095)/adc_values[1];
    // NOTE: this should be close to 3.0V for the F4-discovery

    int v_entier = V / 1000;
    int v_mant   = V % 1000;

    task_display_line("Temp: %2d C, Vref: %1d.%3d V", T, v_entier, v_mant);
    task_sleep(1000);
  }
  
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
  __HAL_RCC_ADC1_CLK_ENABLE();
  HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(ADC_IRQn);
}

// NOTE : this handles all 3 of the ADCs
void ADC_IRQHandler(void)
{
  HAL_ADC_IRQHandler(&AdcHandle);
}

// NOTE : this method relies on this code being faster than
//        the adc conversion (which it is). Use DMA for best results.
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  static int index = 0;
  if (hadc->Instance == ADC1) {
    adc_values[index] = HAL_ADC_GetValue(hadc);
    if (++index >= 2) {
      event_notify(&adc_event);
      index = 0;
    }
  }
}
