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
void* (*libc_realloc)(void*, size_t) = NULL;
void* (*libc_calloc)(size_t, size_t) = NULL;
void* (*libc_valloc)(size_t) = NULL;
void* (*libc_pvalloc)(size_t) = NULL;
void* (*libc_memalign)(size_t, size_t) = NULL;


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


void* realloc(void* ptr, size_t size)
{
  size_t prev_size;
  void* new_ptr;
  if (!libc_realloc) {
    libc_realloc = dlsym(RTLD_NEXT, "__libc_realloc");
    printf(LOGPREFIX "Register __libc_realloc: %p\n", libc_realloc);
  }

  prev_size = free_mark(ptr);
  peak -= prev_size;

  new_ptr = libc_realloc(ptr, size);
  mark(new_ptr, size);

  peak += size;
  printf(LOGPREFIX "reall. %p, [%12d]  %ld\n", new_ptr, peak, (long)size-(long)prev_size);

  return new_ptr;
}

void* calloc(size_t nmemb, size_t size)
{
  void* ptr;
  if (!libc_calloc) {
    libc_calloc = dlsym(RTLD_NEXT, "__libc_calloc");
    printf(LOGPREFIX "Register __libc_calloc: %p\n", libc_calloc);
  }

  ptr = libc_calloc(nmemb, size);
  mark(ptr, nmemb * size);
  peak += nmemb * size;

  printf(LOGPREFIX "calloc %p, [%12d]  %zu\n", ptr, peak, nmemb * size);

  return ptr;
}

void* valloc(size_t size)
{
  if (!libc_valloc) {
    libc_valloc = dlsym(RTLD_NEXT, "__libc_valloc");
    printf(LOGPREFIX "Register __libc_valloc: %p\n", libc_valloc);
  }

  printf(LOGPREFIX "valloc\n");
  /*TODO: fix the alignment. */
  return malloc(size);
}
void* pvalloc(size_t size)
{
  if (!libc_pvalloc) {
    libc_pvalloc = dlsym(RTLD_NEXT, "__libc_pvalloc");
    printf(LOGPREFIX "Register __libc_pvalloc: %p\n", libc_pvalloc);
  }
  printf(LOGPREFIX "pvalloc\n");
  /* TODO: fix the alignment. */
  return malloc(size);
}
void* memalign(size_t alignment, size_t size)
{
  if (!libc_memalign) {
    libc_memalign = dlsym(RTLD_NEXT, "__libc_memalign");
    printf(LOGPREFIX "Register __libc_memalign: %p\n", libc_memalign);
  }
  printf(LOGPREFIX "memalign\n");
  /*TODO: fix the alignment. */
  return malloc(size);
}


/* Internals */

void* __libc_malloc(size_t size)
{
  return malloc(size);
}

void* __libc_realloc(void* ptr, size_t size)
{
  return realloc(ptr, size);
}

void* __libc_calloc(size_t nmemb, size_t size)
{
  return calloc(nmemb, size);
}

void* __libc_valloc(size_t size)
{
  return valloc(size);
}
void* __libc_pvalloc(size_t size)
{
  return pvalloc(size);
}
void* __libc_memalign(size_t alignment, size_t size)
{
  return memalign(alignment, size);
}


int main()
{
  return 0;
}

