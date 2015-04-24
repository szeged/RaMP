#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>

#include "mymalloc.h"
#include "mymark.h"

uint32_t peak = 0;

void* (*libc_malloc)(size_t) = NULL;
void (*libc_free)(void*) = NULL;

void* malloc(size_t c)
{
  void* ptr;

  if (!c)
  	return NULL;

  if (!libc_malloc) {
    libc_malloc = dlsym(RTLD_NEXT, "malloc");
    printf(LOGPREFIX "libc_malloc: %p\n", libc_malloc);
  }

  ptr = libc_malloc(c);

  mark(ptr, c);

  peak += c;
  printf(LOGPREFIX "malloc %p, [%12d]  %zu\n", ptr, peak, c);

  return ptr;
}

void free(void *ptr)
{
  uint32_t size;

  if (!ptr)
  	return;

  if (!libc_free) {
    libc_free = dlsym(RTLD_NEXT, "free");
    printf(LOGPREFIX "libc_free: %p\n", libc_free);
  }

  size = free_mark(ptr);
  peak -= size;

  printf(LOGPREFIX "free   %p, [%12d] -%d\n", ptr, peak, size);

  libc_free(ptr);
}

int main()
{
  return 0;
}

