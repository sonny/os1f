#include "stm32f7xx_hal.h"
#include <string.h>
//#include <stdio.h>
#include "memory.h"
#include "semihosting.h"

#define TASK_INACTIVE (0x0)
#define TASK_ACTIVE   (0x1)
#define TASK_COUNT  4

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
  //  uint32_t lr;
};

struct regs {
  struct manual_regs manual;
  struct stacked_regs stacked;
};


struct task {
  uint32_t flags;
  void * psp;
};

static struct task TCB[TASK_COUNT];
static int current_task_idx = 0;

static void printmsg(char *m);
static void local_osInit(void);
static void local_taskInit(void);
static void local_taskStart(void (*)(void*), int, struct task *); 
static void task_end(void);
static void task_func(void *);

static void syscall_start(void);
static void syscall_yield(void);
static void kernel_critical_begin(void);
static void kernel_critical_end(void);
static uint32_t kernel_PSP_get(void);
static void kernel_PSP_set(uint32_t);
static void kernel_CONTROL_set(uint32_t);
static void kernel_sync_barrier(void);

extern void initialise_monitor_handles(void);

int main(void)
{
  //initialise_monitor_handles();
  
  // start in handler mode
  // using MSP in privileged mode
  local_osInit();
  local_taskStart(task_func, 512, (void*)"Task 0");
  local_taskStart(task_func, 512, (void*)"Task 1");

  printmsg("HELLLLO\n");

  // temporary stack space
  void * pspStart = mem_alloc(256) + 256;
  //kernel_CONTROL_set(0X1 << 1); // USE PSP in thread mode (default is MSP)

  kernel_sync_barrier();
  kernel_PSP_set((uint32_t)pspStart);
  kernel_sync_barrier();
  kernel_CONTROL_set(0x01 << 1 | 0x01 << 0); // use PSP with unprivileged thread mode
  kernel_sync_barrier();

  syscall_start(); // does not return
  
  return 0;
}

volatile int global_bob = 0;
volatile char * taskname = "";
void task_func(void *context)
{
  static char buffer[64];
  taskname = context;
  int k = 0;
  while (1) {
    global_bob = ++k;
    snprintf(buffer, 64, "%s [%d]\n", (char*)context, global_bob);
    printmsg(buffer);
    syscall_yield();
  };
}

void local_osInit(void)
{
  // lowest priority 
  NVIC_SetPriority(PendSV_IRQn, 255);
  
  //  SysTick_Config(HAL_RCC_GetHCLKFreq()/1000);

  mem_init();
}

void local_taskStart(void (*func)(void*), int stack_size, struct task *args)
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

  // for debugging
  r->stacked.r1  = 0xdeaf0001;
  r->stacked.r2  = 0xdeaf0002;
  r->stacked.r3  = 0xdeaf0003;
  r->manual.r4   = 0xdeaf0004;
  r->manual.r5   = 0xdeaf0005;
  r->manual.r6   = 0xdeaf0006;
  r->manual.r7   = 0xdeaf0007;
  r->manual.r8   = 0xdeaf0008;
  r->manual.r9   = 0xdeaf0009;
  r->manual.r10  = 0xdeaf000A;
  r->manual.r11  = 0xdeaf000B;
  r->stacked.r12 = 0xdeaf000C;
}

void task_end(void)
{
  while (1);
}

static void set_PendSV(void)
{
  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

#define SVC_YIELD 0x00000000
#define SVC_START 0x00010000

void SVC_Handler(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3)
{
  switch (r0) {
  case SVC_YIELD:
    set_PendSV();
    break;
  case SVC_START:
    set_PendSV();
    break;
  }
}

// called by ASM PendSV_Handler
void pendsv_handler(uint32_t r0)
{
  struct task * new_task;
  
  switch (r0) {
  case SVC_YIELD:
    kernel_critical_begin();
    TCB[current_task_idx].psp = (void*)kernel_PSP_get();

    // find next active task
    // NOTE: this will spin unless there is at least one active task
    // Since PendSV is the lowest priority, another irq handler
    // can update the state of the TCB to activate an inactive task
    current_task_idx = (current_task_idx + 1) % TASK_COUNT;
    while(!TCB[current_task_idx].flags & TASK_ACTIVE)
      current_task_idx = (current_task_idx + 1) % TASK_COUNT;

    kernel_PSP_set((uint32_t)TCB[current_task_idx].psp);
    kernel_critical_end();
    break;

  case SVC_START:
    kernel_critical_begin();

    // find next active task
    current_task_idx = 0;
    while(!TCB[current_task_idx].flags & TASK_ACTIVE)
      current_task_idx = (current_task_idx + 1) % TASK_COUNT;

    kernel_PSP_set((uint32_t)TCB[current_task_idx].psp);
    kernel_critical_end();
    break;
  }
}

static inline void kernel_critical_begin(void)
{
  __asm volatile ("cpsid i" ::: "memory");
}

static inline void kernel_critical_end(void)
{
  __asm volatile ("cpsie i\n"
                  "isb    \n"
                  ::: "memory");
}

static inline uint32_t kernel_PSP_get(void)
{
  register uint32_t result;
  __asm volatile ("MRS %0, psp\n"  : "=r" (result) );
  return(result);
}

static inline void kernel_PSP_set(uint32_t sp)
{
  __asm volatile ("MSR psp, %0\n" : : "r" (sp) : "sp");
}

static inline void kernel_CONTROL_set(uint32_t ctl)
{
  __asm volatile ("MSR control, %0\n" : : "r" (ctl) );
}

static inline void kernel_sync_barrier(void)
{
  __asm volatile ("isb \n");
}

static inline void syscall_start(void)
{
  __asm volatile ("mov r0, 0x00010000\n"
                  "svc 0             \n" ::: "r0", "lr" );
}

static inline void syscall_yield(void)
{
  __asm volatile ("mov r0, 0x00000000\n"
                  "svc 0             \n" ::: "r0", "lr" );
}

static volatile uint32_t SCB_HFSR;
static volatile uint32_t SCB_CFSR;
void HardFault_Handler(void)
{
  static volatile char * name = "HARD FAULT HANDLER";
 SCB_HFSR = SCB->HFSR;
 if ((SCB_HFSR & (1<<30)) != 0) {
   // Hard Fault is FORCED
   SCB_CFSR = SCB->CFSR;
   
 }
   
  __asm volatile("BKPT #01");
  while(1);
}

void UsageFault_Handler(void)
{
  static volatile char * name = "USAGE FAULT HANDLER";
  while(1);
}

void BusFault_Handler(void)
{
  static volatile char * name = "Bus FAULT HANDLER";
  while(1);
}

void MemFault_Handler(void)
{
  static volatile char * name = "MEM FAULT HANDLER";
  while(1);
}

void NMI_Handler(void)
{
  //printmsg("NMI Handler\n");
  while(1);
}

static void printmsg(char *m)
{
  int data[3] = {
    1,        // stdout
    (int)m,   // pointer to data
    strlen(m) // size of data
  };
  call_host(SEMIHOSTING_SYS_WRITE, data);
}
