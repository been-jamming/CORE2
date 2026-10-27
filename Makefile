all: dictionary.o types.c
	gcc commands.c dictionary.o types.c -Wall -g -o test

dictionary.o: dictionary.c
	gcc -c dictionary.c

