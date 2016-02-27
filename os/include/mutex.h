#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <stdint.h>
#include <stdbool.h>
#include "task.h"
#include "bitfield.h"

struct mutex {
  volatile uint32_t lock;
  //uint8_t waiting[TASK_COUNT];
  bitfield_t waiting;
};

void mutex_init(struct mutex*);
void mutex_lock(struct mutex*);
void mutex_unlock(struct mutex*);

#endif /* __MUTEX_H__ */
