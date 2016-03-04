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

#define MIN_STACK_SIZE 1024
#define MAIN_STACK_SIZE 1024

static struct task *TCB[TASK_COUNT]; // not protected
static struct task *main_task;       // not protected 
static struct task *idle_task;       // not protected 

static int task_insert_index = 0; // protected by lock
static int current_task = 0;      // protected by disable irq
struct task *current_task_ptr;    // protected by disable irq -- global, but not explicitly extern

static volatile uint32_t lock;

static int get_next_task_insert(void) {
  //  spinlock_lock(&lock);
  //  int result = task_insert_index++;
  //  spinlock_unlock(&lock);
  //  return result;
  return atomic_fetch_add(&task_insert_index, 1);
}

static void * task_stack_init(uint32_t stack_end,  uint32_t ret, uint32_t param);
static int task_start_idle(void);
static int task_start_main(void);

void task_init(void)
{
  uint32_t rp = -1;
  // start task idle
  task_start_idle();
  current_task = -1;
  current_task_ptr = idle_task;
  void *idle_stack = idle_task->stack;

  task_start_main();

  // reset stack pointer for idle task
  idle_task->stack = idle_stack;
}

static void * task_stack_alloc_init(uint32_t stack_end, uint32_t ret, uint32_t param)
{
  uint32_t stack_start = (uint32_t)((void*)stack_end - sizeof(struct regs));
  void * result = task_stack_init(stack_start, ret, param);
  return result;
}

static void * task_stack_init(uint32_t stack_start, uint32_t ret, uint32_t param)
{
  struct regs *r = (struct regs*)stack_start;
  memset(r, 0, sizeof(struct regs));   // initialiaze everything to 0
  r->stacked.r0 = param;          // first param to function call
  r->stacked.pc = ret;            // return execution location
  r->stacked.xspr = 0x01000000;   // thumb mode enabled (required);
  r->aux.lr = 0xfffffffd;         // EXEC_RETURN, USE PSP
  return r;
}

static volatile int q = 0;
static void task_idle(void *p)
{
  (void)p;
  while(1) { q++; }
}

static int task_start_idle(void)
{
  int alloc_size = MIN_STACK_SIZE + sizeof(struct task);
  idle_task = mem_alloc(alloc_size);

  // sp points to the END (last address) of the allocated region
  uint32_t sp = (uint32_t)((void*)idle_task + alloc_size);
  void *r = task_stack_alloc_init(sp, (uint32_t)task_idle, 0);

  idle_task->stack = r;
  idle_task->stack_size = MIN_STACK_SIZE;
  idle_task->state = TASK_STATE_READY;
  idle_task->id = -1;
  
  return -1;
}

static int task_start_main(void)
{
  uint32_t stack_base = *((uint32_t*)SCB->VTOR);
  
  uint32_t stack_ptr  = get_SP();
  uint32_t stack_size = stack_base - stack_ptr;
  uint32_t rp = -1;

  int alloc_size = MAIN_STACK_SIZE + sizeof(struct task);
  main_task = mem_alloc(alloc_size);

  // sp points to the END (last address) of the allocated region
  // offset by stack size calculated at the top
  uint32_t sp = (uint32_t)((void*)main_task + alloc_size - stack_size);
  // copy the (current) contents of the stack to the new region
  memcpy((void*)sp, (void*)stack_ptr, stack_size);
  
  __asm volatile ("ldr %0, =task_init_done\n" : "=r"(rp) : : "lr"); 
  void *r = task_stack_alloc_init(sp, rp, 0);
  main_task->stack = r;
  main_task->stack_size = MAIN_STACK_SIZE;
  main_task->state = TASK_STATE_READY;
  main_task->id = 0;
  TCB[0] = main_task;
  task_insert_index = 1;
  
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
  uint32_t sp = (uint32_t)((void*)task + alloc_size);
  /* check the return value? */

  void *r = task_stack_alloc_init(sp, (uint32_t)f, (uint32_t)param);

  task->stack = r;
  task->stack_size = stack_size;
  task->state = TASK_STATE_READY;
  task->id = task_index;
  TCB[task_index] = task;
  
  return task_index;
}

// NOTE: should only be called by task_next
static bool _task_wake_all(void)
{
  int i;
  bool wake = false;
  for (i = 0; i < task_insert_index; ++i) {
    if (TCB[i]->state & TASK_STATE_SLEEP &&
        TCB[i]->sleep_until < osGetTick() ) {
      TCB[i]->state = TASK_STATE_READY;
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

__attribute__ ((always_inline)) inline const struct task * task_current(void)
{
  return current_task_ptr;
}

// this is always executed in the context of the CURRENT_TASK
// as long as constness of t is preserved, its fine
void task_sleep(uint32_t ms) {
  task_sleep_until(osGetTick() + ms);
}

void task_sleep_until(uint32_t ms) {
  struct task * const t = current_task_ptr;
  t->sleep_until = ms;
  t->state = TASK_STATE_SLEEP;
  yield();
}


void task_wait(uint32_t on)
{
  struct task * const t = current_task_ptr;
  switch (on) {
  case WAIT_MUTEX:
    t->state = TASK_STATE_WAIT_MUTEX;
    yield();
    break;
  case WAIT_EVENT:
    t->state = TASK_STATE_WAIT_EVENT;
    yield();
    break;
  default:
    break;
    /* do nothing */
  }
}

inline void task_change_state(uint32_t new)
{
  //struct task * t = current_task_ptr;
  //t->state = new;
  atomic_store(&current_task_ptr->state, new);
}

// NOTE: cannot nest mutexes with this implementation
void task_notify(uint32_t id)
{
  //  struct task * const t = TCB[id];
  //t->state = TASK_STATE_READY;
  atomic_store(&(TCB[id]->state), TASK_STATE_READY);
}

const struct task *task_get(uint32_t id)
{
  return TCB[id];
}
