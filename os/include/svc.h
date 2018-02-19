#ifndef __SVC_H__
#define __SVC_H__

#include "defs.h"
#include <stdbool.h>

typedef void (*svcall_t)(void*);

void service_call(svcall_t, void *cxt, bool protected);

#endif  /* __SVC_H__ */
