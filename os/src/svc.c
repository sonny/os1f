#include <stdint.h>
#include "svc.h"
#include "kernel.h"
#include "kernel_task.h"
#include "defs.h"

void service_call(void (*call)(void*), void *cxt)
{
  __asm volatile("svc 0\n");
}

void SVC_Handler(void)
{
  register uint32_t * frame;
  __asm volatile("mrs %0, psp \n" : "=r" (frame) ::);

  svcall_t call = (svcall_t)frame[0];
  void *args = (void*)frame[1];
  
  call(args);
}


