CFLAGS=-O0 -g -fno-omit-frame-pointer -fpic
LIBFLAGS=-shared -Wl,--version-script=mymalloc.version -ldl
CC=gcc

SRCS=mymalloc.c mymark.c
OBJS=${SRCS:.c=.o}
LIBS=mymalloc.so

.PHONY: all
all: ${LIBS}

mymalloc.so: ${OBJS}
	$(CC) $? ${LIBFLAGS} -o $@

%.o: %.c
	$(CC) $< -c ${CFLAGS} -o $@

.PHONY: clean
clean:
	rm -f ${OBJS} ${LIBS}

.PHONY: test
test: test.out mymalloc.so
	LD_PRELOAD=./mymalloc.so ./test.out

test.out: test.o
	$(CC) $? ${CFLAGS} -o $@

.PHONY: run
run: mymalloc.so
	LD_PRELOAD=./mymalloc.so xeyes
