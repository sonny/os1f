#include "memory.h"
#include "spinlock.h"

// 16KB
#define DYN_MEM_REGION_SIZE 16384
#define ALIGN_SIZE 8

/*
 * Local variables for tracking allocator.
 */

static void *_current;
static void *_end;
static uint8_t dynMemRegion[DYN_MEM_REGION_SIZE];

static volatile uint32_t lock;

static inline void mem_lock(void) { spinlock_lock(&lock); }

static inline void mem_unlock(void) { spinlock_unlock(&lock); }

void mem_init(void)
{
  lock = 0; // initialize lock
  mem_lock();
  _current = &(dynMemRegion[0]);
  _end = &(dynMemRegion[DYN_MEM_REGION_SIZE - 1]);
  mem_unlock();
}


// not protected by lock 
static inline bool _available(ptrdiff_t size)
{
  return (_end - _current) >= size;
}

void *mem_alloc(size_t size)
{
  size_t size_aligned = size % ALIGN_SIZE + size;

  void *new = NULL;
  mem_lock();
  if (_available(size_aligned))
    {
      new = _current;
      _current += size_aligned;
    }
  mem_unlock();
  return new;
}
