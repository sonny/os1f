#ifndef __DEFS_H__
#define __DEFS_H__

#define TIME_SLICE 5 // in milliseconds

#define TASK_INACTIVE (0x0)
#define TASK_ACTIVE   (0x1)
#define TASK_SLEEP    (0x2)
#define TASK_COUNT    16
#define KERNEL_TASK   0

#define SVC_YIELD 0x00000000
#define SVC_START 0x00010000

#endif  /*__DEFS_H__ */
