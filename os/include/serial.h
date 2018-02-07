#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "defs.h"
#include "event.h"

extern struct event *VCPCompleteEvent;

void serialInit(void);
int os_puts_vcp(char *buffer, int len);

#endif  /* __SERIAL_H__ */
