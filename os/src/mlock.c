#include <malloc.h>
#include "mutex.h"

static volatile mutex_t malloc_mutex = MUTEX_STATIC_INIT(malloc_mutex);

void __malloc_lock( __attribute__((unused)) struct _reent reent) {
	mutex_lock(&malloc_mutex);
}

void __malloc_unlock( __attribute__((unused)) struct _reent reent) {
	mutex_unlock(&malloc_mutex);
}
