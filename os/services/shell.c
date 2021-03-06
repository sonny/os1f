#include <errno.h>
#include "os.h"

typedef void (*cmd_t)(void);

typedef struct {
	char * name;
	char * usage;
	cmd_t call;
} shell_cmd_t;

#define SHELL_STACK_SIZE 1024
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
static void shell_cmd_backtrace(void);


static shell_cmd_t commands[] = {
  { "help", "\t-- print this help", shell_cmd_help },
  { "echo", "string\t-- echo string to terminal", shell_cmd_echo },
  { "ps", "\t-- list tasks", shell_cmd_ps },
  { "start", "task_id\t-- start task", shell_cmd_start },
  { "stop", "task_id\t-- stop task", shell_cmd_stop },
  { "mem", "\t-- display memory usage", shell_cmd_mem },
  { "time", "\t-- display system timer values", shell_cmd_time },
  { "bt", "task_id\t-- display backtrace", shell_cmd_backtrace },
};

static int command_count = sizeof(commands) / sizeof(shell_cmd_t);

void shell_init(void) {
	task_t * task = task_new(shell_task, SHELL_STACK_SIZE, NULL, "Shell", TASK_PRI_HIGH);
	lcd_printf_line(task->id, "Shell - /dev/ttyACM0 %d 8 N 1", VCP_BAUD);

}

static void shell_task(void * cxt) {
	(void) cxt;
	os_iprintf("\r\n\nSimple SHELL v1.0\r\n");
	while (1) {
		os_iprintf("\r\nshell> ");
		int len = os_gets(shell_buffer, SHELL_IO_SIZE);
		if (len < 0) {
			//if (len == CONTROL_C)
				continue;
		}
		uint64_t cmd_start = usec_time();
		shell_buffer[len] = '\0';
		shell_parse_cmd(len);

		shell_process_cmd();

		uint32_t usec = usec_time() - cmd_start;
		os_iprintf("CMD processing time: %dus.\r\n", usec);
	}
}

static void shell_process_cmd(void) {
	int i;
	shell_cmd_t * cmd = NULL;
	for (i = 0; i < command_count; ++i) {
		if (strcmp(argv[0], commands[i].name) == 0) {
			cmd = &commands[i];
			break;
		}
	}

	if (cmd == NULL) {
		os_iprintf("CMD: %s is invalid.\r\n", argv[0]);
		shell_cmd_help();
	} else {
		cmd->call();
	}
}

static void shell_parse_cmd(int size) {
	char *p = shell_buffer;

	bool in_word = false;
	argc = 0;
	while (p - shell_buffer < size) {
		if (!isspace(*p)) {
			if (!in_word) {
				in_word = true;
				argv[argc++] = p;
				if (argc == 2)
					break;
			}
		} else {
			*p = '\0';
			in_word = false;
		}

		p++;
	}

	shell_buffer[size] = '\0';
}

static void shell_cmd_help(void) {
	int i;
	for (i = 0; i < command_count; ++i) {
		os_iprintf("%s\t%s\r\n", commands[i].name, commands[i].usage);
	}
}

static void shell_cmd_echo(void) {
	os_iprintf("%s\r\n", argv[1]);
}

static void shell_cmd_ps(void) {
	os_iprintf("ID\tState\tStack\tTime\tName\r\n");
	task_display(task_control_get(IDLE_TASK_ID));
	task_control_each(task_display);
}

static void shell_cmd_start(void) {
	char *end;
	int id = strtol(argv[1], &end, 10);
	task_t * task = task_control_get(id);
	if(task) {
		service_call((svcall_t) task_start, task, true);
		os_iprintf("Task id [%d] is ready\r\n", id);
	}
	else
		os_iprintf("Task id [%d] is invalid\r\n", id);
}

static void shell_cmd_stop(void) {
	char *end;
	int id = strtol(argv[1], &end, 10);
	task_t * task = task_control_get(id);
	if(task) {
		service_call((svcall_t) task_stop, task, true);
		os_iprintf("Task id [%d] is inactive\r\n", id);
	}
	else
		os_iprintf("Task id [%d] is invalid\r\n", id);
}

extern char _end; // Bottom of RAM ???
extern char _sdata, _edata; // start and end of data
extern char _sbss, _ebss; // start and end of bss
extern char _Heap_Begin;
extern char _estack; // top of RAM
extern uint32_t heap_size_get(void);

static void shell_cmd_mem(void) {
	struct mallinfo mi;
	int total_ram = &_estack - &_end;
	int bss = &_ebss - &_sbss;
	int data = &_edata - &_sdata;
	int heap_used = heap_size_get();

	os_iprintf("Total Ram                              %d\r\n", total_ram);
	os_iprintf("Static allocation                      %d\r\n", bss + data);
	os_iprintf("    Initialized   (.data)              %d\r\n", data);
	os_iprintf("    Uninitialized (.bss)               %d\r\n", bss);
	os_iprintf("Heap allocation                        %d\r\n", heap_used);

	mi = mallinfo();

	os_iprintf("Total non-mmapped bytes (arena):       %d\r\n", mi.arena);
	os_iprintf("# of free chunks (ordblks):            %d\r\n", mi.ordblks);
	os_iprintf("# of free fastbin blocks (smblks):     %d\r\n", mi.smblks);
	os_iprintf("# of mapped regions (hblks):           %d\r\n", mi.hblks);
	os_iprintf("Bytes in mapped regions (hblkhd):      %d\r\n", mi.hblkhd);
	os_iprintf("Max. total allocated space (usmblks):  %d\r\n", mi.usmblks);
	os_iprintf("Free bytes held in fastbins (fsmblks): %d\r\n", mi.fsmblks);
	os_iprintf("Total allocated space (uordblks):      %d\r\n", mi.uordblks);
	os_iprintf("Total free space (fordblks):           %d\r\n", mi.fordblks);
	os_iprintf("Topmost releasable block (keepcost):   %d\r\n", mi.keepcost);
}

static void shell_cmd_time(void) {
	uint32_t mstime = HAL_GetTick();
	uint32_t ustime = usec_time();

	os_iprintf("SysTick      time %d ms\r\n", mstime);
	os_iprintf("uSec counter time %d us\r\n", ustime);
}


#define BACKTRACE_SIZE 25

static void shell_cmd_backtrace(void)
{
  backtrace_t backtrace[BACKTRACE_SIZE];
  backtrace_frame_t bt_frame;

  errno = 0;
  char *end;
  int id = strtol(argv[1], &end, 10);
  if (errno) id = 0;

  hw_stack_frame_t hw_frame = task_get_saved_hw_frame(id);
  sw_stack_frame_t sw_frame = task_get_saved_sw_frame(id);

  bt_frame.sp = task_get_sp(id);
  bt_frame.fp = sw_frame.r7;
  bt_frame.lr = hw_frame.lr;
  bt_frame.pc = hw_frame.pc;

  int count = _backtrace_unwind(backtrace, BACKTRACE_SIZE, &bt_frame);
  int i;
  for (i = 0; i < count; ++i)
    os_iprintf("%d: 0x%x - %s@0x%x\r\n", i, backtrace[i].function, backtrace[i].name, backtrace[i].address);
}
