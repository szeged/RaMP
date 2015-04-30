#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <fcntl.h>

#include "mymalloc.h"
#include "mymark.h"

uint32_t peak = 0;
uint32_t max_peak = 0;

void* (*libc_malloc)(size_t) = NULL;
void (*libc_free)(void*) = NULL;
void *(*libmman_mmap)(void*, size_t, int, int, int, off_t) = NULL;
int (*libmman_munmap)(void*, size_t) = NULL;
void* (*libc_realloc)(void*, size_t) = NULL;
void* (*libc_calloc)(size_t, size_t) = NULL;
void* (*libc_valloc)(size_t) = NULL;
void* (*libc_pvalloc)(size_t) = NULL;
void* (*libc_memalign)(size_t, size_t) = NULL;
int (*exit_real)(int status) = NULL;

void* malloc(size_t size)
{
  void* ptr;

  if (!size)
  	return NULL;

  if (!libc_malloc) {
    libc_malloc = dlsym(RTLD_NEXT, "malloc");
    printf(LOGPREFIX "Register libc_malloc: %p\n", libc_malloc);
  }

  ptr = libc_malloc(size);
  mark(ptr, size);
  calc_peak(size);

#if PRINTLOG
  printf(LOGPREFIX "malloc %p, [%12d]  %zu\n", ptr, peak, size);
#endif
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
  calc_peak(-size);

#if PRINTLOG
  printf(LOGPREFIX "free   %p, [%12d] -%d\n", ptr, peak, size);
#endif
  libc_free(ptr);
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
  if (!libmman_mmap) {
    libmman_mmap = dlsym(RTLD_NEXT, "mmap");
    printf(LOGPREFIX "Register libmman_mmap: %p\n", libmman_mmap);
  }

#if PRINTLOG
  printf(LOGPREFIX "mmap: %p %zu\n", addr, length);
#endif
  libmman_mmap(addr, length, prot, flags, fd, offset);
}

int munmap(void *addr, size_t length)
{
  if (!libmman_munmap) {
    libmman_munmap = dlsym(RTLD_NEXT, "munmap");
    printf(LOGPREFIX "Register libmman_munmap: %p\n", libmman_munmap);
  }

#if PRINTLOG
  printf(LOGPREFIX "munmap: %p %zu\n", addr, length);
#endif
  return libmman_munmap(addr, length);
}


void* realloc(void* ptr, size_t size)
{
  size_t prev_size;
  void* new_ptr;

  if (!libc_realloc) {
    libc_realloc = dlsym(RTLD_NEXT, "realloc");
    printf(LOGPREFIX "Register libc_realloc: %p\n", libc_realloc);
  }

  prev_size = free_mark(ptr);
  calc_peak(-prev_size);

  new_ptr = libc_realloc(ptr, size);
  mark(new_ptr, size);
  calc_peak(size);

#if PRINTLOG
  printf(LOGPREFIX "reall. %p, [%12d]  %ld\n", new_ptr, peak, (long)size-(long)prev_size);
#endif
  return new_ptr;
}

void* calloc(size_t nmemb, size_t size)
{
  void* ptr;

  if (!libc_calloc) {
    libc_calloc = dlsym(RTLD_NEXT, "calloc");
    printf(LOGPREFIX "Register libc_calloc: %p\n", libc_calloc);
  }

  ptr = libc_calloc(nmemb, size);
  mark(ptr, nmemb * size);
  calc_peak(nmemb * size);

#if PRINTLOG
  printf(LOGPREFIX "calloc %p, [%12d]  %zu\n", ptr, peak, nmemb * size);
#endif
  return ptr;
}

void* valloc(size_t size)
{
  void* ptr;

  if (!libc_valloc) {
    libc_valloc = dlsym(RTLD_NEXT, "valloc");
    printf(LOGPREFIX "Register libc_valloc: %p\n", libc_valloc);
  }

  ptr = libc_valloc(size);
  mark(ptr, size);
  calc_peak(size);

#if PRINTLOG
  printf(LOGPREFIX "valloc %p, [%12d]  %zu\n", ptr, peak, size);
#endif
}

void* pvalloc(size_t size)
{
  void* ptr;

  if (!libc_pvalloc) {
    libc_pvalloc = dlsym(RTLD_NEXT, "pvalloc");
    printf(LOGPREFIX "Register libc_pvalloc: %p\n", libc_pvalloc);
  }

  ptr = libc_pvalloc(size);
  mark(ptr, size);
  calc_peak(size);

#if PRINTLOG
  printf(LOGPREFIX "pvalloc %p, [%12d]  %zu\n", ptr, peak, size);
#endif
}

void* memalign(size_t alignment, size_t size)
{
  void* ptr;

  if (!libc_memalign) {
    libc_memalign = dlsym(RTLD_NEXT, "memalign");
    printf(LOGPREFIX "Register libc_memalign: %p\n", libc_memalign);
  }

  ptr = libc_memalign(alignment, size);
  mark(ptr, size);
  calc_peak(size);

#if PRINTLOG
  printf(LOGPREFIX "memalign %p, [%12d]  %zu\n", ptr, peak, size);
#endif
  return ptr;
}

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
  if (!libc_memalign) {
    libc_memalign = dlsym(RTLD_NEXT, "memalign");
    printf(LOGPREFIX "Register libc_memalign: %p\n", libc_memalign);
  }

  if (alignment % sizeof(void *) != 0)
    return EINVAL;

  *memptr = libc_memalign(alignment, size);
  mark(*memptr, size);
  calc_peak(size);

#if PRINTLOG
  printf(LOGPREFIX "posix_memalign %p, [%12d]  %zu\n", *memptr, peak, size);
#endif
  return (*memptr != NULL ? 0 : ENOMEM);
}

void exit(int status) {

    if (!exit_real) {
        exit_real = dlsym(RTLD_NEXT, "exit");
        printf(LOGPREFIX "Register exit_real: %p\n", exit_real);
    }

    printf(LOGPREFIX "[%5d] exit(%d) leak[%12d] peak[%12d] \n",getpid(), status, peak, max_peak);
    exit_real(status);
}


/* Internals */

void* __libc_malloc(size_t size)
{
  return malloc(size);
}

void __libc_free(void *ptr)
{
  free(ptr);
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

int __libc_posix_memalign(void **memptr, size_t alignment, size_t size)
{
  return posix_memalign(memptr, alignment, size);
}

void _exit(int status)
{
  exit(status);
}

void _Exit(int status)
{
  exit(status);
}

int main()
{
  return 0;
}

void calc_peak(size_t size){
  peak += size;
  if(max_peak < peak)
    max_peak = peak;
}
