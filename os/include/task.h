#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "defs.h"
#include "memory.h"
#include "svc.h"
#include "event.h"
#include "list.h"
#include "usec_timer.h"

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

typedef struct {
  uint32_t s[16]; // s0-s15
  uint32_t fpscr;
  uint32_t reserved;
} hw_fp_stack_frame_t;

typedef struct {
  uint32_t s[16]; // s16 - s31
} sw_fp_stack_frame_t;

struct task {
  list_t node;
  const char * name;
  void * stack_top;
  void * sp;
  int32_t  id;
  uint32_t state;
  uint32_t sleep_until;
  uint64_t runtime;
  uint64_t lasttime;
  sw_stack_frame_t sw_context;
#ifdef ENABLE_FPU
  sw_fp_stack_frame_t sw_fp_context;
  bool uses_fpu;
#endif
  event_t join;
  uint32_t exc_return;
} __attribute__((aligned(8)));

static inline
task_t * task_alloc(int stack_size)
{
  // ensure that eventual sp is 8 byte aligned
  size_t size = sizeof(task_t) + stack_size + (stack_size % 8) ;
  task_t * t = malloc_aligned(size, 8);
  memset(t, 0, size);
  t->sp = (void*)t + size;
  t->stack_top = t->sp;
  return t;
}

static inline
task_t * task_init(task_t *t, const char * name, int id)
{
  t->id = id;
  t->name = name;
  t->exc_return = 0xfffffffd;
  list_init(&t->node);
  event_init(&t->join);
  return t;
}

static inline
task_t * task_create(int stack_size, const char * name)
{
  task_t * t = task_alloc(stack_size);
  task_init(t, name, kernel_task_next_id());
  return t;
}

task_t * task_frame_init(task_t *t, void (*func)(void*), void *context);
void task_schedule(task_t *task);
void task_sleep(uint32_t ms);
void task_yield(void);

__attribute__((always_inline)) static inline
void task_free(task_t * t)
{
  assert(t->id > 0 && "Cannot free idle or main task.");
  kernel_task_destroy_task(t);
  free_aligned(t);
}

__attribute__((always_inline)) static inline
task_t * task_create_schedule(void (*func)(void*), int stack_size, void *context, const char * name)
{
  task_t * t = task_create(stack_size, name);
  task_frame_init(t, func, context);
  task_schedule(t);
  return t;
}


__attribute__((always_inline)) static inline
void task_join(task_t * t)
{
  event_wait(&t->join);
  task_free(t);
}

static inline
task_t * list_to_task(list_t * list)
{
  return (task_t *)list;
}

static inline
list_t * task_to_list(task_t * task)
{
  return (list_t *)task;
}


typedef struct {
  sw_stack_frame_t sw_frame;
  hw_stack_frame_t hw_frame;
} stack_frame_t;



#endif /* __TASK_H__ */
