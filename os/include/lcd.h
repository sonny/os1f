#ifndef __LCD_H__
#define __LCD_H__

#include <stdarg.h>

void lcdInit(void);
void lcd_vprintf_at(int xpos, int ypos, const char *fmt, va_list args);
void lcd_printf_at(int xpos, int ypos, const char *fmt, ...);


#endif  /* __LCD_H__ */
