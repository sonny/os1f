#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

static char * put_integer(char *str, int val, int base);
static int count_digits(int value, int base);

int os_vsnprintf(char *str, size_t size, const char *format, va_list ap) {

  char ch, char_tmp, *string_tmp, *r;
  int int_tmp, digit;
  size_t length = 0;
  
  while ( ch = *format++) {
    if ( '%' == ch ) {
      switch (ch = *format++) {
        /* %% - print out a single %    */
      case '%':
        *str++ = '%';
        length++;
        if (length >= size - 1)
          goto DONE;
        break;
        /* %c: print out a character    */
      case 'c':
        char_tmp = va_arg(ap, int);
        *str++ = char_tmp;
        length++;
        if (length >= size - 1)
          goto DONE;
        break;
        /* %s: print out a string       */
      case 's':
        string_tmp = va_arg(ap, char *);
        while(ch = *string_tmp++) {
          *str++ = ch;
          length++;
          if (length >= size - 1)
            goto DONE;
        }
        break;

        /* %d: print out an int (only positive)        */
      case 'd':
        int_tmp = va_arg(ap, int);
        if (length + 10 >= size - 1) // worst case
          goto DONE;
        r = put_integer(str, int_tmp, 10);
        length += (r - str);
        str = r;
        break;
        
        /* %x: print out an int in hex  */
      case 'x':
        int_tmp = va_arg(ap, int);
        if (length + 8 >= size - 1) // worst case
          goto DONE;

        r = put_integer(str, int_tmp, 16);
        length += (r - str);
        str = r;
        break;

        /* %x: print out a float in fixed  */
      case 'f':
        int_tmp = (int)(va_arg(ap, double) * 1000);
        if (length + 7 >= size - 1) // worst case
          goto DONE;

        int entier = int_tmp / 1000;
        int mantissa = int_tmp % 1000;
        r = put_integer(str, entier, 10);
        *r++ = '.';
        r = put_integer(r, mantissa, 10);
        length += (r - str);
        str = r;
        break;

        /* anything else */
      default:
        break;
      }
    }
    else { // copy from format string
      *str++ = ch;
      length++;
      if (length >= size - 1)
        goto DONE;
    }
  }

 DONE:
  *str = '\0';
  return length+1;
}

static int count_digits(int value, int base) {
  int count = 0;
  if (value == 0) return 1; // base case
  while (value) {
    value /= base;
    count++;
  }
  return count;
}

static char * put_integer(char *str, int val, int base) {
  int digit;
  char *p = str;
  int num_digits = count_digits(val, base);
  while(num_digits-- > 0) {
    digit = val % base;
    val /= base;
    if (digit < 10)
      *(str + num_digits) = (digit + 48);
    else
      *(str + num_digits) = (digit + 49);
    p++;
  }
  return p;
}

  
