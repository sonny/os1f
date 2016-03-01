#include "spinlock.h"
#include "stm32f746xx.h"
#include <stdatomic.h>

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

__attribute__ ((naked)) bool spinlock_try_lock(volatile uint32_t *l)
{
  __asm volatile("   mov   r1, #1       \n" 
                 "   dmb                \n"
                 "2: ldrex r2, [r0]     \n"
                 "   cmp   r2, #0       \n" // test if locked or unlocked
                 "   bne   1f           \n" // if locked give up
                 "   strex r2, r1, [r0] \n" // else, try to lock it
                 "   cmp   r2, #0       \n" // check if lock failed
                 "   bne   2b           \n" // if we could not lock it, but we get here,
                                            // try again, this means that it wasn't locked but
                                            // we could not get it atomically
                 "1: dmb                \n" // ensures that all preceeding memory access
                                            // are finalized before proceeding
                 "   ite   eq           \n"
                 "   moveq r0, #1       \n" // return success
                 "   movne r0, #0       \n" // return failure
                 "   bx    lr           \n" // return
                 : : 
                 );
}

// this does *exactly* the same think as spinlock_try_lock
bool spinlock_test(uint32_t *l)
{
  const uint32_t unlocked = 0;
  return atomic_compare_exchange_strong(l, &unlocked, 1);
}


void spinlock_lock(volatile uint32_t *l)
{
  //  uint32_t q = 0;
  //spinlock_test(&q);
  while(!spinlock_try_lock(l)) ;
}

void spinlock_unlock(volatile uint32_t *l)
{
  __DMB();
  *l = SPINLOCK_UNLOCKED;
}

