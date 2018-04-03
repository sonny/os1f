#ifndef __OS_PRINTF_H__
#define __OS_PRINTF_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#define printf   os_iprintf
#define vprintf  os_viprintf

int os_iprintf(const char *fmt, ...);
int os_vsniprintf(char * buff, size_t size, const char *fmt, va_list va);
int os_viprintf(const char *fmt, va_list va);
int os_itoa(int val, char *bf, int radix, bool is_unsigned);
int os_ftoa(float f, char *bf, int);
int os_putc_semihosting(char c);
int os_puts_semihosting(const char *str, int len);

#endif /* __OS_PRINTF_H__ */
