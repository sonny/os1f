/*
 * heap.h
 *
 * Created: 10/15/2013 7:41:01 PM
 *  Author: Greg Cook
 *
 * heap data structure
 */ 


#ifndef __HEAP_H__
#define __HEAP_H__

#include <stdbool.h>

#define STATIC_HEAP_SIZE 8

typedef uint32_t heap_key_t;
typedef int8_t heap_index_t;

typedef enum { UNKNOWN, HEAP_MAX, HEAP_MIN } heap_type_t;

typedef struct heap_t heap_t;

typedef heap_key_t (*heap_get_key_fp)(const void*) __attribute__((const));
typedef bool (*heap_cmp_fp)(const heap_t *, heap_index_t, heap_index_t);

bool heap_cmp_min(const heap_t *heap, heap_index_t a, heap_index_t b);
bool heap_cmp_max(const heap_t *heap, heap_index_t a, heap_index_t b);

struct heap_t {
  int size;
  int max_size;
  heap_cmp_fp cmp;
  heap_get_key_fp get_key;
  void **data;
};

#define HEAP_STATIC_ALLOCATE(name, size) struct { heap_t heap; void * data[size]; } name 
#define HEAP_STATIC_INIT(name, size, cmp, key) { {0, size, cmp, key, &name.data[0]}, {0} }
#define HEAP_MAX_STATIC_CREATE(name, size, key)                         \
  HEAP_STATIC_ALLOCATE(name,size) = HEAP_STATIC_INIT(name,size,heap_cmp_max,key)

#define HEAP_MIN_STATIC_CREATE(name, size, key)                             \
  HEAP_STATIC_ALLOCATE(name,size) = HEAP_STATIC_INIT(name,size,heap_cmp_min,key)

typedef struct {
  void    (* const init)(heap_t*, heap_type_t, int, void*, heap_get_key_fp);
  bool    (* const insert)(heap_t*, void*);
  void   *(* const remove_head)(heap_t*);
  void   *(* const head)(const heap_t*);
  bool    (* const is_empty)(const heap_t*);
  bool    (* const is_full)(const heap_t*);
} heap_class_t;

extern heap_class_t Heap;

#endif /* __HEAP_H__ */

