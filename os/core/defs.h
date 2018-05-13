#ifndef __DEFS_H__
#define __DEFS_H__

#define SYSTICK_RESOLUTION 1 // in milliseconds

#define TASK_FLAG_FPU    (1<<31)
#define TASK_FLAG_STATIC (1<<30)

#define DEFAULT_STACK_SIZE 512
#define STDIO_BUFFER_SIZE 256

#ifndef VCP_BAUD
#define VCP_BAUD 115200
#endif

#define LCD_TEXT_COLOR LCD_COLOR_WHITE

#define CONTROL_C -1

#define TASK_SIGNATURE  0xdeadbeef
#define MUTEX_SIGNATURE 0xbeaddaeb
#define EVENT_SIGNATURE 0xfeebfaab

#ifndef MAX_TASK_COUNT
#define MAX_TASK_COUNT  16
#endif

#ifndef MAX_EVENT_COUNT
#define MAX_EVENT_COUNT 32
#endif

#ifndef IDLE_STACK_SIZE
#define IDLE_STACK_SIZE 128
#endif

#ifndef MAIN_STACK_SIZE
#define MAIN_STACK_SIZE 1024  // default size of main stack
#endif

#endif  /*__DEFS_H__ */
