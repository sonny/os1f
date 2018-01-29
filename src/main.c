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

int main(void)
{
  // start in handler mode - using MSP in privileged mode
  osInit();

  taskStart(task_func, 256, (void*)&fdata[0]);
  taskStart(task_func, 256, (void*)&fdata[1]);
  taskStart(task_func, 256, (void*)&fdata[2]);
  taskStart(task_func, 256, (void*)&fdata[3]);

  static char buffer[32];
  
  os_snprintf(buffer, 32, "Ckock is %d\n", HAL_RCC_GetHCLKFreq());
  printmsg(buffer);

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
    os_snprintf(buffer, 32, "%s [%d]\n", fdata->name, k);
    printmsg(buffer);
    taskSleep(fdata->sleep);
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
