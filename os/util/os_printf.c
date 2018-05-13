#include <stdbool.h>
#include <string.h>
//#include <math.h>
#include "defs.h"
#include "os_printf.h"
#include "display.h"

int os_iprintf(const char *fmt, ...) {
	va_list va;

	va_start(va, fmt);
	int len = os_viprintf(fmt, va);
	va_end(va);

	return len;

}

int os_viprintf(const char *fmt, va_list va) {
	char *buffer = malloc(STDIO_BUFFER_SIZE);
	memset(buffer, 0, STDIO_BUFFER_SIZE);
	int len = os_vsniprintf(buffer, STDIO_BUFFER_SIZE, fmt, va);
	os_puts(buffer, len);
	free(buffer);
	return len;
}

int os_vsniprintf(char * buff, size_t size, const char *fmt, va_list va) {
	char ch, *bp = buff, *cp;
	int m;
	uint32_t n;
	long dec_precision = 1;

	while ((ch = *fmt++)) {
		if ('%' == ch) {
			SCAN: switch (ch = *fmt++) {
			case '.':
				dec_precision = strtol(fmt, &cp, 10);
				fmt = cp;
				goto SCAN;
				break;
			case 'c':
				*bp++ = (char) va_arg(va, int);
				if ((size_t) (bp - buff) >= size - 1)
					goto DONE;
				break;
			case 's':
				cp = va_arg(va, char*);
				while ((ch = *cp++)) {
					*bp++ = ch;
					if ((size_t) (bp - buff) >= size - 1)
						goto DONE;
				}
				break;
			case 'u':
			case 'd':
				if ((size_t) ((bp - buff) + 11) >= size - 1)
					goto DONE;
				n = va_arg(va, unsigned int);
				m = os_itoa(n, bp, 10, (*fmt == 'u'));
				bp += m;
				break;
			case 'x':
			case 'X':
				if ((size_t) ((bp - buff) + 8) >= size - 1)
					goto DONE;
				n = va_arg(va, unsigned int);
				m = os_itoa(n, bp, 16, true);
				bp += m;
				break;
			case 'f':
				if ((size_t) ((bp - buff) + 16) >= size - 1)
					goto DONE;
				float f = va_arg(va, double);
				bp += os_ftoa(f, bp, dec_precision);
				dec_precision = 1; //default
				break;
			default:
				*bp++ = ch;
				break;
			}
		} else {
			*bp++ = ch;
			if ((size_t) (bp - buff) >= size - 1)
				goto DONE;
		}
	}

	DONE: *bp++ = '\0';
	return (bp - buff);
}

static const int POW10[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000 };

int os_ftoa(float f, char *bf, int dec_precision) {
	char * p = bf;
	int mult = POW10[dec_precision];

	// put mantissa
	int n = (int) f;
	int m = os_itoa(n, p, 10, false);
	p += m;

	// put decimal
	if (dec_precision > 0) {
		*p++ = '.';
		n = (int) (((float) f * mult)) % mult;

		if (n) {
			m = os_itoa(n, p, 10, false);
			p += m;
		} else {
			while (dec_precision--) {
				*p++ = '0';
			}
		}
	}
	return (p - bf);
}

int os_itoa(int val, char *bf, int radix, bool is_unsigned) {
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
	int result = p - bf;
	do {
		int digit = val % radix;
		*(--p) = (digit < 10) ? '0' + digit : 'a' + digit - 10;
		val /= radix;

	} while (val > 0);

	return result;
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
