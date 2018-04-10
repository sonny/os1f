/*
 * task_type.h
 *
 *  Created on: Apr 9, 2018
 *      Author: sonny
 */

#ifndef OS_CORE_TYPES_TASK_TYPE_H_
#define OS_CORE_TYPES_TASK_TYPE_H_

#include <stdint.h>
#include "list.h"
#include "event_type.h"

typedef enum {
	TASK_INACTIVE,
	TASK_ACTIVE,
	TASK_SLEEP,
	TASK_WAIT,
	TASK_END
} task_state_e;


typedef struct
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr; // r14
	uint32_t pc; // r15
	uint32_t xpsr;
} hw_stack_frame_t;

typedef struct
{
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
} sw_stack_frame_t;

typedef struct
{
	uint32_t s[16]; // s0-s15
	uint32_t fpscr;
	uint32_t reserved;
} hw_fp_stack_frame_t;

typedef struct
{
	uint32_t s[16]; // s16 - s31
} sw_fp_stack_frame_t;

typedef struct
{
	list_t node;
	const char * name;
	uint8_t * stack_top;
	uint8_t * sp;
	int32_t id;
	//uint32_t state;
	task_state_e state;
	uint32_t flags;
	uint32_t sleep_until;
	uint64_t runtime;
	uint64_t lasttime;
	sw_stack_frame_t sw_context;
#ifdef ENABLE_FPU
	sw_fp_stack_frame_t sw_fp_context;
#endif
	event_t join;
	uint32_t exc_return;
	const uint32_t signature;
} task_t __attribute__((aligned(8)));

typedef struct
{
	sw_stack_frame_t sw_frame;
	hw_stack_frame_t hw_frame;
} stack_frame_t;

typedef struct
{
	void (*func)(void*);
	int stack_size;
	void *context;
	const char * name;
	task_t *task;
} task_init_t;


#define TASK_STATIC_ALLOCATE(name, size)                \
  struct {                                              \
    task_t task;                                        \
    uint8_t stack[size] __attribute((aligned(8)));      \
  } name

#define TASK_STATIC_INIT(_name, _name_str, _id) {            \
    { .node = LIST_STATIC_INIT(_name.task.node),             \
		.signature = TASK_SIGNATURE, \
        .name = _name_str,                                   \
        .sp = &_name.stack[0] + sizeof(_name.stack),         \
        .stack_top = &_name.stack[0] + sizeof(_name.stack),  \
        .id = _id,                                           \
        .state = TASK_ACTIVE,                                \
	.flags = (TASK_FLAG_STATIC),                         \
	.sleep_until = 0,                                    \
	.lasttime = 0, \
	.runtime = 0, \
        .join = EVENT_STATIC_INIT(_name.task.join),          \
        .exc_return = 0xfffffffd }, {0}                      \
  }

#define TASK_STATIC_CREATE(name, name_str, size, id) \
  TASK_STATIC_ALLOCATE(name, size) = TASK_STATIC_INIT(name, name_str, id)

#endif /* OS_CORE_TYPES_TASK_TYPE_H_ */
