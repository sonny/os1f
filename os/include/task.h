#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include "kernel.h"
#include "defs.h"

struct task {
  void * psp;
  uint32_t flags;
  uint32_t sleep_until;  
};

extern struct task* current_task;
void taskNext(void);
void taskStart(void (*)(void*), int, struct task *); 

void * taskGet(int i);


__attribute__((always_inline)) static inline void taskSleepUntil(uint32_t ms) 
{
  /* TODO: ensure atomicity */
  current_task->sleep_until = ms;
  current_task->flags = TASK_SLEEP;
  syscall_yield();
}

__attribute__((always_inline)) static inline void taskSleep(uint32_t ms) 
{
  taskSleepUntil(HAL_GetTick() + ms);
}


#endif /* __TASK_H__ */
