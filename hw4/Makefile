CC := gcc
SRCD := src
BLDD := build
BIND := bin
INCD := include

_SRCF := $(shell find $(SRCD) -type f -name *.c)
_OBJF := $(patsubst $(SRCD)/%,$(BLDD)/%,$(_SRCF:.c=.o))
INC := -I $(INCD)

EXEC := sfish

CFLAGS := -Wall -Werror
DFLAGS := -g -DDEBUG
LIBS := readline

.PHONY: clean all

all: exec

debug: CFLAGS += -g -DDEBUG
debug: exec

exec: folder $(EXEC)	

folder: 
	mkdir -p bin build

$(EXEC): $(_OBJF)
	$(CC) $^ -o $(BIND)/$@ -l $(LIBS)

$(BLDD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	$(RM) -r $(BLDD) $(BIND)
	$(RM) $(TESTD)/*.o # I added

# I added
TESTD= test
test: folder debug $(BIND)/sfunit

bin/sfunit: $(TESTD)/sfunit.o $(BLDD)/job_arraylist.o $(BLDD)/sfbuiltin.o $(BLDD)/sfish_helper.o 
	$(CC) $(CFLAGS) $(DFLAGS) $^ $(INC) -lcriterion -lreadline -o $@

$(TESTD)/%.o: $(TESTD)/%.c
	$(CC) $(CFLAGS) $(DFLAGS) $(INC) -c -o $@ $<

