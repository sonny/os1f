#ifndef __OS_DISPLAY_H__
#define __OS_DISPLAY_H__

void display_init(void);
void task_display_line(const char *fmt, ...);
void display_line_at(int line, const char* fmt, ...);

#ifdef OS_USE_SEMIHOSTING

#include "semihosting.h"

#define os_puts os_puts_semihosting
#define os_putc os_putc_semihosting

#define os_gets(s, n) (void)(s)

#elif defined(OS_USE_VCP)

#include "serial.h"
#include "vt100.h"

#define printf_at term_printf_at
#define vprintf_at term_vprintf_at
#define os_puts os_puts_vcp
#define os_putc os_putc_vcp

#define os_gets os_gets_vcp

#elif defined(OS_USE_LCD)

#include "lcd.h"
#include "stm32746g_discovery_lcd.h"

#define printf_at lcd_printf_at
#define vprintf_at lcd_vprintf_at

#define os_puts(s, n) (void)(s)
#define os_gets(s, n) (void)(s)

#endif


#endif  /* __OS_DISPLAY_H__ */
