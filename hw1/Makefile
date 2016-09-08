# Define (atleast) the following targets: all, clean
# all must build the executable file named mapreduce.
# clean must remove all object files and executable files created.
# Look at the introduction doc for information on how make a Makefile.
# This make file must build according to the structure described in the
# introduction doc, if it doesn't you will get a ZERO!

CFLAGS=-Wall -Werror
VPATH=./include:./src

all : map_reduce

map_reduce: map_reduce.o main.o
	gcc -o map_reduce $^ 

%.o : %.c
	gcc -c $(CFLAGS) $^ -I./include

clean:
	-@rm -rf *.o
	-@rm -rf map_reduce