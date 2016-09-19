# Folders
SRCDIR=src
BUILDDIR=build
BINDIR=bin
INCLUDEDIR=include

# Names
BIN = utf

# Compiler related
CC = gcc
CFLAGS = -g -Wall -Werror -pedantic -Wextra 

# Makefile related
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))
HDRS = $(wildcard $(INCLUDEDIR)/*.h)

# Commands Shortcut
MAKE_FOLDER = mkdir -p
RM = rm -rf

all: directories $(BIN)

$(BIN) : $(OBJECTS) $(HDRS)
	$(CC) $(CFLAGS) $(OBJECTS) -I$(INCLUDEDIR) -o $(BINDIR)/$@

$(OBJECTS): $(BUILDDIR)/%.o : $(SRCDIR)/%.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -I$(INCLUDEDIR) -o $@

directories:
	@$(MAKE_FOLDER) $(BUILDDIR)
	@$(MAKE_FOLDER) $(BINDIR)

clean:
	-@$(RM) $(BINDIR)
	-@$(RM) $(BUILDDIR)
