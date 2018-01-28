#include <string.h>
#include "memory.h"
#include "task.h"
#include "defs.h"

static struct task TCB[TASK_COUNT];
static int current_task_idx = 0;
struct task* current_task = &TCB[0];

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

// MUST be called in a critical section
void taskNext(void)
{
  // find next active task
  // NOTE: this will spin unless there is at least one active task
  // Since PendSV is the lowest priority, another irq handler
  // can update the state of the TCB to activate an inactive task
  current_task_idx = (current_task_idx + 1) % TASK_COUNT;
  while(! (TCB[current_task_idx].flags & TASK_ACTIVE) )
    current_task_idx = (current_task_idx + 1) % TASK_COUNT;

  current_task = &TCB[current_task_idx];
}

inline void * taskGet(int i)
{
  return &TCB[i];
}

static void task_end(void)
{
  while (1);
}

void taskStart(void (*func)(void*), int stack_size, struct task *args)
{
  static int task_counter = 0;
  struct task *t = &TCB[task_counter++];
  memset(t, 0, sizeof(struct task));
  void *stack = mem_alloc(stack_size);
  memset(stack, 0, stack_size);
  
  struct regs *r = stack + stack_size - sizeof(struct regs);
  r->stacked.r0 = (uint32_t)args;
  r->stacked.pc = (uint32_t)func & 0xfffffffe;
  r->stacked.lr = (uint32_t)&task_end;
  r->stacked.xpsr = 0x01000000;   // thumb mode enabled (required);
  t->psp = r;
  t->flags |= TASK_ACTIVE;

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
