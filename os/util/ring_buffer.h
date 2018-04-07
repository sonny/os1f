/*
 * ring_buffer.h
 *
 *  Created on: Apr 7, 2018
 *      Author: sonny
 */

#ifndef OS_UTIL_RING_BUFFER_H_
#define OS_UTIL_RING_BUFFER_H_

#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <stdbool.h>
#include <stdint.h>
#include "spinlock.h"

//#define RING_BUFFER_SIZE ((uint8_t)128)
#define RING_BUFFER_TOO_CLOSE ((uint8_t)16)

// Careful there, optimizing cowboy....
// If you use any bigger size, you are
// going to have troubles unless you use bigger
// data types.
typedef struct {
	size_t size;
	size_t start;
	size_t end;
	spinlock_t lock;
	uint8_t * buffer;
} ring_buffer_t;

#define RB_STATIC_ALLOCATE(NAME, SIZE) \
	struct {							\
		ring_buffer_t rb;				\
		uint8_t buffer[SIZE];			\
	} NAME

#define RB_STATIC_INIT(NAME, SIZE) { \
	{ .size = SIZE,					\
	  .start = 0,					\
	  .end = 0,						\
	  .lock = SPINLOCK_UNLOCKED,    \
	  .buffer = NAME.buffer }, {0}  \
	}

#define RB_STATIC_CREATE(NAME, SIZE) \
		RB_STATIC_ALLOCATE(NAME, SIZE) = RB_STATIC_INIT(NAME, SIZE)

typedef struct {
  void (* const init)(ring_buffer_t*, size_t);
  bool (* const full)(const ring_buffer_t*);
  bool (* const empty)(const ring_buffer_t*);
  bool (* const almost_full)(const ring_buffer_t*);
  void (* const insert)(const ring_buffer_t*, char);
  void (* const insert_string)(const ring_buffer_t*, const char*, int);
  char (* const remove)(const ring_buffer_t*);
  uint8_t (* const used)(const ring_buffer_t *);
} ringbuffer_class_t;

extern const ringbuffer_class_t const Ringbuffer;

#endif /* RING_BUFFER_H_ */

#endif /* OS_UTIL_RING_BUFFER_H_ */
