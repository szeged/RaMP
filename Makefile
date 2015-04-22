CFLAGS=-O0 -g -fno-omit-frame-pointer
LDFLAGS=-Wl,--wrap=malloc -Wl,--wrap=free
CC=gcc

all: mymalloc.so

mymalloc.so: mymalloc.o
	$(CC) $< -shared -o $@

mymalloc.o: mymalloc.c
	$(CC) $< -c -fpic -o $@


run: mymalloc.so
	LD_PRELOAD=./mymalloc.so xeyes
