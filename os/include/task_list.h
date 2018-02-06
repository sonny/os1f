#ifndef __TASK_LIST_H__
#define __TASK_LIST_H__

#include "defs.h"

static inline
struct task * list_to_task(struct list * list)
{
  return (struct task *)list;
}

static inline
struct list * task_to_list(struct task * task)
{
  return (struct list *)task;
}




#endif /* __TASK_LIST_H__ */
