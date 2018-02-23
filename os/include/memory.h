#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <malloc.h>

__attribute__((always_inline)) static inline
void * malloc_aligned(size_t size, int alignment)
{
  size_t offset = sizeof(void*) + alignment - 1;
  void * p = malloc(size + offset);
  void * r = (void**)( ((uintptr_t)p + offset) & ~(uintptr_t)(alignment - 1) );
  ((void**)r)[-1] = p; // store location of original pointer
  return r;
}

__attribute__((always_inline)) static inline
void free_aligned(void *p)
{
  free( ((void**)p)[-1] );
}

#endif /* __MEMORY_H__ */
