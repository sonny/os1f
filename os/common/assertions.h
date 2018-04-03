/*
 * assertions.h
 *
 *  Created on: Mar 14, 2018
 *      Author: sonny
 */

#ifndef OS_INCLUDE_ASSERTIONS_H_
#define OS_INCLUDE_ASSERTIONS_H_

#include <assert.h>

void assert_os_started(void); // implemented in kernel.c


static inline
void assert_task_sig(task_t *t)
{
	assert(t->signature == 0xdeadbeef && "Invalid task signature.");
}

__attribute__ ((always_inline)) static inline
void assert_protected(void)
{
	uint32_t primask;
	__asm volatile("mrs %0, PRIMASK\n" : "=r"(primask));
	assert(primask & 1 && "Not in protected section.");
}

#endif /* OS_INCLUDE_ASSERTIONS_H_ */
