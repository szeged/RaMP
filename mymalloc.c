#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>

#include <fcntl.h>

#include "mymalloc.h"
#include "mymark.h"

uint32_t peak = 0;
uint32_t max_peak = 0;

void* (*libc_malloc)(size_t) = NULL;
void (*libc_free)(void*) = NULL;
void* (*libc_realloc)(void*, size_t) = NULL;
void* (*libc_calloc)(size_t, size_t) = NULL;
void* (*libc_valloc)(size_t) = NULL;
void* (*libc_pvalloc)(size_t) = NULL;
void* (*libc_memalign)(size_t, size_t) = NULL;

void* (*libmman_mmap)(void*, size_t, int, int, int, off_t) = NULL;
void* (*libmman_mmap64) (void *, size_t, int, int, int, off64_t) = NULL;
int (*libmman_munmap)(void*, size_t) = NULL;
void* (*libmman_mremap) (void *, size_t, size_t, int, void *) = NULL;

int (*exit_real)(int status) = NULL;


void* mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
  void* ptr;

  if (!libmman_mmap) {
    libmman_mmap = dlsym(RTLD_NEXT, "mmap");
    printf(REGPREFIX "Register libmman_mmap: %p\n", libmman_mmap);
  }

  ptr = libmman_mmap(addr, length, prot, flags, fd, offset);
#if PRINTLOG
  printf(LOGPREFIX "mmap: %x %p %zu\n", (int)ptr, addr, length);
#endif
  return ptr;
}

void* mmap64(void *addr, size_t length, int prot, int flags, int fd, off64_t offset)
{
  void* ptr;

  if (!libmman_mmap64) {
    libmman_mmap64 = dlsym(RTLD_NEXT, "mmap64");
    printf(REGPREFIX "Register libmman_mmap64: %p\n", libmman_mmap64);
  }

  ptr = libmman_mmap64(addr, length, prot, flags, fd, offset);
#if PRINTLOG
  printf(LOGPREFIX "mmap64: %x %p %zu\n", (int)ptr, addr, length);
#endif
  return ptr;
}

int munmap(void *addr, size_t length)
{
  int value;

  if (!libmman_munmap) {
    libmman_munmap = dlsym(RTLD_NEXT, "munmap");
    printf(REGPREFIX "Register libmman_munmap: %p\n", libmman_munmap);
  }

  value = libmman_munmap(addr, length);
#if PRINTLOG
  printf(LOGPREFIX "munmap: %x %p %zu\n", value, addr, length);
#endif
  return value;
}

void* mremap(void *addr, size_t old_length, size_t length, int flags, ...)
{
  void *ptr;
  va_list ap;

  va_start (ap, flags);
  void *newaddr = (flags & MREMAP_FIXED) ? va_arg (ap, void *) : NULL;
  va_end (ap);

  if (!libmman_mremap) {
      libmman_mremap = dlsym(RTLD_NEXT, "mremap");
      printf(REGPREFIX "Register libmman_mremap: %p\n", libmman_mremap);
  }

  ptr = libmman_mremap(addr, old_length, length, flags, newaddr);
#if PRINTLOG
    printf(LOGPREFIX "mremap: %x %p %zu\n", (int)ptr, addr, length);
#endif
  return ptr;
}

void* malloc(size_t size)
{
  void* ptr;

  if (!size)
  	return NULL;

  if (!libc_malloc) {
    libc_malloc = dlsym(RTLD_NEXT, "malloc");
    printf(REGPREFIX "Register libc_malloc: %p\n", libc_malloc);
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
    printf(REGPREFIX "Register libc_free: %p\n", libc_free);
  }

  size = free_mark(ptr);
  calc_peak(-size);

#if PRINTLOG
  printf(LOGPREFIX "free   %p, [%12d] -%d\n", ptr, peak, size);
#endif
  libc_free(ptr);
}

void* realloc(void* ptr, size_t size)
{
  size_t prev_size;
  void* new_ptr;

  if (!libc_realloc) {
    libc_realloc = dlsym(RTLD_NEXT, "realloc");
    printf(REGPREFIX "Register libc_realloc: %p\n", libc_realloc);
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
    printf(REGPREFIX "Register libc_calloc: %p\n", libc_calloc);
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
    printf(REGPREFIX "Register libc_valloc: %p\n", libc_valloc);
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
    printf(REGPREFIX "Register libc_pvalloc: %p\n", libc_pvalloc);
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
    printf(REGPREFIX "Register libc_memalign: %p\n", libc_memalign);
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
    printf(REGPREFIX "Register libc_memalign: %p\n", libc_memalign);
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
        printf(REGPREFIX "Register exit_real: %p\n", exit_real);
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
