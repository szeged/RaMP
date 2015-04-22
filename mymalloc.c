#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <fcntl.h>

#define NUM_OF_HASH_GROUP 0xff

#define LOGPREFIX "[---] "

typedef struct alloc_s alloc_t;

struct alloc_s {
  uintptr_t addr;
  uint32_t size;
  void*    stack;

  alloc_t* next;
  alloc_t* prev;
};

static alloc_t* alloc_heads[NUM_OF_HASH_GROUP+1] = {NULL};
static uint32_t peak = 0;

uint32_t hash(uintptr_t ptr)
{
  uint32_t val = (((uint32_t)ptr) >> 8) & 0xffff;
  return ((val >> 4) ^ val) & 0xff;
}

alloc_t* mark(void* ptr, size_t size)
{
  void *(*libc_malloc)(size_t) = dlsym(RTLD_NEXT, "malloc");
  uint32_t h = hash((uintptr_t)ptr);
  alloc_t* alloc;

  alloc = (alloc_t*) libc_malloc(sizeof(alloc_t));
  alloc->addr  = (uintptr_t) ptr;
  alloc->size  = size;
  alloc->stack = NULL;
  alloc->next = alloc_heads[h];
  if (alloc->next)
    alloc->next->prev = alloc;
  alloc->prev  = NULL;

  return (alloc_heads[h] = alloc);
}

alloc_t* find(void* ptr)
{
  uint32_t h = hash((uintptr_t)ptr);
  alloc_t* alloc = alloc_heads[h];

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
  void (*libc_free)(void*);
  void* array[10];
  size_t size;
  char **strings;
  size_t i;

  libc_free = dlsym(RTLD_NEXT, "free");
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

void* malloc(size_t c)
{
  void* (*libc_malloc)(size_t);
  void* ptr;

  if (!c)
  	return NULL;

  libc_malloc = dlsym(RTLD_NEXT, "malloc");
  ptr = libc_malloc(c);

  mark(ptr, c);

  peak += c;
  printf(LOGPREFIX "malloc %p, [%12d]  %zu\n", ptr, peak, c);
  print_trace();

  return ptr;
}

void free(void *ptr)
{
  void (*libc_free)(void*);
  alloc_t* alloc;
  uint32_t size;

  if (!ptr)
  	return;

  libc_free = dlsym(RTLD_NEXT, "free");
  alloc = find(ptr);
  size = 0;
  if (alloc) {
    size = alloc->size;
    if (!alloc->prev) {
      uint32_t h = hash((uintptr_t)ptr);
      alloc_heads[h] = alloc->next;
      if (alloc_heads[h])
        alloc_heads[h]->prev = NULL;
    } else {
      alloc->prev->next = alloc->next;
      alloc->next->prev = alloc->prev;
    }
    libc_free(alloc);
    peak -= size;
  }
  printf(LOGPREFIX "free   %p, [%12d] -%d\n", ptr, peak, size);
  libc_free(ptr);
}

int main()
{
    return 0;
}

