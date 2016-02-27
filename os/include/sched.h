#ifndef SCHED_H
#define SCHED_H

__attribute__ ((always_inline)) static inline void yield(void)
{
  __asm volatile("svc 0 \n");
}


#endif /* SCHED_H */
