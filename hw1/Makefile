# Define (atleast) the following targets: all, clean
# all must build the executable file named mapreduce.
# clean must remove all object files and executable files created.
# Look at the introduction doc for information on how make a Makefile.
# This make file must build according to the structure described in the
# introduction doc, if it doesn't you will get a ZERO!

# Compiler related
CC=gcc
CFLAGS=-Wall -Werror

# Directory related
MAKE_FOLDER=mkdir -p
# 	expected to exist
SRCDIR=./src
INCLUDEDIR=./include
# 	expected to not exist
BUILDDIR=./build
BINDIR=./bin

# Targets 
BINS=$(BINDIR)/mapreduce
OBJECTS = $(BUILDDIR)/map_reduce.o $(BUILDDIR)/main.o

# Actual Makefile begins
all : directories $(BINS)

$(BINS): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJECTS) : $(BUILDDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -I$(INCLUDEDIR) -o $@

directories:
	@$(MAKE_FOLDER) $(BINDIR)
	@$(MAKE_FOLDER) $(BUILDDIR)

clean:
	-@rm -rf $(BINDIR)
	-@rm -rf $(BUILDDIR)