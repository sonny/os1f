#if defined(BOARD_DISCOVERY)

#include <stdarg.h>
#include <ctype.h>
#include "stm32746g_discovery_lcd.h"
#include "defs.h"
#include "mutex.h"
#include "os_printf.h"

static mutex_t screen_lock = MUTEX_STATIC_INIT(screen_lock);

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
  char * buffer = malloc(STDIO_BUFFER_SIZE);
  int len = os_vsniprintf(buffer, STDIO_BUFFER_SIZE, fmt, args);
  
  mutex_lock(&screen_lock);
  BSP_LCD_DisplayStringAt(xpos, ypos, buffer, LEFT_MODE);
  mutex_unlock(&screen_lock);

  free(buffer);
}

void lcd_printf_at(int xpos, int ypos, const char *fmt, ...)
{
  va_list args;

  char *p = (char*)fmt;
  while(*p) {
    //if (*p == '\n' || *p == '\t') *p = ' ';
    if (iscntrl(*p)) *p = ' ';
    p++;
  }

  ypos = (ypos)*(BSP_LCD_GetFont()->Height + 4) + 5;

  va_start(args, fmt);	
  lcd_vprintf_at(xpos, ypos, fmt, args);
  va_end(args);
}

#endif
