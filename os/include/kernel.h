#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "stm32f7xx_hal.h"

void osInit(void);
void osStart(void);

static inline void __attribute__ ((always_inline))
kernel_critical_begin(void)
{
  __asm volatile ("cpsid i" ::: "memory");
}

static inline void __attribute__ ((always_inline))
kernel_critical_end(void)
{
  __asm volatile ("cpsie i\n"
                  "isb    \n"
                  ::: "memory");
}

static inline uint32_t __attribute__ ((always_inline))
kernel_PSP_get(void)
{
  register uint32_t result;
  __asm volatile ("MRS %0, psp\n"  : "=r" (result) );
  return(result);
}

static inline void __attribute__ ((always_inline))
kernel_PSP_set(uint32_t sp)
{
  __asm volatile ("MSR psp, %0\n" : : "r" (sp) : "sp");
}

static inline void __attribute__ ((always_inline))
kernel_CONTROL_set(uint32_t ctl)
{
  __asm volatile ("MSR control, %0\n" : : "r" (ctl) );
}

static inline void __attribute__ ((always_inline))
kernel_sync_barrier(void)
{
  __asm volatile ("isb \n");
}

static inline void __attribute__ ((always_inline))
syscall_start(void)
{
  __asm volatile ("mov r0, 0x00010000\n"
                  "svc 0             \n" ::: "r0", "lr" );
}

static inline void __attribute__ ((always_inline))
syscall_yield(void)
{
  __asm volatile ("mov r0, 0x00000000\n"
                  "svc 0             \n" ::: "r0", "lr" );
}


static inline void __attribute__ ((always_inline))
PendSV_set(void)
{
  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

#endif /* __KERNEL_H__ */
