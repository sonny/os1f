#include <stdint.h>
#include "kernel.h"
#include "task.h"
#include "defs.h"
#include "memory.h"

void osInit(void)
{
  /* Enable the CPU Cache */
  //CPU_CACHE_Enable();

  HAL_Init();

  // lowest priority 
  NVIC_SetPriority(PendSV_IRQn, 255);
  
#ifdef ENABLE_FP
  set_FPCCR( get_FPCCR() | FPCCR_LSPEN | FPCCR_ASPEN );
  set_CONTROL( get_CONTROL() | CONTROL_FPCA );
#endif /* ENABLE_FP */

  mem_init();
  //  int ticks_per_ms = HAL_RCC_GetHCLKFreq() / 1000;
  //  SysTick_Config(ticks_per_ms);
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
