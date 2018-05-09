#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "board.h"

#define SPINLOCK_UNLOCKED 0
#define SPINLOCK_LOCKED   1

typedef uint32_t spinlock_t;

// see mutex implementation
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dht0008a/ch01s03s03.html
/*
 * How this works: ldrex and strex work together. strex has two condition for writing:
 * 1. it must use the same memory address as ldrex
 * 2. the value returned from ldrex must be the current value at the address
 * So even though the ldrex loop looks pointless, it is working together with strex
 * to ensure synchronization
 */

__attribute__ ((always_inline)) static inline
bool spinlock_locked_as(const volatile spinlock_t const *l, spinlock_t lock_value)
{
	return *l == lock_value;
}

__attribute__ ((always_inline)) static inline
bool spinlock_try_lock_value(volatile spinlock_t *l, spinlock_t lock_value)
{
	const spinlock_t unlocked = SPINLOCK_UNLOCKED;
	return atomic_compare_exchange_strong(l, &unlocked, lock_value);
}

__attribute__ ((always_inline)) static inline
bool spinlock_try_lock(volatile spinlock_t *l)
{
	return spinlock_try_lock_value(l, SPINLOCK_LOCKED);
}

__attribute__ ((always_inline)) static inline
void spinlock_lock(volatile spinlock_t *l)
{
	while (!spinlock_try_lock(l))
		;
}

__attribute__ ((always_inline)) static inline
void spinlock_unlock(volatile spinlock_t *l)
{
	__DMB();
	*l = SPINLOCK_UNLOCKED;
}

#endif  /* __SPINLOCK_H__ */
