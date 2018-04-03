#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "defs.h"
#include "event.h"

void serialInit(void);
int os_puts_vcp(char *buffer, int len);
int os_gets_vcp(char *buffer, int len);

#endif  /* __SERIAL_H__ */
