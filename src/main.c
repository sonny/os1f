#include <string.h>
#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "kernel.h"
#include "memory.h"
#include "defs.h"
#include "task.h"
#include "semihosting.h"
#include "vsnprintf.h"

void printmsg(char *m);
static void task_func(void *);

struct func_data {
  char * name;
  uint32_t sleep;
};


static struct func_data fdata[4] = {
  {.name = "Task1", .sleep = 4000},
  {.name = "Task2", .sleep = 2000},
  {.name = "Task3", .sleep = 1000},
  {.name = "Task4", .sleep = 500},
};

#define DEFAULT_STACK_SIZE 512

int main(void)
{
  // start in handler mode - using MSP in privileged mode
  osInit();

  // temporary stack space
  void * pspStart = mem_alloc(DEFAULT_STACK_SIZE) + DEFAULT_STACK_SIZE;
 
  kernel_sync_barrier();
  kernel_PSP_set((uint32_t)pspStart);
  kernel_sync_barrier();
  kernel_CONTROL_set(0x01 << 1 | 0x01 << 0); // use PSP with unprivileged thread mode
  kernel_sync_barrier();
  
  struct task * t0 = task_create(task_func, DEFAULT_STACK_SIZE, (void*)&fdata[0]);
  struct task * t1 = task_create(task_func, DEFAULT_STACK_SIZE, (void*)&fdata[1]);
  struct task * t2 = task_create(task_func, DEFAULT_STACK_SIZE, (void*)&fdata[2]);
  struct task * t3 = task_create(task_func, DEFAULT_STACK_SIZE, (void*)&fdata[3]);

  printf("Clock is %d\n", HAL_RCC_GetHCLKFreq());
  
  task_start(t0);
  task_start(t1);
  task_start(t2);
  task_start(t3);
  

  osStart(); // does not return
  
  return 0;
}

void task_func(void *context)
{
  char buffer[32];
  int divisor = 1000000; // 10 million
  unsigned int k = 0;
  struct func_data * fdata = context;
  while (1) {
    ++k;
    printf("%s [%d]\n", fdata->name, k);
    task_sleep(fdata->sleep);

  };
}

/* void printmsg(char *m) */
/* { */
/*   int data[3] = { */
/*     1,        // stdout */
/*     (int)m,   // pointer to data */
/*     strlen(m) // size of data */
/*   }; */
/*   call_host(SEMIHOSTING_SYS_WRITE, data); */
/* } */
