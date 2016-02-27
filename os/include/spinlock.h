#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include <stdint.h>
#include <stdbool.h>

bool spinlock_try_lock(volatile uint32_t *l);
void spinlock_lock(volatile uint32_t *l);
void spinlock_unlock(volatile uint32_t *l);

#endif  /* __SPINLOCK_H__ */
