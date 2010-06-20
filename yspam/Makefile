.PHONY:
BASEPATH = ..
include $(BASEPATH)/Makefile.Base

INSTALLDIR=$(BBSHOME)/bin
YSPAM=bakmail yspamd

all: $(YSPAM)

bakmail: bakmail.c qp_conv.c
	$(CC) -g $(CFLAGSTEST) $(OS_DEF) $(CPPFLAGS) $(DEFINES) -o bakmail $^ $(LIBSQL) $(BBSLIB) $(LIBS)
	
yspamd: yspamd.c
	$(CC) -g $(CFLAGSTEST) $(OS_DEF) $(CPPFLAGS) $(DEFINES) -o yspamd yspamd.c $(LIBSQL) $(BBSLIB) $(LIBS)

qp_conv.c:../mail2bbs/qp_conv.c
	ln -s $^ $@
install: $(YSPAM)
	-install -g $(BBSGRP) -m  750 -o $(BBSUID) $(YSPAM) $(INSTALLDIR)
clean:
	rm -f $(YSPAM)
