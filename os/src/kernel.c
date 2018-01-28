#include <stdint.h>
#include "kernel.h"
#include "task.h"
#include "defs.h"

void SVC_Handler(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3)
{
  switch (r0) {
  case SVC_YIELD:
    PendSV_set();
    break;
  case SVC_START:
    PendSV_set();
    break;
  }
}

// called by ASM PendSV_Handler
void pendsv_handler(uint32_t r0)
{
  struct task * new_task;
  
  switch (r0) {
  case SVC_YIELD:
    kernel_critical_begin();

    current_task->psp = (void*)kernel_PSP_get();
    taskNext();
    kernel_PSP_set((uint32_t)current_task->psp);
    
    kernel_critical_end();
    break;

  case SVC_START:
    kernel_critical_begin();

    taskNext();
    kernel_PSP_set((uint32_t)current_task->psp);

    kernel_critical_end();
    break;
  }
}
