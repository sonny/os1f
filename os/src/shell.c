#include "defs.h"
#include "task.h"
#include "os_printf.h"
#include "display.h"
#include "kernel_task.h"
#include "svc.h"
#include "usec_timer.h"
#include <ctype.h>
#include <string.h>

typedef void (*cmd_t)(void);

typedef struct {
  char * name;
  char * usage;
  cmd_t call;
} shell_cmd_t;


#define SHELL_IO_SIZE 2048
#define MAX_ARGC 16
static char shell_buffer[SHELL_IO_SIZE];
static char *argv[MAX_ARGC];
static int argc = 0;

static void shell_task(void * ctx);
static void shell_parse_cmd(int);
static void shell_process_cmd(void);

static void shell_cmd_help(void);
static void shell_cmd_echo(void);
static void shell_cmd_ps(void);
static void shell_cmd_start(void);
static void shell_cmd_stop(void);
static void shell_cmd_mem(void);
static void shell_cmd_time(void);

static shell_cmd_t commands[] = {
  {"help", "\t-- print this help", shell_cmd_help},
  {"echo", "string\t-- echo string to terminal", shell_cmd_echo},
  {"ps", "\t-- list tasks", shell_cmd_ps},
  {"start", "task_id\t-- start task", shell_cmd_start},
  {"stop", "task_id\t-- stop task", shell_cmd_stop},
  {"mem", "\t-- display memory usage", shell_cmd_mem},
  {"time", "\t-- display system timer values", shell_cmd_time},
};

static int command_count = sizeof(commands)/sizeof(shell_cmd_t);

void shell_init(void)
{
  task_create_schedule(shell_task, DEFAULT_STACK_SIZE, NULL, "Shell");
}

static void shell_task(void * ctx)
{
  os_iprintf("\n\nSimple SHELL v1.0\n");
  while (1) {
    os_iprintf("\nshell> ");
    int len = os_gets(shell_buffer, SHELL_IO_SIZE);
    if (len < 0) {
      if (len == CONTROL_C) continue;
    }
    shell_buffer[len] = '\0';
    shell_parse_cmd(len);

    shell_process_cmd();
  }
}

static void shell_process_cmd(void)
{
  int i;
  shell_cmd_t * cmd = NULL;
  for (i = 0; i < command_count; ++i) {
    if (strcmp(argv[0], commands[i].name) == 0) {
      cmd = &commands[i];
      break;
    }
  }

  if (cmd == NULL) {
    os_iprintf("CMD: %s is invalid.\n", argv[0]);
    shell_cmd_help();
  }
  else {
    cmd->call();
  }
}

static void shell_parse_cmd(int size)
{
  char *p = shell_buffer;
  
  bool in_word = false;
  argc = 0;
  while (p - shell_buffer < size) {
    if (!isspace(*p)) {
      if (!in_word) { 
        in_word = true; 
        argv[argc++] = p;
        if (argc == 2 ) break; 
      } 
    } 
    else {
      *p = '\0';
      in_word = false;
    } 
      
    p++;
   }

   shell_buffer[size] = '\0';
}

static void shell_cmd_help(void)
{
  int i;
  for (i = 0; i < command_count; ++i) {
    os_iprintf("%s\t%s\n", commands[i].name, commands[i].usage);
  }
}

static void shell_cmd_echo(void)
{
  os_iprintf("%s\n", argv[1]);
}

static void shell_cmd_ps(void)
{
  kernel_task_display_task_stats();
}

static void shell_cmd_start(void)
{
  char *end;
  int id = strtol(argv[1], &end, 10);
  service_call((svcall_t)kernel_task_start_id, (void*)id, true);
}

static void shell_cmd_stop(void)
{
  char *end;
  int id = strtol(argv[1], &end, 10);
  service_call((svcall_t)kernel_task_stop_id, (void*)id, true);
}

extern char __data_start__; // Bottom of RAM ???
extern char _Heap_Begin;
extern char __stack; // top of RAM
extern uint32_t heap_size_get(void);

static void shell_cmd_mem(void)
{
  struct mallinfo mi;
  int total_ram = &__stack - &__data_start__;
  int static_used = &_Heap_Begin - &__data_start__;
  int heap_used = heap_size_get();
  
  os_iprintf("Total Ram                              %d\n", total_ram);
  os_iprintf("Static allocation                      %d\n", static_used);
  os_iprintf("Heap allocation                        %d\n", heap_used);
  
  mi = mallinfo();

  os_iprintf("Total non-mmapped bytes (arena):       %d\n", mi.arena);
  os_iprintf("# of free chunks (ordblks):            %d\n", mi.ordblks);
  os_iprintf("# of free fastbin blocks (smblks):     %d\n", mi.smblks);
  os_iprintf("# of mapped regions (hblks):           %d\n", mi.hblks);
  os_iprintf("Bytes in mapped regions (hblkhd):      %d\n", mi.hblkhd);
  os_iprintf("Max. total allocated space (usmblks):  %d\n", mi.usmblks);
  os_iprintf("Free bytes held in fastbins (fsmblks): %d\n", mi.fsmblks);
  os_iprintf("Total allocated space (uordblks):      %d\n", mi.uordblks);
  os_iprintf("Total free space (fordblks):           %d\n", mi.fordblks);
  os_iprintf("Topmost releasable block (keepcost):   %d\n", mi.keepcost);
}

static void shell_cmd_time(void)
{
  uint32_t mstime = HAL_GetTick();
  uint32_t ustime = usec_time();

  os_iprintf("SysTick      time %d ms\n", mstime);
  os_iprintf("uSec counter time %d us\n", ustime);
}

