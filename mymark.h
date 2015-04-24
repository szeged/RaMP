#ifndef _MYMARK_H_
#define _MYMARK_H_

#include <stdint.h>

#define NUM_OF_HASH_GROUP 0xff

/* Binary expansion */
#define DO_P2_TIMES_1(x)	DO_P2_TIMES_0(x); DO_P2_TIMES_0((x) + (1<<0))
#define DO_P2_TIMES_2(x)	DO_P2_TIMES_1(x); DO_P2_TIMES_1((x) + (1<<1))
#define DO_P2_TIMES_3(x)	DO_P2_TIMES_2(x); DO_P2_TIMES_2((x) + (1<<2))
#define DO_P2_TIMES_4(x)	DO_P2_TIMES_3(x); DO_P2_TIMES_3((x) + (1<<3))
#define DO_P2_TIMES_5(x)	DO_P2_TIMES_4(x); DO_P2_TIMES_4((x) + (1<<4))
#define DO_P2_TIMES_6(x)	DO_P2_TIMES_5(x); DO_P2_TIMES_5((x) + (1<<5))
#define DO_P2_TIMES_7(x)	DO_P2_TIMES_6(x); DO_P2_TIMES_6((x) + (1<<6))

/* Allocation structure. */
typedef struct alloc_s alloc_t;

struct alloc_s {
  uintptr_t addr;
  uint32_t size;
  void*    stack;

  alloc_t* next;
  alloc_t* prev;
};


/* Generate a simple hash to speed up the search. */
uint32_t hash(uintptr_t ptr);

/* Mark allocations to be able to free up later. */
extern alloc_t* mark(void* ptr, size_t size);

/* Free allocation in the store. */
extern uint32_t free_mark(void* ptr);

/* Find an allocation in the storage. */
alloc_t* find(void* ptr);

/* For backtracing */
int backtrace(void **buffer, int size);
int backtrace2(void **buffer, int size);
void print_trace ();


#endif /* _MYMARK_H_ */
