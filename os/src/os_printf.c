#include <stdbool.h>
#include <string.h>
#include "defs.h"
#include "os_printf.h"
#include "semihosting.h"



int os_iprintf(const char *fmt, ...)
{
  int len;
  va_list va;

  char *buffer = malloc(STDIO_BUFFER_SIZE);
  va_start (va, fmt);
  len = os_vsniprintf(buffer, STDIO_BUFFER_SIZE, fmt, va);
  va_end (va);

  os_sputs(buffer, len);
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

int os_viprintf(const char *fmt0, va_list va)
{
  char  *cp, *fmt = (char *)fmt0;;
  int l;
  char buffer[16] = {0};
  
  while (1) {
    cp = fmt;
    while (*fmt != '\0' && *fmt != '%') fmt++;
    if ((l = (fmt - cp)) != 0) {
      os_sputs(cp, l);
    }

    if (*fmt == '\0') goto done;
    fmt++; // this must be %
    
    switch (*fmt) {
    case 'c':
      os_sputc((char)va_arg(va, int));
      fmt++;
      break;
    case 's':
      cp = va_arg(va, char*);
      int l = strlen(cp);
      os_sputs(cp, l);
      fmt += l;
      break;
    case 'u':
    case 'd':
      l = os_itoa(va_arg(va, unsigned int), buffer, 10, (*fmt=='u'));
      os_sputs(buffer, l);
      fmt += l;
      break;
    case 'x':
    case 'X':
      l = os_itoa(va_arg(va, unsigned int), buffer, 16, true);
      os_sputs(buffer, l);
      fmt += l;
      break;
    default:
      os_sputc(*fmt);
      fmt++;
      break;
    }
  }
   
 done:
  return (fmt - fmt0);
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
    *(--p) = (digit < 10) ? '0' + digit : 'a' + digit;
    val /= radix;
      
  } while (val > 0);
  
  return num_digits;
}

int os_sputc_semihosting(char c)
{
  char out = c;
  call_host(SEMIHOSTING_SYS_WRITEC, &out);
}

int os_sputs_semihosting(const char *str, int len)
{
  uint32_t args[3] = {1, (uint32_t)str, len};
  call_host(SEMIHOSTING_SYS_WRITE, args);
}

