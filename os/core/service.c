#include <stdint.h>
#include "service.h"
#include "kernel.h"
#include "kernel_task.h"
#include "defs.h"

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

