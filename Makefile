CC = gcc -std=c99

all: main_fluency main_test

main_fluency: src/main_fluency.c src/main_fluency_inarray.c
	$(CC) -o main_fluency src/main_fluency.c
	$(CC) -o main_fluency_inarray src/main_fluency_inarray.c

main_test: src/main_test.c
	$(CC) -o main_test src/main_test.c

clean:
	rm -f main_* src/*.o
