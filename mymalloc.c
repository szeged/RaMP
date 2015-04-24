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
void *(*libmman_mmap)(void*, size_t, int, int, int, off_t) = NULL;
int (*libmman_munmap)(void*, size_t) = NULL;


void* malloc(size_t c)
{
  void* ptr;

  if (!c)
  	return NULL;

  if (!libc_malloc) {
    libc_malloc = dlsym(RTLD_NEXT, "malloc");
    printf(LOGPREFIX "Register libc_malloc: %p\n", libc_malloc);
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
    printf(LOGPREFIX "Register libc_free: %p\n", libc_free);
  }

  size = free_mark(ptr);
  peak -= size;

  printf(LOGPREFIX "free   %p, [%12d] -%d\n", ptr, peak, size);

  libc_free(ptr);
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
  if (!libmman_mmap)
    libmman_mmap = dlsym(RTLD_NEXT, "mmap");

  printf(LOGPREFIX "mmap: %p %zu\n", addr, length);

  libmman_mmap(addr, length, prot, flags, fd, offset);
}

int munmap(void *addr, size_t length)
{
  if (!libmman_munmap)
    libmman_munmap = dlsym(RTLD_NEXT, "munmap");

  printf(LOGPREFIX "munmap: %p %zu\n", addr, length);

  return libmman_munmap(addr, length);
}

int main()
{
  return 0;
}

