BASEPATH = ../..
include $(BASEPATH)/Makefile.Base

PROGNAME = tetris
.SUFFIXES: .o .c

.c.o:   ;   gcc -g -c $*.c $(CFLAGS)

all: $(PROGNAME)

ifeq (.depend,$(wildcard .depend))
include .depend
endif

depend dep:
	gcc $(CFLAGS) -M *.c >.depend
tetris: tetris.o
	$(CC) $(CFLAGS) -o $@ $^
clean:
	rm *.o $(PROGNAME)

