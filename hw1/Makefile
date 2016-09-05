# Define (atleast) the following targets: all, clean
# all must build the executable file named mapreduce.
# clean must remove all object files and executable files created.
# Look at the introduction doc for information on how make a Makefile.
# This make file must build according to the structure described in the
# introduction doc, if it doesn't you will get a ZERO!

CFLAGS=-Wall -Werror
VPATH=./include:./src

all : functions.o
	@echo Create $^

%.o : %.c
	@echo Compiling $^ file
	gcc -c $(CFLAGS) $^ -I./include

clean:
	-@rm -rf *.o