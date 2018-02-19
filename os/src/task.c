#include <string.h>
#include <stdlib.h>
#include "task.h"
#include "defs.h"
#include "kernel_task.h"

// implemented in kernel_task
extern void kernel_task_end(void);

task_t * task_stack_init(task_t *t, void (*func)(void*), void *context)
{
  hw_stack_frame_t *frame = t->sp - sizeof(hw_stack_frame_t);
  frame->r0 = (uint32_t)context;
  frame->pc = (uint32_t)func & 0xfffffffe;
  frame->lr = (uint32_t)&kernel_task_end;
  frame->xpsr = 0x01000000;   // thumb mode enabled (required);

  t->sp = frame;
  return t;
}

//static int32_t next_task_id = 1;
task_t * task_create(int stack_size, const char * name)
{
  task_t *t = malloc(sizeof(task_t));
  memset(t, 0, sizeof(task_t));

  // We need to ensure that the stack is 8-byte aligned
  // We allocate 7 more bytes and round up the address
  void * s = malloc(stack_size + 7);
  t->stack_free = s;
  s = (void*)((uintptr_t)s & ~(uintptr_t)0x7);

  memset(s, 0, stack_size);
  t->sp = s + stack_size;
  t->id = kernel_task_next_id();
  t->exc_return = 0xfffffffd;
  t->name = name;
  
  list_init(&t->node);
  event_init(&t->join);

  return t;
}

static void protected_task_start(void * cxt)
{
  task_t * new = cxt;
  kernel_critical_begin();
  kernel_task_start_task(new);
  kernel_critical_end();
}

static void protected_task_sleep(void *cxt)
{
  uint32_t ms = (uint32_t)cxt;
  kernel_critical_begin();
  kernel_task_sleep_current(ms);
  kernel_critical_end();
  protected_kernel_context_switch(NULL);
}

inline
void task_schedule(task_t *task)
{
  service_call(protected_task_start, task, false);
}

inline
void task_sleep(uint32_t ms) 
{
  service_call(protected_task_sleep, (void*)ms, false);
}

inline
void task_yield(void) 
{
  kernel_context_switch();
}

