CC = gcc

all: clean shared example

shared:
	$(CC) -lsndfile -lfftw3 -c -fpic libsia.c
	$(CC) -shared -o libsia.so libsia.o

static:
	$(CC) -lsndfile -lfftw3 -c libsia.c
	ar rs libsia.a libsia.o

example:
	$(CC) -lportaudio -lsndfile example.c -o example

.PHONY: clean

clean: 
	rm -f libsia.a libsia.o libsia.so example
