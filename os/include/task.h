#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>
#include "syscall.h"
#include "event.h"
#include "list.h"

typedef struct {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr; // r14
  uint32_t pc; // r15
  uint32_t xpsr;
} hw_stack_frame_t;

typedef struct {
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
  uint32_t r8;
  uint32_t r9;
  uint32_t r10;
  uint32_t r11;
} sw_stack_frame_t;

struct task {
  struct list node;
  void * sp;
  void * stack_free;
  int32_t  id;
  uint32_t state;
  uint32_t sleep_until;
  sw_stack_frame_t sw_context;
  struct event join;
};

struct task * task_create(int stack_size);
struct task * task_stack_init(struct task *t, void (*func)(void*), void *context);


__attribute__((always_inline)) static inline
void task_free(struct task * t)
{
  free(t->stack_free);
  free(t);
}

__attribute__((always_inline)) static inline
void task_schedule(struct task *task)
{
  syscall_task_start(task);
}

__attribute__((always_inline)) static inline
struct task * task_create_schedule(void (*func)(void*), int stack_size, void *context)
{
  struct task * t = task_create(stack_size);
  task_stack_init(t, func, context);
  task_schedule(t);
  return t;
}

__attribute__((always_inline)) static inline
void task_yield(void) 
{
  syscall_yield();
}

__attribute__((always_inline)) static inline
void task_sleep(uint32_t ms) 
{
  syscall_task_sleep(ms);
}

__attribute__((always_inline)) static inline
void task_join(struct task * t)
{
  event_wait(&t->join);
  task_free(t);
}

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


typedef struct {
  sw_stack_frame_t sw_frame;
  hw_stack_frame_t hw_frame;
} stack_frame_t;



#endif /* __TASK_H__ */
