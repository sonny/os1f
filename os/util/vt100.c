/*
 * vt100.c
 *
 * Created: 7/19/2013 7:36:18 PM
 *  Author: Greg Cook
 *
 * Basic VT100 display functionality
 */

//#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <vcp.h>
#include "vt100.h"
#include "display.h"
#include "os_printf.h"

// Save Cursor	<ESC>[s
static inline
int vt100_save_cursor(void) {
	os_puts(ESC "[s", 3);
	return 3;
}

// Unsave Cursor <ESC>[u
static inline
int vt100_unsave_cursor(void) {
	os_puts( ESC "[u", 3);
	return 3;
}

// Cursor Down <ESC>[{COUNT}B
static inline
int vt100_cursor_down(int count) {
	return printf( ESC "[%dB", count);
}

// Erase Screen <ESC>[2J
static inline
int vt100_erase_screen(void) {
	os_puts( ESC "[2J", 4);
	return 4;
}

// Home <ESC>[{row};{col}H
static inline
int vt100_cursor_home(int col, int row) {
	return printf( ESC "[%d;%dH", row, col);
}

// <ESC>[?25l - hide cursor
static inline
int vt100_hide_cursor(void) {
	os_puts( ESC "[?25l", 6);
	return 6;
}

// <ESC>[?25h - display cursor
static inline
int vt100_show_cursor(void) {
	os_puts( ESC "[?25h", 6);
	return 6;
}

/**
 * Initialize termial
 * @return void
 */
void term_init(void) {
	vt100_hide_cursor();
	vt100_erase_screen();
}

/**
 * cleanup terminal
 */
void term_cleanup(void) {
	vt100_show_cursor();
}

int term_vprintf_at(int col, int row, const char *fmt, va_list args) {

	int len = vt100_save_cursor();
	len += vt100_cursor_home(col, row);

	len += os_viprintf(fmt, args);

	len += vt100_unsave_cursor();
	return len;
}

/**
 * call printf at specified coordinate
 * @param c coordinate of first character
 * @param fmt printf format string
 * @param ... paramater list to printf
 */
int term_printf_at(int col, int row, const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	int len = term_vprintf_at(col, row, fmt, args);
	va_end(args);

	return len;
}
