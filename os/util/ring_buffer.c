/*
 * ring_buffer.c
 *
 *  Created on: Apr 7, 2018
 *      Author: sonny
 */

#include <assert.h>
#include "ring_buffer.h"

/**
 * Initialize Ring Buffer
 * @param rb Ring Buffer object
 * @return void
 */
static
void ringbuffer_init(ring_buffer_t *rb, uint8_t * buffer, size_t size)
{
	rb->size = size;
	rb->start = 0;
	rb->end = 0;
	rb->lock = SPINLOCK_UNLOCKED;
	rb->buffer = buffer;
}

/**
 * Returns true if Ring Buffer is full
 * @param rb Ring Buffer object
 * @return true if rb is full
 */
static inline
bool ringbuffer_isFull(const ring_buffer_t * rb)
{
	spinlock_lock(&rb->lock);
	bool result = (((rb->end + 1) % rb->size) == rb->start);
	spinlock_unlock(&rb->lock);
	return result;
}

/**
 * Return true if Ring Buffer is empty
 * @param rb Ring Buffer object
 * @return true if rb is empty
 */
static inline
bool ringbuffer_isEmpty(const ring_buffer_t * rb)
{
	spinlock_lock(&rb->lock);
	bool result = (rb->start == rb->end);
	spinlock_unlock(&rb->lock);
	return result;
}

/**
 * Returns number of elements left in Ring Buffer
 * @param rb Ring Buffer object
 * @return count of elements remaining in rb
 */
static inline
uint8_t ringbuffer_remainder(const ring_buffer_t *rb)
{
	spinlock_lock(&rb->lock);
	uint8_t result = (rb->start + rb->size - rb->end) % rb->size;
	spinlock_unlock(&rb->lock);
	return result;
}

/**
 * Returns true if the remainder count is less than RING_BUFFER_TOO_CLOSE
 *
 * This function is used for serial comms to trigger hardware flow control
 * before the buffer overruns
 *
 * @param rb Ring Buffer object
 * @return true if rb remainder is too close
 */
static inline
bool ringbuffer_almost_full(const ring_buffer_t * rb)
{
	spinlock_lock(&rb->lock);
	bool result = (ringbuffer_remainder(rb) >= RING_BUFFER_TOO_CLOSE);
	spinlock_unlock(&rb->lock);
	return result;
}

/**
 * Inserts new element into the Ring Buffer.
 *
 * @param rb Ring Buffer object
 * @param c Element to insert
 * @return void
 */
static inline
void ringbuffer_insert_element(ring_buffer_t * const rb, char c)
{
  assert(!Ringbuffer.full(rb) && "Ring Buffer is Full.");
  spinlock_lock(&rb->lock);
  rb->buffer[rb->end] = c;

  rb->end = (rb->end + 1 ) % rb->size;
  spinlock_unlock(&rb->lock);
}

/**
 * Copy entire string into Ring buffer
 * Not actually used anywhere, as far as I know
 * @param rb Ring Buffer object
 * @param s String to copy
 * @return void
 */
static void
ringbuffer_insert_string(const ring_buffer_t * restrict rb, const char * restrict s, int len)
{
	int i;
	for (i = 0; i < len; ++i)
		ringbuffer_insert_element(rb, s[i]);
}

/**
 * Remove next element from Ring Buffer
 * @param rb Ring Buffer object
 * @return next element from ring buffer
 * @Note This does not account for the possibility that the Ring Buffer may be empty.
 *       Check First.
 */
static char
ringbuffer_extract_element(ring_buffer_t * const rb)
{
	spinlock_lock(&rb->lock);
	char c = rb->buffer[rb->start];
	rb->start = (rb->start + 1 ) % rb->size;
	spinlock_unlock(&rb->lock);
	return c;
}

/**
 * Calculates the used size of the ring buffer.
 *
 * This is the inverse operation of remainder. That is
 * Used + Remainder = Size
 *
 * @param rb Ring Buffer object
 * @return number of elements occupied in rb
 */
static
size_t ringbuffer_used(const ring_buffer_t *rb)
{
	spinlock_lock(&rb->lock);
	size_t result = (rb->size + rb->end - rb->start) % rb->size;
	spinlock_unlock(&rb->lock);
	return result;
}

const ringbuffer_class_t const Ringbuffer = {
  .init = ringbuffer_init,
  .full = ringbuffer_isFull,
  .empty = ringbuffer_isEmpty,
  .almost_full = ringbuffer_almost_full,
  .insert = ringbuffer_insert_element,
  .insert_string = ringbuffer_insert_string,
  .remove = ringbuffer_extract_element,
  .used = ringbuffer_used
};
