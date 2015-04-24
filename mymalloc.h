#ifndef _MYMALLOC_H_
#define _MYMALLOC_H_

#include <stdint.h>
#include <sys/mman.h>

#define LOGPREFIX "[---] "

extern uint32_t peak;

extern void* (*libc_malloc)(size_t);
extern void (*libc_free)(void*);

extern void* malloc(size_t c);
extern void free(void *ptr);


extern void *(*libmman_mmap)(void*, size_t, int, int, int, off_t);
extern int (*libmman_munmap)(void*, size_t);

extern void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
extern int munmap(void *addr, size_t length);

#endif /* _MYMALLOC_H_ */
