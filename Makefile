CC=gcc -ansi -std=c99 -I.

inarray: main_inarray.c
	gcc -o main_inarray main_inarray.c

clean:
	rm -f main_inarray