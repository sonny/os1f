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

// The screen is a shared resource
// use a mutex to lock it
static struct mutex screen_lock;

// Save Cursor	<ESC>[s
static inline void vt100_save_cursor(void) {
  printf( ESC "[s");
}

// Unsave Cursor <ESC>[u
static inline void vt100_unsave_cursor(void) {
  printf( ESC "[u" );
}

// Cursor Down <ESC>[{COUNT}B
static inline void vt100_cursor_down(int count) {
  printf( ESC "[%dB", count);
}

// Erase Screen <ESC>[2J
static inline void vt100_erase_screen(void) {
  printf( ESC "[2J" );
}

// Home <ESC>[{row};{col}H
static inline void vt100_cursor_home(coord_t pos) {
  printf( ESC "[%d;%dH", pos.row, pos.col );
}

// scroll_screen <ESC>[{start};{end}r
static inline void vt100_scroll_screen(scroll_t scroll) {
  printf( ESC "[%d;%dr", scroll.start, scroll.end );
}

// <ESC>[?25l - hide cursor
static inline void vt100_hide_cursor(void) {
  printf( ESC "[?25l" );
}

// <ESC>[?25h - display cursor
static inline void vt100_show_cursor(void) {
  printf( ESC "[?25h" );
}

/**
 * Initialize termial
 * @return void
 */
void term_init(void) {
  mutex_init(&screen_lock);
  mutex_lock(&screen_lock);
  vt100_hide_cursor();
  vt100_erase_screen();
  mutex_unlock(&screen_lock);
}

/**
 * cleanup terminal
 */
void term_cleanup(void) {
  vt100_show_cursor();
  //vt100_erase_screen();
}

/**
 * Set scroll region
 * @param row starting row
 * @count number of lines to include in scrolling region
 */
void term_set_scroll(int row, int count) {
  vt100_scroll_screen((scroll_t){row, row + count});
  vt100_cursor_home((coord_t){row, 0});
}

/**
 * call printf at specified coordinate
 * @param c coordinate of first character
 * @param fmt printf format string
 * @param ... paramater list to printf
 */
void term_printf_at(coord_t c, const char *fmt, ...) {
  va_list arg;
  mutex_lock(&screen_lock);
  vt100_save_cursor();
  vt100_cursor_home(c);

  va_start(arg, fmt);	
  vprintf(fmt, arg);
  va_end(arg);
	
  vt100_unsave_cursor();
  //fflush(stdout);
  mutex_unlock(&screen_lock);
}
