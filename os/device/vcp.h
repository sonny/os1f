#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "defs.h"
#include "event.h"

typedef enum {
	ASCII_ETX = 0x03,
	ASCII_EOT = 0x04,
	ASCII_BS  = 0x08,
	ASCII_LF  = 0x0A,
	ASCII_CR  = 0x0D,
	ASCII_ESC = 0x1B
} ascii_control;

void serialInit(void);
int os_puts_vcp(char *buffer, int len);
int os_gets_vcp(char *buffer, int len);

#endif  /* __SERIAL_H__ */
