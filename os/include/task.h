#ifndef TASK_H
#define TASK_H

#include <stdint.h>


#define TASK_COUNT 8
#define KB         1024

struct task {
  void *stack;
  uint32_t id;
  uint32_t stack_size;
  uint32_t timeout_at;
  uint32_t sleep_until;
  uint32_t state
  __attribute__ ((aligned (8)));
};

void task_init(void);
void task_bootstrap_main(uint32_t lr);
int task_start(void (*)(void*), int, void*);
const struct task * task_current(void);
void task_sleep(uint32_t ms);
void task_sleep_until(uint32_t ms);
void task_wait(uint32_t on);
void task_notify(uint32_t id);
const struct task *task_get(uint32_t id);
void task_change_state(uint32_t new);

struct stacked_regs {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr; // r14
  uint32_t pc; // r15
  uint32_t xspr;
#ifdef ENABLE_FP
  uint32_t s0;
  uint32_t s1;
  uint32_t s2;
  uint32_t s3;
  uint32_t s4;
  uint32_t s5;
  uint32_t s6;
  uint32_t s7;
  uint32_t s8;
  uint32_t s9;
  uint32_t s10;
  uint32_t s11;
  uint32_t s12;
  uint32_t s13;
  uint32_t s14;
  uint32_t s15;
  uint32_t fpscr;
  uint32_t reserved;
#endif /* ENABLE_FP */
};

struct aux_regs {
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
  uint32_t r8;
  uint32_t r9;
  uint32_t r10;
  uint32_t r11;
  uint32_t lr;
#ifdef ENABLE_FP
  uint32_t s16;
  uint32_t s17;
  uint32_t s18;
  uint32_t s19;
  uint32_t s20;
  uint32_t s21;
  uint32_t s22;
  uint32_t s23;
  uint32_t s24;
  uint32_t s25;
  uint32_t s26;
  uint32_t s27;
  uint32_t s28;
  uint32_t s29;
  uint32_t s30;
  uint32_t s31;
#endif /* ENABLE_FP */
};

struct regs {
  struct aux_regs aux;
  struct stacked_regs stacked;
};

#define TASK_STATE_ACTIVE      (1<<0)  // The current active task
#define TASK_STATE_READY       (1<<1)  // Task is waiting to run
#define TASK_STATE_SLEEP       (1<<2)  // Task is asleep
#define TASK_STATE_WAIT_MUTEX  (1<<3)  // Task is asleep
#define TASK_STATE_WAIT_EVENT  (1<<4)  // Task is asleep

#define WAIT_MUTEX 1
#define WAIT_EVENT 2

#endif /* TASK_H */
