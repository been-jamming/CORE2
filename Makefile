all: dictionary.o types.c
	gcc dictionary.o types.c -g -o test

dictionary.o: dictionary.c
	gcc -c dictionary.c

