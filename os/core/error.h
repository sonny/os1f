/*
 * error.h
 *
 *  Created on: May 12, 2018
 *      Author: sonny
 */

#ifndef OS_CORE_ERROR_H_
#define OS_CORE_ERROR_H_

#include <limits.h>

typedef enum {
	OSERROR = INT_MIN,
	OSERR_VALUE,
	OSERR_SPACE,

	OS_OK = 0
} error_e;

#endif /* OS_CORE_ERROR_H_ */
