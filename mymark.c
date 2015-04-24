#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>

#include "mymalloc.h"
#include "mymark.h"

static alloc_t* alloc_head = NULL;

uint32_t hash(uintptr_t ptr)
{
  uint32_t val = (((uint32_t)ptr) >> 8) & 0xffff;
  return ((val >> 4) ^ val) & 0xff;
}

alloc_t* mark(void* ptr, size_t size)
{
  uint32_t h = hash((uintptr_t)ptr);
  alloc_t* alloc;

  alloc = (alloc_t*) libc_malloc(sizeof(alloc_t));
  alloc->addr  = (uintptr_t) ptr;
  alloc->size  = size;
  alloc->stack = NULL;
  alloc->next = alloc_head;
  if (alloc->next)
    alloc->next->prev = alloc;
  alloc->prev  = NULL;

  alloc_head = alloc;
  return alloc_head;
}

uint32_t free_mark(void* ptr)
{
  alloc_t* alloc;
  uint32_t size = 0;

  if (!ptr)
  	return;

  alloc = find(ptr);

  if (alloc) {
    size = alloc->size;
    if (!alloc->prev) {
      uint32_t h = hash((uintptr_t)ptr);
      alloc_head = alloc->next;
      if (alloc_head)
        alloc_head->prev = NULL;
    } else {
      alloc->prev->next = alloc->next;
      alloc->next->prev = alloc->prev;
    }
    libc_free(alloc);
  }
  
  return size;
}

alloc_t* find(void* ptr)
{
  uint32_t h = hash((uintptr_t)ptr);
  alloc_t* alloc = alloc_head;

  while (alloc && alloc->addr != (uintptr_t)ptr)
    alloc = alloc->next;

  return alloc;
}

/* Binary expansion */
#define DO_P2_TIMES_1(x)	DO_P2_TIMES_0(x); DO_P2_TIMES_0((x) + (1<<0))
#define DO_P2_TIMES_2(x)	DO_P2_TIMES_1(x); DO_P2_TIMES_1((x) + (1<<1))
#define DO_P2_TIMES_3(x)	DO_P2_TIMES_2(x); DO_P2_TIMES_2((x) + (1<<2))
#define DO_P2_TIMES_4(x)	DO_P2_TIMES_3(x); DO_P2_TIMES_3((x) + (1<<3))
#define DO_P2_TIMES_5(x)	DO_P2_TIMES_4(x); DO_P2_TIMES_4((x) + (1<<4))
#define DO_P2_TIMES_6(x)	DO_P2_TIMES_5(x); DO_P2_TIMES_5((x) + (1<<5))
#define DO_P2_TIMES_7(x)	DO_P2_TIMES_6(x); DO_P2_TIMES_6((x) + (1<<6))

static void* getreturnaddr(int level)
{
  switch(level) {
#define DO_P2_TIMES_0(x)  case (x): return __builtin_return_address((x) + 1)
    DO_P2_TIMES_4(0);
#undef DO_P2_TIMES_0
    default: return NULL;
  }
}

static void* getframeaddr(int level)
{
  switch(level) {
#define DO_P2_TIMES_0(x)  case (x): return __builtin_frame_address((x) + 1)
    DO_P2_TIMES_4(0);
#undef DO_P2_TIMES_0
    default: return NULL;
  }
}

int backtrace(void **buffer, int size)
{
  int i;
  for (i = 1; getframeaddr(i + 1) != NULL && i != size + 1; i++) {
  	printf("back %d\n", i);
    buffer[i - 1] = getreturnaddr(i);
    if (buffer[i - 1] == NULL)
      break;
    printf("end for\n");
  }
  return (i - 1);
}

#define REPEAT(x) do { \
  if (size == i) \
    return i; \
  tmp = __builtin_frame_address(x+1); \
  if (!tmp || !nullfd || (write(nullfd, tmp, 4) < 0)) \
    return i; \
  buffer[i] = __builtin_return_address(x); \
  if (!buffer[i]) \
  	return i; \
  i++; \
} while (0)

int backtrace2(void **buffer, int size)
{
  static int nullfd = 0;
  int i = 0;
  void* tmp;
  if (!nullfd)
    nullfd = open("/dev/random", O_WRONLY);
  REPEAT(1);
  REPEAT(2);
  REPEAT(3);
  REPEAT(4);
  REPEAT(5);
  REPEAT(6);
  REPEAT(7);
  REPEAT(8);
  REPEAT(9);
  REPEAT(10);
  return size;
}

void print_trace ()
{
  void* array[10];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace2(array, 10);
/*
  for (i = 0; i < size; i++)
     printf (" -- %p\n", array[i]);


  if (size > 0)
    strings = backtrace_symbols (array, size);
*/
  printf("OK3 size:%zu\n", size);
/*
  printf ("Obtained %zd stack frames.\n", size);

  for (i = 0; i < size; i++)
     printf ("%s\n", strings[i]);

  libc_free (strings);
*/
}
