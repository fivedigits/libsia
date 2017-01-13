CC = gcc

all: shared static

shared:
	$(CC) -lsox -lfftw3 -lm -c -fpic libsia.c
	$(CC) -shared -o libsia.so libsia.o

static:
	$(CC) -lsox -lfftw3 -lm -c libsia.c
	ar rs libsia.a libsia.o

.PHONY: clean

clean: 
	rm libsia.a libsia.o libsia.so
