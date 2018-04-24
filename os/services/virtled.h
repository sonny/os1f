/*
 * virtled.h
 *
 *  Created on: Apr 24, 2018
 *      Author: sonny
 */

#ifndef OS_SERVICES_VIRTLED_H_
#define OS_SERVICES_VIRTLED_H_

void virtled_init(void);
void virtled_set(int i);
void virtled_reset(int i);
void virtled_toggle(int i);

#endif /* OS_SERVICES_VIRTLED_H_ */
