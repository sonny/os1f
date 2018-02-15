#include <string.h>
//#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "kernel.h"
#include "kernel_task.h"
#include "defs.h"
#include "task.h"
#include "display.h"


void printmsg(char *m);
static void task_func(void *);
static void task_once(void *);

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

extern void memory_thread_test(void);
extern uint32_t heap_size_get(void);
extern char _Heap_Begin, _Heap_Limit,_estack;

int main(void)
{
  // switch modes and make main a normal user task
  os_start();

  display_line_at(1, "Clock is %d\n", HAL_RCC_GetHCLKFreq());
  int heap_size = (int)(&_Heap_Limit - &_Heap_Begin);
  display_line_at(2, "Total Heap Space %d\n", heap_size);
    
  task_create_schedule(task_func, DEFAULT_STACK_SIZE, (void*)&fdata[0]);
  task_create_schedule(task_func, DEFAULT_STACK_SIZE, (void*)&fdata[1]);
  task_create_schedule(task_func, DEFAULT_STACK_SIZE, (void*)&fdata[2]);
  task_create_schedule(task_func, DEFAULT_STACK_SIZE, (void*)&fdata[3]);
  
  // Unocmment to test memory allocation syncronization
  // memory_thread_test();
  
  uint32_t z = 0;
  int tid = current_task_id();
  while (1) {
    ++z;
    display_line_at(3, "Current Heap Size %d\n", heap_size_get());
    display_line_at(4, "Main Task\tid : %d, counter : %d\n", tid, z);

    task_t * tonce =
      task_create_schedule(task_once, DEFAULT_STACK_SIZE, NULL);

    task_join(tonce);

    task_sleep(500);
  }
  
  return 0;
}

void task_func(void *context)
{
  int k = 0;
  int tid = current_task_id();
  struct func_data * fdata = context;
  while (1) {
    ++k;
    display_line_at(tid+4, "Simple Task\tid : %d, counter : %d\n", tid, k);
    task_sleep(fdata->sleep);

  };
}

void task_once(void *context)
{
  uint32_t tick = HAL_GetTick();
  int tid = current_task_id();
  
  display_line_at(12, "ONCE Task\tid : %d at %d ms\n", tid, tick);
}

