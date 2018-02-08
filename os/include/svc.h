#ifndef __SVC_H__
#define __SVC_H__

#include "defs.h"
#include "kernel.h"

typedef void (*svcall_t)(void*);

void service_call(svcall_t, void *cxt);
void pend_service_call(svcall_t, void *cxt);

#endif  /* __SVC_H__ */
