#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "stm32f746xx.h"

#define SPINLOCK_UNLOCKED 0
#define SPINLOCK_LOCKED   1

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
bool spinlock_try_lock(volatile uint32_t *l)
{
  const uint32_t unlocked = SPINLOCK_UNLOCKED;
  return atomic_compare_exchange_strong(l, &unlocked, SPINLOCK_LOCKED);
}

__attribute__ ((always_inline)) static inline
void spinlock_lock(volatile uint32_t *l)
{
  while (!spinlock_try_lock(l)) ;
}

__attribute__ ((always_inline)) static inline
void spinlock_unlock(volatile uint32_t *l)
{
  __DMB();
  *l = SPINLOCK_UNLOCKED;
}

#endif  /* __SPINLOCK_H__ */
