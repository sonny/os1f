#include "os.h"
#include "task.h"
#include <stdarg.h>


#ifdef DISPLAY_LCD
#include "lcd.h"
#include "stm32746g_discovery_lcd.h"
#endif

static void __display_line_at(int line, const char* fmt, va_list args)
{
  int ypos;

#ifdef DISPLAY_LCD
  ypos = (line)*(BSP_LCD_GetFont()->Height + 4) + 5;
  lcd_vprintf_at(5, ypos, fmt, args);
#elif  DISPLAY_SERIAL
  ypos = line + 1;
  term_vprintf_at(1, ypos, fmt, args);
#endif

}

void os_display_line_at(int line, const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  __display_line_at(line, fmt, args);
  va_end(args);
}

void task_display_line(const char *fmt, ...)
{
  va_list args; 
  va_start(args, fmt);
  __display_line_at(task_current()->id, fmt, args);
  va_end(args);
}


void displayInit(void)
{
#ifdef DISPLAY_LCD
  lcdInit();
#elif  DISPLAY_SERIAL
  serialInit();
#endif
}

