/*
 * bit_array.h
 *
 *  Created on: Apr 10, 2018
 *      Author: sonny
 */

#ifndef OS_UTIL_BIT_ARRAY_H_
#define OS_UTIL_BIT_ARRAY_H_

// declare a bitarray of _size bytes
#define BITARRAY_DECL(_name, _size) uint32_t[_size] _name
// declare a bitarray big enough for _bits bits
#define BITARRAY_DECL_BITS(_name, _bits) BITARRAY_DECL(_name, (_bits - 1 / 32) + 1)

static inline
void bitarray_set(uint32_t a[], size_t bit_pos)
{
	size_t idx = bit_pos / 32;
	size_t pos = bit_pos % 32;
	a[idx] |= (1 << pos);
}

static inline
void bitarray_clear(uint32_t a[], size_t bit_pos)
{
	size_t idx = bit_pos / 32;
	size_t pos = bit_pos % 32;
	a[idx] &= ~(1 << pos);
}

static inline
bool bitarray_test(uint32_t a[], size_t bit_pos)
{
	size_t idx = bit_pos / 32;
	size_t pos = bit_pos % 32;
	return a[idx] & (1 << pos);
}

static inline
void bitarray_each(uint32_t a[], void (*f)(int, void*), void * p)
{
	int i, j;
	for (i = 0; i < sizeof(a) * sizeof(uint32_t); ++i) {
		if (bitarray_test(a, i))
			f(i, p);
	}
}


#endif /* OS_UTIL_BIT_ARRAY_H_ */
