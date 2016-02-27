#ifndef __REGISTERS_H__
#define __REGISTERS_H__
/*
#include <stdint.h>
#include "devices.h"

// stolen more-or-less from ChibiOS

__attribute__( ( always_inline ) ) static inline void enable_irq(void)
{
  __asm volatile ("cpsie i" : : : "memory");
}

__attribute__( ( always_inline ) ) static inline void disable_irq(void)
{
  __asm volatile ("cpsid i" : : : "memory");
}


__attribute__( ( always_inline ) ) static inline uint32_t get_MSP(void)
{
  register uint32_t result;
  __asm volatile ("MRS %0, msp\n"  : "=r" (result) );
  return(result);
}

__attribute__( ( always_inline ) ) static inline void set_MSP(uint32_t mainStack)
{
  __asm volatile ("MSR msp, %0\n" : : "r" (mainStack) : "sp");
}

__attribute__( ( always_inline ) ) static inline uint32_t get_PSP(void)
{
  register uint32_t result;
  __asm volatile ("MRS %0, psp\n"  : "=r" (result) );
  return(result);
}

__attribute__( ( always_inline ) ) static inline void set_PSP(uint32_t processStack)
{
  __asm volatile ("MSR psp, %0\n" : : "r" (processStack) : "sp");
}

*/
__attribute__( ( always_inline ) ) static inline uint32_t get_SP(void)
{
  register uint32_t result;
  __asm volatile ("mov %0, sp\n"  : "=r" (result) );
  return(result);
}
/*
__attribute__( ( always_inline ) ) static inline void set_CONTROL(uint32_t ctl)
{
  __asm volatile ("MSR control, %0\n" : : "r" (ctl) );
}

__attribute__( ( always_inline ) ) static inline uint32_t get_CONTROL(void)
{
  register uint32_t result;
  __asm volatile ("MRS %0, control\n"  : "=r" (result) );
  return(result);
}

__attribute__( ( always_inline ) ) static inline uint32_t get_LR(void)
{
  register uint32_t result;
  __asm volatile ("mov %0, lr\n"  : "=r" (result) );
  return(result);
}

__attribute__( ( always_inline ) ) static inline void set_LR(uint32_t linkReg)
{
  __asm volatile ("mov lr, %0\n" : : "r" (linkReg) : "lr");
}

__attribute__( ( always_inline ) ) static inline uint32_t get_FP(void)
{
  register uint32_t result;
  __asm volatile ("mov %0, r7\n"  : "=r" (result) );
  return(result);
}

#ifdef ENABLE_FP
__attribute__( ( always_inline ) ) static inline uint32_t get_FPCCR(void)
{
  register uint32_t result;
   __asm volatile ("ldr %0, [%1]    \n"  : "=r" (result) : "r" (FPCCR) );
  return(result);
}

__attribute__( ( always_inline ) ) static inline void set_FPCCR(uint32_t val)
{
  __asm volatile ("str %0, [%1]    \n"  : : "r" (val), "r" (FPCCR) );
}

#define CONTROL_FPCA (1<<2)
#define FPCCR_ASPEN (1<<31)
#define FPCCR_LSPEN (1<<30)

#endif /* ENABLE_FP */

#endif  /* __REGISTERS_H__ */
