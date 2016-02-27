#include <stdio.h>
#include <stdarg.h>
#include "stm32746g_discovery_lcd.h"
#include "mutex.h"

static struct mutex screen_lock;

void lcd_printf_at(int xpos, int ypos, const char *fmt, ...)
{
  static char pbuff[128];
  va_list arg;
  mutex_lock(&screen_lock);

  va_start(arg, fmt);	
  vsnprintf(pbuff, 128, fmt, arg);
  va_end(arg);
	
  BSP_LCD_DisplayStringAt(xpos, ypos, pbuff, LEFT_MODE);
  mutex_unlock(&screen_lock);
}
