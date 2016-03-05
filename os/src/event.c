#include "event.h"
#include "stm32f746xx.h"

void event_init(struct event *e)
{
  bitfield_init(&e->waiting);
}

void event_add_wait(struct event *e)
{
  const struct task *t = task_current();
  bitfield_set(&e->waiting, t->id);
  task_change_state(TASK_STATE_WAIT_EVENT);
  __DMB();
}

void event_wait(struct event *e)
{
  const struct task *t = task_current();
  if (!bitfield_is_set(e->waiting, t->id) ||
      t->state == TASK_STATE_WAIT_EVENT) {
    bitfield_set(&e->waiting, t->id);
    task_wait(WAIT_EVENT); /* yields until notify */
  }
  
  bitfield_clear(&e->waiting, t->id);
}

void event_notify(struct event *e)
{
  int i;
  /* notify all waiting tasks */
  for (i = 0; i < TASK_COUNT; ++i) {
    if (bitfield_is_set(e->waiting, i)) {
      if (task_get(i)->state == TASK_STATE_WAIT_EVENT) {
        bitfield_clear(&e->waiting, i);
        __DMB(); 
        task_notify(i); 
      }
    }
    else {
      if (task_get(i)->state != TASK_STATE_WAIT_EVENT) {
        bitfield_set(&e->waiting, i);
      }
    }



    /* if (bitfield_is_set(e->waiting, i) && */
    /*     task_get(i)->state == TASK_STATE_WAIT_EVENT)   { */
    /*   bitfield_clear(&e->waiting, i); */
    /*   __DMB(); */
    /*   task_notify(i); */
    /* } */
  }
}
