CC = gcc

all: clean static example

static:
	$(CC) -static -c libsia.c -lsndfile -lfftw3 -lm -lcomplex
	ar -cvq libsia.a libsia.o

example:
	$(CC) -g -o example example.c ./libsia.a -lportaudio -lsndfile -lm -lfftw3 -latomic

.PHONY: clean

clean: 
	rm -f libsia.o libsia.a example
