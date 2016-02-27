#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

void *mem_alloc(size_t size);
void mem_init(void);

#endif /* __MEMORY_H__ */
