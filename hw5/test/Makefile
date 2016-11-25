#Makefile
CC=gcc
CFLAGS=-Wall -Werror -g -DDEBUG

all: DIRS list_unit ../include/list.h


list_unit: ./build/list_unit.o ../build/list.o  
	$(CC) $(CFLAGS) $^ -o unit.out -I../include -lcriterion 

../build/%.o: ../src/%.c
	$(CC) $(CFLAGS) -I../include -c $< -o $@ 

./build/%.o: ./src/%.c
	$(CC) $(CFLAGS) -I./include -I../include -c $< -o $@ 

DIRS:
	mkdir -p ../build
	mkdir -p ./build

clean:
	rm -f *.o
	rm -f *.out
	rm -rf ./build