/*
 * virtled.h
 *
 *  Created on: Apr 24, 2018
 *      Author: sonny
 */

#ifndef OS_SERVICES_VIRTLED_H_
#define OS_SERVICES_VIRTLED_H_

enum {
	VLED0 = 0,
	VLED1,
	VLED2,
	VLED3,
	VLED4,
	VLED5,
	VLED6,
	VLED7,
	VLED8,
	VLED9,
	VLED10,
	VLED11,
	VLED12,
	VLED13,
	VLED14,
	VLED15
};

void virtled_init(void);
void virtled_set(int i);
void virtled_reset(int i);
void virtled_toggle(int i);

#endif /* OS_SERVICES_VIRTLED_H_ */
