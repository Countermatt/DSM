CC=gcc
CFLAGS=-Wall -I./Include -lpthread

BINDIR=./bin
SRCDIR=./src
OBJDIR=./.obj
SRC=$(wildcard $(SRCDIR)/*.c)
OBJ=$(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

EXEC=exemple

default: $(EXEC)

all: $(EXEC)

$(EXEC):	$(OBJ) $(BINDIR)
	$(CC) $(OBJ) -o $(BINDIR)/$@ $(CFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)
		$(CC) -o $@ -c $< $(CFLAGS)

$(OBJDIR):
	mkdir $(OBJDIR)

$(BINDIR):
	mkdir $(BINDIR)

clean:
	-rm -rf $(BINDIR)
	-rm -rf $(OBJDIR)
