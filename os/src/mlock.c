#include <malloc.h>
#include "mutex.h"

static struct mutex malloc_mutex = {0};

void __malloc_lock (struct _reent reent)
{
  mutex_lock(&malloc_mutex);
}

void __malloc_unlock (struct _reent reent)
{
  mutex_unlock(&malloc_mutex);
}