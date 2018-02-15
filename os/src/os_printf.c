#include <stdbool.h>
#include <string.h>
#include "defs.h"
#include "os_printf.h"
#include "display.h"

int os_iprintf(const char *fmt, ...)
{
  va_list va;

  va_start (va, fmt);
  int len = os_viprintf(fmt, va);
  va_end (va);

  return len;

}

int os_viprintf(const char *fmt, va_list va)
{
  char *buffer = malloc(STDIO_BUFFER_SIZE);
  int len = os_vsniprintf(buffer, STDIO_BUFFER_SIZE, fmt, va);
  os_puts(buffer, len);
  free(buffer);
  return len;
}

int os_vsniprintf(char * buff, size_t size, const char *fmt, va_list va)
{
  char ch, *bp = buff, *cp;
  int m;
  uint32_t n;

  while ( ch = *fmt++ ) {
    if ('%' == ch) {
      
      switch(ch = *fmt++) {
      case 'c':
        *bp++ = (char)va_arg(va, int);
        if ((size_t)(bp - buff) >= size -1) goto DONE;
        break;
      case 's':
        cp = va_arg(va, char*);
        while (ch = *cp++) {
          *bp++ = ch;
          if ((size_t)(bp - buff) >= size -1) goto DONE;
        }
        break;
      case 'u':
      case 'd':
        if ((size_t)((bp - buff) + 11) >= size -1) goto DONE;
        n= va_arg(va, unsigned int);
        m = os_itoa(n, bp, 10, (*fmt=='u'));
        bp += m;
        break;
      case 'x':
      case 'X':
        if ((size_t)((bp - buff) + 8) >= size -1) goto DONE;
        n = va_arg(va, unsigned int);
        m = os_itoa(n, bp, 16, true);
        bp += m;
        break;
      default:
        *bp++ = ch;
        break;
      }
    }
    else {
      *bp++ = ch;
      if ((size_t)(bp - buff) >= size -1) goto DONE;
    }
  }

 DONE:
  *bp = '\0';
  return (bp - buff);
}

int os_itoa(int val, char *bf, int radix, bool is_unsigned)
{
  char *p = bf;
  if (val < 0 && !is_unsigned) {
    *p++ = '-';
    val = -val;
  }

  int num_digits = 0;
  int val_size = val;

  if (val_size == 0)
    num_digits = 1;
  else {
    while (val_size) {
      val_size /= radix;
      num_digits++;
    }
  }
  
  p += num_digits;
  do {
    int digit = val % radix;
    *(--p) = (digit < 10) ? '0' + digit : 'a' + digit - 10;
    val /= radix;
      
  } while (val > 0);
  
  return num_digits;
}


#ifdef OS_USE_SEMIHOSTING

int os_putc_semihosting(char c)
{
  char out = c;
  call_host(SEMIHOSTING_SYS_WRITEC, &out);
}

int os_puts_semihosting(const char *str, int len)
{
  uint32_t args[3] = {1, (uint32_t)str, len};
  call_host(SEMIHOSTING_SYS_WRITE, args);
}

#endif
