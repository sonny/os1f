#include <string.h>
#include "stm32f7xx_hal.h"
#include "kernel.h"
#include "memory.h"
#include "defs.h"
#include "task.h"
#include "semihosting.h"

void printmsg(char *m);
static void local_osInit(void);


static void task_func(void *);

extern void initialise_monitor_handles(void);

int main(void)
{
  //initialise_monitor_handles();
  
  // start in handler mode
  // using MSP in privileged mode
  local_osInit();
  taskStart(task_func, 512, (void*)"Task 0");
  taskStart(task_func, 512, (void*)"Task 1");
  taskStart(task_func, 512, (void*)"Task 2");
  taskStart(task_func, 512, (void*)"Task 3");

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

void task_func(void *context)
{
  static char buffer[64];

  int k = 0;
  while (1) {
    snprintf(buffer, 64, "%s [%d]\n", (char*)context, ++k);
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

void printmsg(char *m)
{
  int data[3] = {
    1,        // stdout
    (int)m,   // pointer to data
    strlen(m) // size of data
  };
  call_host(SEMIHOSTING_SYS_WRITE, data);
}
