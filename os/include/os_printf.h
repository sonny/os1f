#ifndef __OS_PRINTF_H__
#define __OS_PRINTF_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


#define printf   os_iprintf
#define os_sputs os_sputs_semihosting
#define os_sputc os_sputc_semihosting

int os_iprintf(const char *fmt, ...);
int os_vsniprintf(char * buff, size_t size, const char *fmt, va_list va);
int os_viprintf(const char *fmt, va_list va);
int os_itoa(int val, char *bf, int radix, bool is_unsigned);
int os_sputc_semihosting(char c);
int os_sputs_semihosting(const char *str, int len);


#endif /* __OS_PRINTF_H__ */
