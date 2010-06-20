# Makefile for bbsinnd

#  The source code directory for BBS (include bbs.h, ...)

BASEPATH = ../..
include $(BASEPATH)/Makefile.Base

OBJ  = bbspost.o
all:	bbspost$(EXEEXT)

COBJS   :=$(CFILE:.c=.o)
.c.o:   ;  $(CC) -c $*.c -D_GNU_SOURCE $(CFLAGS) $(OS_DEF) -Wall

bbspost$(EXEEXT): ${OBJ}
	${CC} ${CFLAGS} -o bbspost ${OBJ} ${BBSLIB} $(LIBS)
install:
	cp mailpost /home/bbs/innd
	cp expirepost /home/bbs/innd
	cp bbspost$(EXEEXT) /home/bbs/innd

clean:
	${RM} *.o bbspost$(EXEEXT)

