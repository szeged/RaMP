#ifndef _MYMALLOC_H_
#define _MYMALLOC_H_

#include <stdint.h>
#include <sys/mman.h>

#define LOGPREFIX "[---] "

extern uint32_t peak;

extern void* (*libc_malloc)(size_t);
extern void  (*libc_free)(void*);
extern void* (*libc_malloc)(size_t);
extern void* (*libc_realloc)(void*, size_t);
extern void* (*libc_calloc)(size_t, size_t);
extern void* (*libc_valloc)(size_t);
extern void* (*libc_pvalloc)(size_t);
extern void* (*libc_memalign)(size_t, size_t);

extern void* malloc(size_t c);
extern void free(void *ptr);


extern void *(*libmman_mmap)(void*, size_t, int, int, int, off_t);
extern int (*libmman_munmap)(void*, size_t);

extern void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
extern int munmap(void *addr, size_t length);

extern void* malloc(size_t size);
extern void* realloc(void* ptr, size_t size);
extern void* calloc(size_t nmemb, size_t size);
extern void* valloc(size_t size);
extern void* pvalloc(size_t size);
extern void* memalign(size_t alignment, size_t size);


/* Internals */

extern void* __libc_malloc(size_t size);
extern void* __libc_realloc(void* ptr, size_t size);
extern void* __libc_calloc(size_t nmemb, size_t size);
extern void* __libc_valloc(size_t size);
extern void* __libc_pvalloc(size_t size);
extern void* __libc_memalign(size_t alignment, size_t size);

#endif /* _MYMALLOC_H_ */
