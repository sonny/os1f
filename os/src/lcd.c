#include <stdio.h>
#include <stdarg.h>
#include "stm32746g_discovery_lcd.h"
#include "mutex.h"

static struct mutex screen_lock;

void lcdInit(void)
{
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
  BSP_LCD_SetFont(&Font16);
}


void lcd_vprintf_at(int xpos, int ypos, const char *fmt, va_list args)
{
  static char pbuff[128];

  mutex_lock(&screen_lock);
  vsnprintf(pbuff, 128, fmt, args);
  BSP_LCD_DisplayStringAt(xpos, ypos, pbuff, LEFT_MODE);
  mutex_unlock(&screen_lock);
}

void lcd_printf_at(int xpos, int ypos, const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);	
  lcd_vprintf_at(xpos, ypos, fmt, args);
  va_end(args);
}
