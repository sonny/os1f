#include <string.h>
#include <stdbool.h>
#include "stm32f746xx.h"
#include "stm32f7xx_hal.h"
#include "task.h"
#include "sched.h"
#include "memory.h"
#include "registers.h"
#include "core.h"
#include "spinlock.h"
#include "os.h"
#include <stdatomic.h>

/*
 * How tasks work:
 * Tasks are listed in an array.
 * The current task is accessed one of two ways:
 *    current_task_ptr points directly to the task struct
 *    current_task is an index into the TCB array
 * BOTH are ONLY modified in task_next, which is ONLY
 * called from SVC_Handler, which disables IRQs to ensure
 * no race conditions occur.
 * Both are static, and the only way to access the current task
 * is to call task_current().
 *
 * The IDLE task has an index of -1 and is not listed in the
 * TCB. It is run when NONE of the other tasks in the
 * READY state.
 *
 * The MAIN task has an index of 0 and is run at the same
 * level as all of the others after task_bootstrap_main is invoked.
 *
 * Both the MAIN and IDLE tasks have special handles that
 * exist outside of the TCB, but are only accessible to task library
 * functions.
 */

#define IDLE_STACK_SIZE 256
#define MIN_STACK_SIZE 512
#define MAIN_STACK_SIZE 1024

struct task *TCB[TASK_COUNT];        // not protected
static struct task *main_task;       // not protected 
static struct task *idle_task;       // not protected 

static int task_insert_index = 0;    // protected by lock
static int current_task = 0;         // protected by disable irq
struct task *current_task_ptr = NULL;    // protected by disable irq -- global, but not explicitly extern

static volatile uint32_t lock;

static int get_next_task_insert(void) {
  return atomic_fetch_add(&task_insert_index, 1);
}

static void * task_stack_init(struct regs* task_sp,  uint32_t ret, uint32_t param);
static int task_start_idle(void);
static int task_start_main(void);

void task_init(void)
{
  uint32_t rp = -1;
  task_start_idle();
  current_task = -1;
  current_task_ptr = idle_task;
  void *idle_stack = idle_task->stack;

  task_start_main();

  // reset stack pointer for idle task
  idle_task->stack = idle_stack;
}

static void * task_stack_alloc_init(void* stack_start, uint32_t ret, uint32_t param)
{
  void* task_sp = (void*)stack_start - sizeof(struct regs);
  // ensure that address where (struct stacked_regs) is allocated is 8 byte aligned
  // TODO: check CCR first - CCR has a bit enforcing stack alignment to 8 or 4 bytes -
  // it defaults to 8 bytes
  int rem = (uint32_t) ((void*)task_sp + sizeof(struct aux_regs)) % 8;
  if (rem) {        // if there is a remainder, then we are not aligned
    task_sp -= rem; // rem should really only be 4 or 0
  }
  void * result = task_stack_init(task_sp, ret, param);
  
  return task_sp;
}

static void * task_stack_init(struct regs* task_sp, uint32_t ret, uint32_t param)
{
  memset(task_sp, 0, sizeof(struct regs));   // initialiaze everything to 0
  task_sp->stacked.r0 = param;          // first param to function call
  task_sp->stacked.pc = ret;            // return execution location
  task_sp->stacked.xspr = 0x01000000;   // thumb mode enabled (required);
  task_sp->aux.lr = 0xfffffffd;         // EXEC_RETURN, USE PSP
  return task_sp;
}

static volatile int q = 0;
static void task_idle(void *p)
{
  (void)p;
  while(1) { q++; }
}

static int task_start_idle(void)
{
  int alloc_size = IDLE_STACK_SIZE + sizeof(struct task);
  idle_task = mem_alloc(alloc_size);

  // sp points to the END (last address) of the allocated region
  void *stack_start = (void*)idle_task + alloc_size;
  void *task_sp = task_stack_alloc_init(stack_start, (uint32_t)task_idle, 0);

  idle_task->stack_start = stack_start;
  idle_task->stack_end = (void*)idle_task + sizeof(struct task);

  idle_task->stack = task_sp;
  idle_task->stack_size = IDLE_STACK_SIZE;
  idle_task->state = TASK_STATE_READY;
  idle_task->id = -1;
  
  return -1;
}

static int task_start_main(void)
{
  uint32_t stack_base = *((uint32_t*)SCB->VTOR);
  
  uint32_t stack_ptr  = get_SP();
  uint32_t stack_size = stack_base - stack_ptr;
  uint32_t return_ptr = -1;

  int alloc_size = MAIN_STACK_SIZE + sizeof(struct task);
  main_task = mem_alloc(alloc_size);

  // sp points to the END (last address) of the allocated region
  // offset by stack size calculated at the top
  void *task_sp = (void*)main_task + alloc_size - stack_size;
  // copy the (current) contents of the stack to the new region
  memcpy(task_sp, (void*)stack_ptr, stack_size);
  
  __asm volatile ("ldr %0, =task_init_done\n" : "=r"(return_ptr) : : "lr"); 
  task_sp = task_stack_alloc_init(task_sp, return_ptr, 0);

  main_task->stack_start = (void*)main_task + alloc_size;
  main_task->stack_end = (void*)main_task + sizeof(struct task);

  main_task->stack = task_sp;
  main_task->stack_size = MAIN_STACK_SIZE;
  main_task->state = TASK_STATE_READY;
  main_task->id = 0;
  TCB[0] = main_task;
  atomic_store(&task_insert_index, 1);
  
  // need to initialize the PSP for the first SVC call
  __set_PSP(__get_MSP());
  __asm volatile("svc 0 \n"
                 "task_init_done: \n"  // return point from SVC_Handler
                 );

  return 0;
}

int task_start(void (*f)(void*), int stack_size, void *param)
{
  int task_index = get_next_task_insert();
  if (task_index >= TASK_COUNT) return -1;

  /* min stack size */
  if (stack_size < MIN_STACK_SIZE) stack_size = MIN_STACK_SIZE;
  int alloc_size = stack_size + sizeof(struct task);
  struct task *task = mem_alloc(alloc_size);

  // sp points to the END (last address) of the allocated region
  void *stack_start = (void*)task + alloc_size;
  /* TODO: check the return value? */

  void *task_sp = task_stack_alloc_init(stack_start, (uint32_t)f, (uint32_t)param);

  task->stack_start = stack_start;
  task->stack_end = (void*)task + sizeof(struct task);
  task->stack = task_sp;
  task->stack_size = stack_size;
  task->state = TASK_STATE_READY;
  task->id = task_index;
  TCB[task_index] = task;
  
  return task_index;
}

static inline bool task_change_from_state(struct task *t, uint32_t from, uint32_t to) {
  return atomic_compare_exchange_strong(&(t->state), &from, to);
}

static inline bool task_change_from_state_current(uint32_t from, uint32_t to) {
  return task_change_from_state(current_task_ptr, from, to);
}

// NOTE: should only be called by task_next
static bool _task_wake_all(void)
{
  int i;
  bool wake = false;
  uint32_t tick = osGetTick();
  for (i = 0; i < task_insert_index; ++i) {
    if (TCB[i]->state & TASK_STATE_SLEEP &&
        TCB[i]->sleep_until < tick) {
      TCB[i]->state = TASK_STATE_READY; // no need for stdatomic if called as specified
      wake = true;
    }
  }

  return wake;
}

// NOTE: should only be called by SVC_Handler
struct task * task_next(void)
{
  _task_wake_all();

  int nt = current_task;
  int last = current_task == -1 ? task_insert_index -1 : current_task;

  /* This loop should have 2 SUBTLE features
   * 1. if the current task is -1 (IDLE), then it starts at 0 index
   *    and examines all tasks
   * 2. otherwise, it looks at all tasks starting at current+1 mod
   *    the number of tasks, and should look at current LAST before
   *    giving up.
   */
  current_task_ptr = idle_task; // the default
  do {
    nt = (nt + 1) % task_insert_index;
    if (TCB[nt]->state == TASK_STATE_READY)
      {
        current_task = nt;
        current_task_ptr = TCB[nt];
        break;
      }
  } while (nt != last);
  current_task_ptr->timeout_at = osGetTick() + TIMESLICE;
  return current_task_ptr;
}


__attribute__((always_inline)) void task_sleep(uint32_t ms) {
  task_sleep_until(osGetTick() + ms);
}

__attribute__((always_inline)) void task_sleep_until(uint32_t ms) {
  if (task_change_from_state_current(TASK_STATE_READY, TASK_STATE_SLEEP)) {
    current_task_ptr->sleep_until = ms;
    yield();
  }
}

__attribute__((always_inline)) void task_wait(uint32_t state)
{
  if (task_change_from_state_current(TASK_STATE_READY, state))
      yield();
}

__attribute__((always_inline)) void task_change_state(uint32_t new_state)
{
  atomic_store(&current_task_ptr->state, new_state);
}

// NOTE: cannot nest mutexes with this implementation
__attribute__((always_inline)) bool task_notify(uint32_t id, int state)
{
  return task_change_from_state(TCB[id], state, TASK_STATE_READY);
}

const struct task *task_get(uint32_t id)
{
  return TCB[id];
}
