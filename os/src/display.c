#include "os.h"
#include "task.h"

#include <stdarg.h>


#ifdef DISPLAY_LCD
#include "lcd.h"
#include "stm32746g_discovery_lcd.h"
#elif defined DISPLAY_SERIAL
#include "serial.h"
#include "vt100.h"
#endif

static void __display_line_at(int line, const char* fmt, va_list args)
{
  int ypos;

#ifdef DISPLAY_LCD
  ypos = (line)*(BSP_LCD_GetFont()->Height + 4) + 5;
  lcd_vprintf_at(5, ypos, fmt, args);
#elif defined  DISPLAY_SERIAL
  ypos = line + 2;
  term_vprintf_at_wait(2, ypos, fmt, args);
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
#elif defined  DISPLAY_SERIAL
  serialInit();
  term_init();
#endif
}

