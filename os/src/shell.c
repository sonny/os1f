#include "defs.h"
#include "task.h"
#include "os_printf.h"
#include "display.h"
#include <ctype.h>
#include <string.h>

typedef void (*cmd_t)(void);

typedef struct {
  char * name;
  cmd_t call;
} shell_cmd_t;


#define SHELL_IO_SIZE 2048
#define MAX_CARGS 16
static char shell_buffer[SHELL_IO_SIZE];
static char *vargs[MAX_CARGS];
static int cargs = 0;

static void shell_task(void * ctx);
static void shell_parse_cmd(int);
static void shell_process_cmd(void);

static void shell_cmd_echo(void);
static void shell_cmd_ps(void);

static shell_cmd_t commands[] = {
  {"echo", shell_cmd_echo},
  {"ps", shell_cmd_ps}
};

static int command_count = sizeof(commands)/sizeof(shell_cmd_t);

void shell_init(void)
{
  task_create_schedule(shell_task, DEFAULT_STACK_SIZE, NULL);
}

static void shell_task(void * ctx)
{
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
    if (strcmp(vargs[0], commands[i].name) == 0) {
      cmd = &commands[i];
      break;
    }
  }

  if (cmd == NULL) {
    os_iprintf("CMD: [%s : %s] is invalid.\n", vargs[0], vargs[1]);
  }
  else {
    cmd->call();
  }
}

static void shell_parse_cmd(int size)
{
  char *p = shell_buffer;
  
  bool in_word = false;
  cargs = 0;
  while (p - shell_buffer < size) {
    if (!isspace(*p)) {
      if (!in_word) { 
        in_word = true; 
        vargs[cargs++] = p;
        if (cargs == 2 ) break; 
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

static void shell_cmd_echo(void)
{
  os_iprintf("%s\n", vargs[1]);
}

static void shell_cmd_ps(void)
{
  //  kernel_task_display_task_stats(void);
  os_iprintf("PS: implement me\n");
}
