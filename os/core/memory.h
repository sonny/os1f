#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <malloc.h>
#include <stdbool.h>
#include "board.h"

extern char _Heap_Begin; // Defined by the linker.
extern char __Main_Stack_Limit; // Defined by the linker.

__attribute__((always_inline)) static inline
bool dynamically_allocated(void *p)
{
	void * region_min = &_Heap_Begin;
	void * region_max = &__Main_Stack_Limit;
	return ((p >= region_min) && (p <= region_max));
}

extern char _sdata; // start and end of data
extern char _ebss; // start and end of bss

__attribute__((always_inline)) static inline
bool statically_allocated(void *p)
{
	void * region_min = &_sdata;
	void * region_max = &_ebss;
	return ((p >= region_min) && (p <= region_max));
}

__attribute__((always_inline)) static inline
void * malloc_aligned(size_t size, int alignment)
{
	size_t offset = sizeof(void*) + alignment - 1;
	void * p = malloc(size + offset);
	void * r = (void**) (((uintptr_t) p + offset) & ~(uintptr_t)(alignment - 1));
	((void**) r)[-1] = p; // store location of original pointer
	return r;
}

__attribute__((always_inline)) static inline
void free_aligned(void *p)
{
	void * r = ((void**) p)[-1];
	if (!statically_allocated(r)) free(r);
}



__attribute__((always_inline)) static inline
void os_free(void *p)
{
	//if (dynamically_allocated(p)) free(p);
	if (!statically_allocated(p)) free(p);
}

#endif /* __MEMORY_H__ */
