/*
 * vt100.c
 *
 * Created: 7/19/2013 7:36:18 PM
 *  Author: Greg Cook
 *
 * Basic VT100 display functionality
 */ 

#include <stdio.h>
#include <stdarg.h>
#include "vt100.h"
#include "mutex.h"
#include "event.h"
#include "serial.h"

// The screen is a shared resource
// use a mutex to lock it
static struct mutex screen_lock;

// Save Cursor	<ESC>[s
static inline int vt100_save_cursor(char * restrict s) {
  return sprintf(s, ESC "[s");
}

// Unsave Cursor <ESC>[u
static inline int vt100_unsave_cursor(char * restrict s) {
  return sprintf(s, ESC "[u" );
}

// Cursor Down <ESC>[{COUNT}B
static inline int vt100_cursor_down(char * restrict s, int count) {
  return sprintf(s, ESC "[%dB", count);
}

// Erase Screen <ESC>[2J
static inline int  vt100_erase_screen(char * restrict s) {
  return sprintf(s, ESC "[2J" );
}

// Home <ESC>[{row};{col}H
static inline int vt100_cursor_home(char * restrict s, int col, int row) {
  return sprintf(s, ESC "[%d;%dH", row, col );
}

// scroll_screen <ESC>[{start};{end}r
static inline void vt100_scroll_screen(scroll_t scroll) {
  printf( ESC "[%d;%dr", scroll.start, scroll.end );
}

// <ESC>[?25l - hide cursor
static inline int vt100_hide_cursor(char * restrict s) {
  return sprintf(s, ESC "[?25l" );
}

// <ESC>[?25h - display cursor
static inline int vt100_show_cursor(char * restrict s) {
  return sprintf(s, ESC "[?25h" );
}

/**
 * Initialize termial
 * @return void
 */
void term_init(void) {
  mutex_init(&screen_lock);
  mutex_lock(&screen_lock);
  char pbuff[32];
  int offset = vt100_hide_cursor(pbuff);
  offset += vt100_erase_screen(pbuff+offset);
  printf(pbuff);
  mutex_unlock(&screen_lock);
}

/**
 * cleanup terminal
 */
void term_cleanup(void) {
  char pbuff[32];
  vt100_show_cursor(pbuff);
  printf(pbuff);
}

/**
 * Set scroll region
 * @param row starting row
 * @count number of lines to include in scrolling region
 */
/* void term_set_scroll(int row, int count) { */
/*   vt100_scroll_screen((scroll_t){row, row + count}); */
/*   vt100_cursor_home(0,row); */
/* } */


void term_vprintf_at_wait(int col, int row, const char *fmt, va_list args) {
  static char pbuff[128];

  mutex_lock(&screen_lock);

  int offset = vt100_save_cursor(pbuff);
  offset += vt100_cursor_home(pbuff + offset, col, row);
  offset += vsnprintf(pbuff + offset, 128 - offset, fmt, args);
  offset += vt100_unsave_cursor(pbuff + offset);

  event_subscribe(VCPCompleteEvent);
  printf(pbuff);
  event_wait(VCPCompleteEvent);

  mutex_unlock(&screen_lock);
}

/**
 * call printf at specified coordinate
 * @param c coordinate of first character
 * @param fmt printf format string
 * @param ... paramater list to printf
 */
void term_printf_at(int col, int row, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);	
  term_vprintf_at_wait(col, row, fmt, args);
  va_end(args);
	
}
