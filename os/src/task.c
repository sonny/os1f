#include <string.h>
#include <stdlib.h>
#include "task.h"
#include "defs.h"

// implemented in kernel_task
extern void kernel_task_end(void);

struct task * task_stack_init(struct task *t, void (*func)(void*), void *context)
{
  struct regs *r = t->stackp - sizeof(struct regs);
  r->stacked.r0 = (uint32_t)context;
  r->stacked.pc = (uint32_t)func & 0xfffffffe;
  r->stacked.lr = (uint32_t)&kernel_task_end;
  r->stacked.xpsr = 0x01000000;   // thumb mode enabled (required);

  t->stackp = r;
  return t;
}

static int32_t next_task_id = 1;
struct task * task_create(int stack_size)
{
  struct task *t = malloc(sizeof(struct task));
  memset(t, 0, sizeof(struct task));

  // We need to ensure that the stack is 8-byte aligned
  // We allocate 7 more bytes and round up the address
  //void * s = (void*)((uintptr_t)malloc(stack_size + 7) & ~(uintptr_t)0x7);
  void * s = malloc(stack_size + 7);
  t->stack_free = s;
  s = (void*)((uintptr_t)s & ~(uintptr_t)0x7);

  memset(s, 0, stack_size);
  t->stackp = s + stack_size;
  t->id = next_task_id++;

  list_init(&t->node);
  event_init(&t->join);

  return t;
}


