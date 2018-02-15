/*
 * vt100.h
 *
 * Created: 7/19/2013 7:32:46 PM
 *  Author: Greg Cook
 *
 * Configurable terminal display with default layout
 */ 


#ifndef VT100_H_
#define VT100_H_

#include <stdint.h>
#include <stdarg.h>

typedef struct {
  uint16_t start;
  uint16_t end;
} scroll_t;

#define ESC           "\x1b"

void term_init(void);
void term_cleanup(void);
//void term_set_scroll(int line, int count);
int term_printf_at(int col, int row, const char *fmt, ...);
int term_vprintf_at(int col, int row, const char *fmt, va_list args);
#endif /* VT100_H_ */
