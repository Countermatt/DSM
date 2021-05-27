ALL: default

CC           = gcc
CLINKER      = $(CC)
OPTFLAGS     = -O0


SHELL = /bin/sh

CFLAGS  =   -DREENTRANT -Wunused -Wall -g
CCFLAGS = $(CFLAGS)
LIBS =  -lpthread

EXECS =  init dsmexec dsmwrap truc

default: $(EXECS)

dsmexec: dsmexec.o common_dsmexec.o
	$(CLINKER) $(OPTFLAGS) -o dsmexec dsmexec.o common_dsmexec.o $(LIBS)
	mv dsmexec bin
	mv *o .obj

dsmwrap: dsmwrap.o common_dsmwrap.o
	$(CLINKER) $(OPTFLAGS) -o dsmwrap dsmwrap.o common_dsmwrap.o $(LIBS)
	mv dsmwrap bin
	mv *o .obj

truc: truc.o
	$(CLINKER) $(OPTFLAGS) -o truc truc.o $(LIBS)
	mv truc bin
	mv *o .obj

clean:
	@-/bin/rm -f *.o *~ PI* $(EXECS) *.out core
	rm -rf bin
	rm -rf .obj

.c:
	$(CC) $(CFLAGS) -o $* $< $(LIBS)
.c.o:
	$(CC) $(CFLAGS) -c $<
.o:
	${CLINKER} $(OPTFLAGS) -o $* $*.o $(LIBS)

init: clean
		mkdir bin
		mkdir .obj
