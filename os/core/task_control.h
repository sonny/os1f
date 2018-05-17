/*
 * task_control.h
 *
 *  Created on: May 12, 2018
 *      Author: sonny
 */

#ifndef OS_TASK_CONTROL_H_
#define OS_TASK_CONTROL_H_

#define IDLE_TASK_ID    -1

void task_control_init(void);
int task_control_add(task_t * task);
int task_control_remove(task_t * task);
task_t * task_control_get(int id);
void task_control_each(void (*f)(task_t*));

task_t * task_control_get_idle_task(void);

#endif /* OS_TASK_CONTROL_H_ */
