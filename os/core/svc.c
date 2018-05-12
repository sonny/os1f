#include <stdint.h>
#include "svc.h"
#include "kernel.h"
#include "kernel_task.h"
#include "defs.h"

//static uint32_t pendsv_stack[64];
//static uint32_t *pendsv_stack_ptr = &pendsv_stack[0];

// do not give compiler oportunity to optimize out
// the placement for the parameters to this function
__attribute__ ((noinline))
void service_call(svcall_t call, void *cxt, bool protected) {
	__asm volatile("svc 0\n");
}

void SVC_Handler(void) {
	register uint32_t * frame;
	__asm volatile("mrs %0, psp \n" : "=r" (frame) ::);

	svcall_t call = (svcall_t) frame[0];
	void *args = (void*) frame[1];
	bool protected = (bool) frame[2];

	if (protected) {
		__disable_irq();
		call(args);
		__enable_irq();
	} else {
		call(args);
	}
}

__attribute__ ((naked))
void PendSV_Handler(void) {
	uint32_t exc_return;
	__asm volatile("mov %0, lr \n" : "=r"(exc_return));

	__disable_irq();
	kernel_task_save_context_current(exc_return);
	kernel_task_update_runtime_current();

	kernel_task_save_PSP_current();
	kernel_task_schedule_current();
	kernel_task_active_next_current();

	assert_kernel_task_valid();

	kernel_task_load_PSP_current();

	kernel_task_update_lasttime_current();
	exc_return = kernel_task_load_context_current();
	__enable_irq();
	__asm volatile("bx %0 \n" :: "r"(exc_return));
}
