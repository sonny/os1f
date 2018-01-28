#include <stdint.h>
#include "kernel.h"
#include "task.h"
#include "defs.h"
#include "memory.h"

static void kernel_task(void *context);

void osInit(void)
{
  HAL_Init();

  // lowest priority 
  NVIC_SetPriority(PendSV_IRQn, 255);
  
#ifdef ENABLE_FP
  set_FPCCR( get_FPCCR() | FPCCR_LSPEN | FPCCR_ASPEN );
  set_CONTROL( get_CONTROL() | CONTROL_FPCA );
#endif /* ENABLE_FP */

  mem_init();
  taskStart(kernel_task, 128, NULL);
}

void osStart(void)
{
  // temporary stack space
  void * pspStart = mem_alloc(256) + 256;
 
  kernel_sync_barrier();
  kernel_PSP_set((uint32_t)pspStart);
  kernel_sync_barrier();
  kernel_CONTROL_set(0x01 << 1 | 0x01 << 0); // use PSP with unprivileged thread mode
  kernel_sync_barrier();

  syscall_start();
}

static void kernel_task(void *context)
{
  /* TODO: ensure atomicity */
  while (1) {
    // wake up sleeping tasks
    int i;
    /* TODO: handle rollover */
    uint32_t tick = HAL_GetTick();
    for (i = 0; i < TASK_COUNT; ++i) {
      struct task *t = taskGet(i);
      if (t->flags & TASK_SLEEP) {
        if (t->sleep_until < tick) {
          t->flags = TASK_ACTIVE;
          t->sleep_until = 0;
        }
      }
    }
    
    syscall_yield();
  }
}

void SysTick_Handler(void)
{
  HAL_IncTick();
  static int counter = 0;

  if (++counter > TIME_SLICE) {
    __asm volatile ("mov r0, 0" ::: "r0"); // SVC_YIELD
    PendSV_set();
    counter = 0;
  }
}

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
