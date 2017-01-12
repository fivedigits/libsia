CC = gcc

bin:
	$(CC) -lsox -lfftw3 -lm libsia.c -o libsia 
