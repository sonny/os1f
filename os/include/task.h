#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>

struct task {
  uint32_t flags;
  void * psp;
};

extern struct task* current_task;
void taskNext(void);
void taskStart(void (*)(void*), int, struct task *); 

#endif /* __TASK_H__ */
