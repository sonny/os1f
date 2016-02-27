#include "spinlock.h"
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

__attribute__ ((naked)) bool spinlock_try_lock(volatile uint32_t *l)
{
  __asm volatile("   mov   r1, #1       \n" 
                 "3: ldrex r2, [r0]     \n"
                 "   cmp   r2, r1       \n" // test if locked or unlocked
                 "   beq   1f           \n" // if locked give up
                 "   strex r2, r1, [r0] \n" // else, try to lock it
                 "   cmp   r2, r1       \n" // check if lock failed
                 "   beq   3b           \n" // if we could not lock it, but we get here,
                                            // try again, this means that it wasn't locked but
                                            // we could not get it atomically
                 "   dmb                \n" // ensures that all preceeding memory access
                                            // are finalized before proceeding
                 "   mov   r0, #1       \n" // return success
                 "   b     2f           \n" // jump to return
                 "1: mov   r0, #0       \n" // return failure
                 "2: bx    lr           \n" // return
                 : : 
                 );
}

void spinlock_lock(volatile uint32_t *l)
{
  while(!spinlock_try_lock(l)) ;
}

void spinlock_unlock(volatile uint32_t *l)
{
  __DMB();
  *l = SPINLOCK_UNLOCKED;
}
