#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

struct list {
  struct list *prev;
  struct list *next;
};

#define LIST_STATIC_INIT(name) { &(name), &(name) }

void printlist(struct list *head);

/**
 * Initialize list 
 *
 * @param head : List head
 * @return void
 */
static inline
void list_init(struct list *head)
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
static inline
int list_size(const struct list *head)
{
  assert(0 && "Not implemented");
  return 0;
}

/**
 * Is Empty predicate
 *
 * @param head : List head
 * @return true if list is empty
 */
static inline
bool list_empty(const struct list *head)
{
  assert(head && "List Head is a Null Pointer");
  return (head->next == head);
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
static inline
void list_insert(struct list * restrict new, struct list *prev, struct list *next)
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
static inline
void list_addAtFront(struct list * restrict head, struct list * restrict new)
{
  assert(head && new && "List Head is a Null Pointer");
  assert(new->next == new && new->prev == new && "New node is not initialized");
  list_insert(new, head, head->next);
}

/**
 * Insert node into rear of list
 *
 * @param head : List head
 * @param new  : Node to insert
 * @return void
 */
static inline
void list_addAtRear(struct list * restrict head, struct list * restrict new)
{
  assert(head && new && "List Head is a Null Pointer");
  assert(new->next == new && new->prev == new && "New node is not initialized");
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
static inline
void list_remove(struct list *node)
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
static inline
struct list *list_removeNode(struct list * restrict head, struct list * restrict node)
{
  assert(head && node && "List Node is a Null Pointer");
  if (list_empty(head)) return NULL;
  list_remove(node);
  return node;
}

/**
 * Remove node from front of list
 *
 * @param head : List head
 * @return first node in list
 */
static inline
struct list* list_removeFront(struct list *head)
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
static inline
struct list *list_removeRear(struct list *head)
{
  assert(head && "List Head is a Null Pointer");
  return list_removeNode(head, head->prev);
}

#define LIST_FOR_EACH_SAFE(pos, tmp, head)                      \
  for (pos = (head)->next, tmp = pos->next; pos != (head);      \
       pos = tmp, tmp = pos->next)


/**
 * Conditionally call function on each element of list 
 *
 * @param head    : List head
 * @param func    : Function to call on each element if pred is true for that element
 * @param pred    : Predicate function returns bool for each element
 * @param context : Data passed to pred and func
 * @return void
 */
static inline
void list_each_do_if(struct list *head,
                     void (*func)(struct list*, const void*),
                     bool (*pred)(struct list*, const void*),
                     const void * context) {
  struct list *pos, *tmp;
  for (pos = (head)->next, tmp = pos->next;
       pos != (head);
       pos = tmp, tmp = pos->next) {

    // if pred is NULL, then we assume true
    if (!pred || pred(pos, context)) {
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
static inline
void list_each_do(struct list *head,
               void (*func)(struct list*, const void*),
               const void *context) {
  list_each_do_if(head, func, NULL, context);
}

#endif  /* __LIST_H__ */
