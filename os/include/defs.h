#ifndef __DEFS_H__
#define __DEFS_H__

#define TIME_SLICE 5 // in milliseconds

#define TASK_INACTIVE (0)
#define TASK_ACTIVE   (1)
#define TASK_SLEEP    (2)
#define TASK_WAIT     (3)
#define TASK_END      (4)

#define TASK_COUNT    16

#define KERNEL_TASK   0

#define SVC_YIELD         0x00000000
#define SVC_START         0x00010000
#define SVC_TASK_START    0x00010001
#define SVC_TASK_SLEEP    0x00010002
#define SVC_EVENT_WAIT    0x00010004
#define SVC_EVENT_NOTIFY  0x00010005
#define SVC_TASK_REMOVE   0x00010006

#define DEFAULT_STACK_SIZE 256
#define STDIO_BUFFER_SIZE 256

#ifndef VCP_BAUD
#define VCP_BAUD 115200
#endif

#define CONTROL_C -1

/* Forward struct declarations */
typedef struct task task_t;
typedef struct event event_t;
typedef struct list list_t;
typedef struct mutex mutex_t;

#endif  /*__DEFS_H__ */
