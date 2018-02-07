#ifndef __OS_PRINTF_H__
#define __OS_PRINTF_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define printf   os_iprintf

#ifdef OS_USE_SEMIHOSTING

#define os_puts os_puts_semihosting
#define os_putc os_putc_semihosting

#elif defined(OS_USE_VCP)

#define os_puts os_puts_vcp
#define os_putc os_putc_vcp

#endif

int os_iprintf(const char *fmt, ...);
int os_vsniprintf(char * buff, size_t size, const char *fmt, va_list va);
int os_viprintf(const char *fmt, va_list va);
int os_itoa(int val, char *bf, int radix, bool is_unsigned);
int os_putc_semihosting(char c);
int os_puts_semihosting(const char *str, int len);


#endif /* __OS_PRINTF_H__ */
