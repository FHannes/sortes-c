CC = gcc -std=c99

all: main main_inarray

main: src/main.c
	$(CC) -o main src/main.c

main_inarray: src/main.c
	$(CC) -o main_inarray src/main_inarray.c

clean:
	rm -f main_inarray src/*.o
