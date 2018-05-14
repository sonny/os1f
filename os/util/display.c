#include <stdarg.h>
#include "os_printf.h"
#include "display.h"

static void __display_line_at(int line, const char* fmt, va_list args) {
	int ypos = 0, xpos = line;

#ifdef OS_USE_SEMIHOSTING
	vprintf(fmt, args);

#elif OS_USE_LCD

	vprintf_at(xpos, ypos, fmt, args);

#elif defined  OS_USE_VCP
	ypos = line + 2;
	xpos = 2;

	vprintf_at(xpos, ypos, fmt, args);
#endif

}

void display_line_at(int line, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	__display_line_at(line, fmt, args);
	va_end(args);
}

void task_display_line(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	__display_line_at(get_current_task_id(), fmt, args);
	va_end(args);
}

extern void initialise_monitor_handles(void);

void display_init(void) {
#ifdef OS_USE_LCD
	lcdInit();
#endif

#if defined OS_USE_SEMIHOSTING
	initialise_monitor_handles();
#endif

#if defined  OS_USE_VCP
	serialInit();
#endif
}

