#ifndef __BITFIELD_H__
#define __BITFIELD_H__

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t bitfield_t;

__attribute__((always_inline)) static inline void bitfield_init(bitfield_t *b)
{
  *b = 0;
}

__attribute__((always_inline)) static inline void bitfield_set(bitfield_t *b, int idx)
{
  *b |= (1<<idx);
}

__attribute__((always_inline)) static inline void bitfield_clear(bitfield_t *b, int idx)
{
  *b &= ~(1<<idx);
}

__attribute__((always_inline)) static inline bool bitfield_is_set(bitfield_t b, int idx)
{
  return b & (1<<idx);
}


#endif  /* __BITFIELD_H__ */
