#ifndef __LCD_H__
#define __LCD_H__

#include <stdarg.h>

void lcdInit(void);
int lcd_vprintf_at(int xpos, int ypos, const char *fmt, va_list args);
int lcd_printf_at(int xpos, int ypos, const char *fmt, ...);
int lcd_vprintf_line(int line, const char *fmt, va_list args);
int lcd_printf_line(int line, const char *fmt, ...);

#endif  /* __LCD_H__ */
