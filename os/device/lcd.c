#include <stdarg.h>
#include <ctype.h>
#include "stm32746g_discovery_lcd.h"
#include "defs.h"
#include "mutex.h"
#include "os_printf.h"

static mutex_t screen_lock = MUTEX_STATIC_INIT(screen_lock);

void lcdInit(void) {
	/* LCD Initialization */
	BSP_LCD_Init();

	/* LCD Initialization */
	BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);

	/* Enable the LCD */
	BSP_LCD_DisplayOn();

	/* Select the LCD Background Layer  */
	BSP_LCD_SelectLayer(0);

	/* Clear the Background Layer */
	BSP_LCD_Clear(LCD_COLOR_BLACK);

	/* Configure the transparency for background */
	BSP_LCD_SetTransparency(0, 100);

	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
	BSP_LCD_SetFont(&Font12);
}

int lcd_vprintf_line(int line, const char *fmt, va_list args)
{
	uint8_t * buffer = malloc(STDIO_BUFFER_SIZE);
	int len = os_vsniprintf((char*)buffer, STDIO_BUFFER_SIZE, fmt, args);

	mutex_lock(&screen_lock);
	BSP_LCD_ClearStringLine(line);
	BSP_LCD_DisplayStringAtLine(line, buffer);
	mutex_unlock(&screen_lock);

	free(buffer);
	return len;
}

int lcd_vprintf_at(int xpos, int ypos, const char *fmt, va_list args) {
	uint8_t * buffer = malloc(STDIO_BUFFER_SIZE);
	int len = os_vsniprintf((char*)buffer, STDIO_BUFFER_SIZE, fmt, args);

	mutex_lock(&screen_lock);
	BSP_LCD_DisplayStringAt(xpos, ypos, buffer, LEFT_MODE);
	mutex_unlock(&screen_lock);

	free(buffer);
	return len;
}

int lcd_printf_line(int line, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	int len = lcd_vprintf_line(line, fmt, args);
	va_end(args);
	return len;

}

int lcd_printf_at(int xpos, int ypos, const char *fmt, ...) {
	va_list args;
	assert(ypos >= 0 && ypos <=16);

	ypos = (ypos) * (BSP_LCD_GetFont()->Height + 4) + 5;
	va_start(args, fmt);
	int len = lcd_vprintf_at(xpos, ypos, fmt, args);
	va_end(args);
	return len;
}

/* Overrides implementation in stm32746g_discovery_lcd.c */

void BSP_LCD_ClockConfig(LTDC_HandleTypeDef *hltdc, void *Params)
{
  //static RCC_PeriphCLKInitTypeDef  periph_clk_init_struct;

  /* RK043FN48H LCD clock configuration */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/5 = 38.4 Mhz */
  /* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_4 = 38.4/4 = 9.6Mhz */
//  periph_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
//  periph_clk_init_struct.PLLSAI.PLLSAIN = 192;
//  periph_clk_init_struct.PLLSAI.PLLSAIR = RK043FN48H_FREQUENCY_DIVIDER;
//  periph_clk_init_struct.PLLSAIDivR = RCC_PLLSAIDIVR_4;
//  HAL_RCCEx_PeriphCLKConfig(&periph_clk_init_struct);

  /* copied from stm32f7xx_hal_rcc_ex.c lines 578 - 671 */

  /*-------------------------------------- PLLSAI Configuration ---------------------------------*/
  /* PLLSAI is configured when a peripheral will use it as source clock : SAI1, SAI2, LTDC or CK48 */
  /* Disable PLLSAI Clock */
  __HAL_RCC_PLLSAI_DISABLE();

  /* Get Start Tick*/
  uint32_t tickstart = HAL_GetTick();

  /* Wait till PLLSAI is disabled */
  while(__HAL_RCC_PLLSAI_GET_FLAG() != RESET)
  {
    if((HAL_GetTick() - tickstart) > PLLSAI_TIMEOUT_VALUE)
    {
      /* return in case of Timeout detected */
      return;
    }
  }

  /*---------------------------- LTDC configuration -------------------------------*/
  /* Read PLLSAIP and PLLSAIQ value from PLLSAICFGR register (these value are not needed for LTDC configuration) */
  uint32_t tmpreg0 = ((RCC->PLLSAICFGR & RCC_PLLSAICFGR_PLLSAIQ) >> RCC_PLLSAICFGR_PLLSAIQ_Pos);
  uint32_t tmpreg1 = ((RCC->PLLSAICFGR & RCC_PLLSAICFGR_PLLSAIP) >> RCC_PLLSAICFGR_PLLSAIP_Pos);

  /* PLLSAI_VCO Input  = PLL_SOURCE/PLLM */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN */
  /* LTDC_CLK(first level) = PLLSAI_VCO Output/PLLSAIR */
  __HAL_RCC_PLLSAI_CONFIG(192 , tmpreg1, tmpreg0, RK043FN48H_FREQUENCY_DIVIDER);

  /* LTDC_CLK = LTDC_CLK(first level)/PLLSAIDIVR */
  __HAL_RCC_PLLSAI_PLLSAICLKDIVR_CONFIG(RCC_PLLSAIDIVR_4);

  /* Enable PLLSAI Clock */
  __HAL_RCC_PLLSAI_ENABLE();

  /* Get Start Tick*/
  tickstart = HAL_GetTick();

  /* Wait till PLLSAI is ready */
  while(__HAL_RCC_PLLSAI_GET_FLAG() == RESET)
  {
      if((HAL_GetTick() - tickstart) > PLLSAI_TIMEOUT_VALUE)
      {
        /* return in case of Timeout detected */
        return;
      }
  }
}

