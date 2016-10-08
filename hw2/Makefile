# Folders
SRCDIR=src
BUILDDIR=build
BINDIR=bin
INCLUDEDIR=include

# Names
BIN = utf

# Compiler related
CC = gcc
CFLAGS = -Wall -Werror -pedantic -Wextra

# Makefile related
#SRCS = $(wildcard $(SRCDIR)/*.c)
SRCS= $(SRCDIR)/utfconverter.c $(SRCDIR)/filemanager.c
#OBJECTS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))
OBJECTS = $(BUILDDIR)/utfconverter.o $(BUILDDIR)/filemanager.o
#HDRS = $(wildcard $(INCLUDEDIR)/*.h)
HDRS = $(INCLUDEDIR)/main.h $(INCLUDEDIR)/utfconverter.h $(INCLUDEDIR)/filemanager.h

# Commands Shortcut
MAKE_FOLDER = mkdir -p
RM = rm -rf

# To support debug compilation
all: directories $(BIN)

debug: directories $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -g -c -I$(INCLUDEDIR) $(SRCDIR)/utfconverter.c -o $(BUILDDIR)/utfconverter.o
	$(CC) $(CFLAGS) -g -c -I$(INCLUDEDIR) $(SRCDIR)/filemanager.c -o $(BUILDDIR)/filemanager.o
	$(CC) $(CFLAGS) -g $(BUILDDIR)/utfconverter.o $(BUILDDIR)/filemanager.o -I$(INCLUDEDIR) -o $(BINDIR)/$(BIN)

$(BIN) : $(OBJECTS) $(HDRS)
	$(CC) $(CFLAGS) $(OBJECTS) -I$(INCLUDEDIR) -o $(BINDIR)/$@

$(BUILDDIR)/%.o : $(SRCDIR)/%.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -I$(INCLUDEDIR) -o $@

directories:
	@$(MAKE_FOLDER) $(BUILDDIR)
	@$(MAKE_FOLDER) $(BINDIR)

clean:
	-@$(RM) $(BINDIR)
	-@$(RM) $(BUILDDIR)
