#include <string.h>
#include "stm32f7xx_hal.h"
#include "kernel.h"
#include "memory.h"
#include "defs.h"
#include "task.h"
#include "semihosting.h"
#include "vsnprintf.h"

void printmsg(char *m);
static void task_func(void *);

int main(void)
{
  // start in handler mode - using MSP in privileged mode
  osInit();

  taskStart(task_func, 128, (void*)"Task 0");
  taskStart(task_func, 128, (void*)"Task 1");
  taskStart(task_func, 128, (void*)"Task 2");
  taskStart(task_func, 128, (void*)"Task 3");

  static char buffer[64];
  
  os_snprintf(buffer, 64, "Ckock is %d\n", HAL_RCC_GetHCLKFreq());
  printmsg(buffer);

  osStart(); // does not return
  
  return 0;
}

void task_func(void *context)
{
  static char buffer[64];
  int divisor = 1000000; // 10 million
  int k = 0;
  while (1) {
    ++k;
    if ((k % divisor) == 0) { // 10 million
      os_snprintf(buffer, 64, "%s [%d]\n", (char*)context, k/divisor);
      printmsg(buffer);
      //syscall_yield();
    }
  };
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
