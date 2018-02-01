#include <string.h>
#include "memory.h"
#include "task.h"
#include "defs.h"

struct stacked_regs {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr; // r14
  uint32_t pc; // r15
  uint32_t xpsr;
};

struct manual_regs {
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
  uint32_t r8;
  uint32_t r9;
  uint32_t r10;
  uint32_t r11;
};

struct regs {
  struct manual_regs manual;
  struct stacked_regs stacked;
};


static void task_end(void)
{
  while (1);
}

struct task * task_create(void (*func)(void*), int stack_size, void *context)
{
  //static int task_counter = 0;
  struct task *t = mem_alloc(sizeof(struct task));
  memset(t, 0, sizeof(struct task));
  void *stack = mem_alloc(stack_size);
  memset(stack, 0, stack_size);
  
  struct regs *r = stack + stack_size - sizeof(struct regs);
  r->stacked.r0 = (uint32_t)context;
  r->stacked.pc = (uint32_t)func & 0xfffffffe;
  r->stacked.lr = (uint32_t)&task_end;
  r->stacked.xpsr = 0x01000000;   // thumb mode enabled (required);

  t->psp = r;
  return t;
  //t->id = task_counter++;
  //t->flags |= TASK_ACTIVE;

  /* // for debugging */
  /* r->stacked.r1  = 0xdeaf0001; */
  /* r->stacked.r2  = 0xdeaf0002; */
  /* r->stacked.r3  = 0xdeaf0003; */
  /* r->manual.r4   = 0xdeaf0004; */
  /* r->manual.r5   = 0xdeaf0005; */
  /* r->manual.r6   = 0xdeaf0006; */
  /* r->manual.r7   = 0xdeaf0007; */
  /* r->manual.r8   = 0xdeaf0008; */
  /* r->manual.r9   = 0xdeaf0009; */
  /* r->manual.r10  = 0xdeaf000A; */
  /* r->manual.r11  = 0xdeaf000B; */
  /* r->stacked.r12 = 0xdeaf000C; */
}


