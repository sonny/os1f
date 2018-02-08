#ifndef __SVC_H__
#define __SVC_H__

#include "defs.h"
#include "kernel.h"

typedef void (*svcall_t)(void*);

void service_call(void (*call)(void*), void *cxt);

#endif  /* __SVC_H__ */
