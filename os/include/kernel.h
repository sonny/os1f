#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "stm32f7xx_hal.h"
#include "defs.h"
#include "svc.h"

void os_start(void);

__attribute__ ((always_inline)) static inline
void protected_kernel_context_switch(void * ctx) {
	(void) ctx;
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

__attribute__ ((always_inline)) static inline
void kernel_context_switch(void) {
	service_call(protected_kernel_context_switch, NULL, false);
}

__attribute__ ((always_inline)) static inline
void kernel_critical_begin(void) {
	__asm volatile ("cpsid i" ::: "memory");
}

__attribute__ ((always_inline)) static inline
void kernel_critical_end(void) {
	__asm volatile ("cpsie i\n"
			"isb    \n"
			::: "memory");
}

__attribute__ ((always_inline)) static inline
uint32_t kernel_SP_get(void) {
	register uint32_t result;
	__asm volatile ("mov %0, sp\n" : "=r" (result) );
	return (result);
}

__attribute__ ((always_inline)) static inline
uint32_t kernel_PSP_get(void) {
	register uint32_t result;
	__asm volatile ("MRS %0, psp\n" : "=r" (result) );
	return (result);
}

__attribute__ ((always_inline)) static inline
void kernel_PSP_set(uint32_t sp) {
	__asm volatile ("MSR psp, %0\n" : : "r" (sp) : "sp");
}

__attribute__ ((always_inline)) static inline
void kernel_MSP_set(uint32_t sp) {
	__asm volatile ("MSR msp, %0\n" : : "r" (sp) : "sp");
}

__attribute__ ((always_inline)) static inline
void kernel_CONTROL_set(uint32_t ctl) {
	__asm volatile ("MSR control, %0\n" : : "r" (ctl) );
}

__attribute__( ( always_inline ))  static inline uint32_t kernel_CONTROL_get(void) {
	register uint32_t result;
	__asm volatile ("MRS %0, control\n" : "=r" (result) );
	return (result);
}

#ifdef ENABLE_FPU

__attribute__( ( always_inline ) ) static inline
void kernel_FPU_enable(void) {
	// enable CP10 and CP11 coprocessors in CPACR
	__asm volatile ("ldr r0, =0xE000ED88  \n"
			"ldr r1, [r0]         \n"
			"orr r1, #(0xf << 20) \n"
			"str r1, [r0]         \n"
			"dsb                  \n"
			:
			:
			: "r0", "r1");
}

#endif

__attribute__ ((always_inline)) static inline
void kernel_sync_barrier(void) {
	__asm volatile ("isb \n");
}

__attribute__ ((always_inline)) static inline
void kernel_break(void) {
	__asm volatile("BKPT #01");
}

#endif /* __KERNEL_H__ */
