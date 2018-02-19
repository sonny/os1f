#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_adc.h"
#include "event.h"
#include "display.h"
#include "task.h"

static event_t adc_event = EVENT_STATIC_INIT(adc_event);
static ADC_HandleTypeDef    AdcHandle;
static uint32_t adc_values[2];
static void adc_task_init(void);

void adc_task(void *p)
{
  const uint16_t V25 = 943; // V25 = 0.76V, Vref = 3.3V
  //const uint16_t Avg_Slope = 3; // Avg_Slope = 2.5mV/C
  const float Avg_Slope = 2.5;
  float V, T;
  char rot[] = ".oOo";
  int rot_idx = 0;

  adc_task_init();

  while(1) {
    //HAL_ADC_Start_IT(&AdcHandle);
    HAL_ADC_Start_DMA(&AdcHandle, (uint32_t*)adc_values, 2);
    event_wait(&adc_event);
    // NOTE: T can actually differ up to 45 degrees from one chip to another;
    // it may be useful for relative temp, but not absolute
    T = ((adc_values[0]-V25)/Avg_Slope) + 25.0;

    // reference value is related to a 1.2V internal band gap (I don't know what this is)
    // the relation to the Vref is:
    V = (1.2 * 4095.0)/adc_values[1];
    // NOTE: this should be close to 3.0V for the F4-discovery

    //int v_entier = V / 1000;
    //int v_mant   = V % 1000;
    //double v = V/1000.0;

    //task_display_line("Temp: %.1f C, Vref: %.2f V %c\n", T, V, rot[rot_idx]);
    rot_idx = (rot_idx + 1) % 4;
    task_sleep(200);
  }
  
}

static void adc_task_init(void)
{
  //event_init(&adc_event);

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
  AdcHandle.Init.DMAContinuousRequests = ENABLE;
  AdcHandle.Init.EOCSelection          = DISABLE;

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
}

static void adc_nvic_init(void *p)
{
 /*## Configure the NVIC for DMA #########################################*/
  /* NVIC configuration for DMA transfer complete interrupt */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}

static DMA_HandleTypeDef  hdma_adc;
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
  /*## Enable peripherals and GPIO Clocks #################################*/
  __HAL_RCC_ADC1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /*## Configure the DMA streams ##########################################*/
  /* Set the parameters to be configured */
  hdma_adc.Instance = DMA2_Stream0; //ADCx_DMA_STREAM;
  hdma_adc.Init.Channel = DMA_CHANNEL_0; //ADCx_DMA_CHANNEL;
  hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
  hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_adc.Init.Mode = DMA_CIRCULAR;
  hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_adc.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  hdma_adc.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
  hdma_adc.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma_adc.Init.PeriphBurst = DMA_PBURST_SINGLE;

  HAL_DMA_Init(&hdma_adc);

  /* Associate the initialized DMA handle to the ADC handle */
  __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc);

  /*## Configure the NVIC for DMA #########################################*/
  /* NVIC configuration for DMA transfer complete interrupt */
  //  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  //  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  service_call(adc_nvic_init, NULL, false);
}


// NOTE : this handles all 3 of the ADCs
void DMA2_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler(AdcHandle.DMA_Handle);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1) {
    protected_event_notify(&adc_event);
  }
}
