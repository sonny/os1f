#ifndef __VSNPRINTF_H__
#define __VSNPRINTF_H__

#include <stdarg.h>

int os_snprintf(char *buffer, size_t size, const char * fmt, ...);
int os_vsnprintf(char *str, size_t size, const char *format, va_list ap);

#endif /* __VSNPRINTF_H__ */
