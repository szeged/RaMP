#ifndef _MYMALLOC_H_
#define _MYMALLOC_H_

#include <stdint.h>

#define LOGPREFIX "[---] "

extern uint32_t peak;

extern void* (*libc_malloc)(size_t);
extern void (*libc_free)(void*);

extern void* malloc(size_t c);
extern void free(void *ptr);

#endif /* _MYMALLOC_H_ */
