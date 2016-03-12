#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "os.h"
#include "event.h"

#ifndef VCP_BAUD
#define VCP_BAUD 115200
#endif

extern struct event *VCPCompleteEvent;

void serialInit(void);
void serial_register_event(struct event *e);

#endif  /* __SERIAL_H__ */
