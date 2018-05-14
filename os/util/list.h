#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

typedef struct list list_t;

struct list
{
	list_t *prev;
	list_t *next;
};

#define LIST_STATIC_INIT(name) { &(name), &(name) }

#define list_head(list) ((list)->next)
#define list_tail(list) ((list)->prev)

void printlist(list_t *head);

/**
 * Initialize list 
 *
 * @param head : List head
 * @return void
 */
__attribute__((always_inline)) static inline
void list_init(list_t *head)
{
	assert(head && "List Head is a Null Pointer");
	head->prev = head;
	head->next = head;
}

/**
 * Get size of linked list
 *
 * @param head : List head
 * @return size of list
 * @note Not Implemented
 */
__attribute__((always_inline)) static inline
int list_size(const list_t *head)
{
	(void) head;
	assert(0 && "Not implemented");
	return 0;
}

/**
 * Is Empty predicate
 *
 * @param head : List head
 * @return true if list is empty
 */
__attribute__((always_inline)) static inline
bool list_empty(const list_t const * head)
{
	assert(head && "List Head is a Null Pointer");
	return (head->next == head);
}

__attribute__((always_inline)) static inline
bool list_element(const list_t const * node)
{
	return !list_empty(node);
}

/**
 * Intert node into list
 *
 * For inserting a new node when the next and prev are known
 *
 * @param new  : Node to insert
 * @param prev : Node before new
 * @param next : Node to follow new
 * @Note prev and next can be aliased to each other
 * @return void
 */
__attribute__((always_inline)) static inline
void list_insert(list_t * restrict new, list_t *prev, list_t *next)
{
	assert(new && prev && next && "Invalid list pointers");
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * Insert node into front of list
 *
 * @param head : List head
 * @param new  : Node to insert
 * @return void
 */
__attribute__((always_inline)) static inline
void list_addAtFront(list_t * restrict head, list_t * restrict new)
{
	assert(head && new && "List Head is a Null Pointer");
	assert(
			new->next == new && new->prev == new
					&& "New node is not initialized");
	list_insert(new, head, head->next);
}

/**
 * Insert node into rear of list
 *
 * @param head : List head
 * @param new  : Node to insert
 * @return void
 */
__attribute__((always_inline)) static inline
void list_addAtRear(list_t * restrict head, list_t * restrict new)
{
	assert(head && new && "List Head is a Null Pointer");
	assert(
			new->next == new && new->prev == new
					&& "New node is not initialized");
	list_insert(new, head->prev, head);
}

/**
 * Remove node from list
 *
 * @param node : Node to remove from list
 * @return void
 * @note This function does not require the node to belong to any particular list. It ensures
 *       that it is removed from any that it might belong to.
 */
__attribute__((always_inline)) static inline
void list_remove(list_t *node)
{
	assert(node && "List node is a Null Pointer");
	node->next->prev = node->prev;
	node->prev->next = node->next;
	list_init(node);
}

/**
 * Remove node from list
 *
 * @param head : List head that node belongs to
 * @param node : Node to remove
 * @return node that was removed
 */
__attribute__((always_inline)) static inline
list_t *list_removeNode(list_t * restrict head,
		list_t * restrict node)
{
	assert(head && node && "List Node is a Null Pointer");
	if (list_empty(head))
		return NULL;
	list_remove(node);
	return node;
}

/**
 * Remove node from front of list
 *
 * @param head : List head
 * @return first node in list
 */
__attribute__((always_inline)) static inline
list_t* list_removeFront(list_t *head)
{
	assert(head && "List Head is a Null Pointer");
	return list_removeNode(head, head->next);
}

/**
 * Remove node from rear of list
 *
 * @param head : List head
 * @return last node in list
 */
__attribute__((always_inline)) static inline
list_t *list_removeRear(list_t *head)
{
	assert(head && "List Head is a Null Pointer");
	return list_removeNode(head, head->prev);
}

#define LIST_FOR_EACH_SAFE(pos, tmp, head)                      \
  for (pos = (head)->next, tmp = pos->next; pos != (head);      \
       pos = tmp, tmp = pos->next)

__attribute__((always_inline)) static inline
bool list_element_of(list_t * node, list_t * list)
{
	list_t *pos, *tmp;
	LIST_FOR_EACH_SAFE(pos, tmp, list)
	{
		if (pos == node)
			return true;
	}
	return false;
}

__attribute__((always_inline)) static inline
void list_insert_condition(list_t *head, list_t *node, bool (*cond)(list_t*, list_t*))
{
	list_t *pos, *tmp;
	LIST_FOR_EACH_SAFE(pos, tmp, head)
	{
		if (cond(pos, node))
		{
			// insert new node before pos node
			list_insert(node, pos->prev, pos);
			return;
		}
	}
	// if we get here, insert it at the end
	list_addAtRear(head, node);
}

/**
 * Conditionally call function on each element of list 
 *
 * @param head    : List head
 * @param func    : Function to call on each element if pred is true for that element
 * @param pred    : Predicate function returns bool for each element
 * @param context : Data passed to pred and func
 * @return void
 */
__attribute__((always_inline)) static inline
void list_each_do_if(list_t *head, void (*func)(list_t*, const void*),
bool (*pred)(list_t*, const void*), const void * context)
{
	list_t *pos, *tmp;
	for (pos = (head)->next, tmp = pos->next; pos != (head); pos = tmp, tmp =
			pos->next)
	{
		// if pred is NULL, then we assume true
		if (!pred || pred(pos, context))
		{
			func(pos, context);
		}
	}
}

/**
 * Call function for each element in list
 *
 * @param head    : List head
 * @param func    : Function to call on each element
 * @param context : Data for fnc
 * @return void
 */
__attribute__((always_inline)) static inline
void list_each_do(list_t *head, void (*func)(list_t*, const void*),
		const void *context)
{
	list_each_do_if(head, func, NULL, context);
}

#endif  /* __LIST_H__ */
