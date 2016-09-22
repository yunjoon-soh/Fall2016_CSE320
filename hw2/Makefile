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
EXTRA_FLAG=

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
debug : CFLAGS += -g

all: directories $(BIN)

debug : clean all

$(BIN) : $(OBJECTS) $(HDRS)
	$(CC) $(CFLAGS) $(EXTRA_FLAG) $(OBJECTS) -I$(INCLUDEDIR) -o $(BINDIR)/$@

$(BUILDDIR)/%.o : $(SRCDIR)/%.c $(HDRS)
	$(CC) $(CFLAGS) $(EXTRA_FLAG) -c $< -I$(INCLUDEDIR) -o $@

directories:
	@$(MAKE_FOLDER) $(BUILDDIR)
	@$(MAKE_FOLDER) $(BINDIR)

clean:
	-@$(RM) $(BINDIR)
	-@$(RM) $(BUILDDIR)
