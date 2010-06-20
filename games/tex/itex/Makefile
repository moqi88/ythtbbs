LEX = flex
YACC = yacc
PROGRAMS = itex2MML math2mathplayer
CC = gcc

.c.o:   ;   $(CC) -c $*.c

all: $(PROGRAMS)

itex2MML: y.tab.o itex2MML.o
	$(CC) -o itex2MML y.tab.o itex2MML.o

lex.yy.o: lex.yy.c y.tab.h


y.tab.c y.tab.h: itex2MML.y
	$(YACC) -d -v itex2MML.y

itex2MML.c: itex2MML.l
	$(LEX) -oitex2MML.c itex2MML.l

math2mathplayer.c: math2mathplayer.l
	$(LEX) -omath2mathplayer.c math2mathplayer.l

clean:
	rm -f *.o math2mathplayer.c y.tab.c y.tab.h itex2MML.c $(PROGRAMS)
