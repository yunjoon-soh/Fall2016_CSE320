CC = gcc
BIN = utfconverter
CFLAGS = -g -Wall -Werror -pedantic -Wextra 

OBJECTS = ./build/utfconverter.o

# Folders
SRCDIR=./src
BUILDDIR=./build
BINDIR=./bin
INCLUDEDIR=./include

# Commands Shortcut
MAKE_FOLDER = mkdir -p
RM = rm -rf

all: directories $(BIN)

$(BIN) : $(OBJECTS)
	$(CC) $(CFLAGS) $^ -I$(INCLUDEDIR) -o $(BINDIR)/$@

$(OBJECTS) : $(BUILDDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -I$(INCLUDEDIR) -o $@

directories:
	@$(MAKE_FOLDER) $(BUILDDIR)
	@$(MAKE_FOLDER) $(BINDIR)
clean:
	-@$(RM) $(BINDIR)
	-@$(RM) $(BUILDDIR)
