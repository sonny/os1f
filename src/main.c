/*
  main.c
 */

#include "task.h"
#include "os.h"
#include "display.h"

struct task_data {
  uint32_t interval;
};

void task(void *p);
void task_rude(void *p);
void adc_task(void *p);

static struct task_data td[4] = {
  { interval:500},
  { interval:250},
  { interval:125},
  { interval:800}
};

extern struct task *TCB[];

int main()
{
  osInit();

  task_start(task, 128, &td[0]);
  task_start(task, 128, &td[1]);
  task_start(task, 128, &td[2]);
  task_start(task_rude, 128, NULL);
  task_start(adc_task, 0, NULL);
  

  /* Our main starts here */
  int count = 0;
  while (1) {
    uint32_t now = osGetTick();
    task_display_line("Task Main: %d", count++);
    task_sleep_until(now + 1000);
  }
  return 0;

}

void task(void *p)
{
  struct task_data *q = p;
  uint32_t counter = 0;
  
  int id = task_current()->id;
  
  while(1)
    {
      uint32_t now = osGetTick();

      task_display_line("Task   %d: %d", id, counter++);
      task_sleep_until(now + q->interval);
    } 
}

void task_rude(void *p)
{
  struct task_data *q = p;
  uint32_t counter = 0;
  const int divisor = 2000000; // 2 Million!!!

  int id = task_current()->id;

  while(1)
    {
      counter++;
      if ((counter % divisor) == 0) {
        task_display_line("Task   %d: %d", id, counter/divisor);
      }
    } 
}

