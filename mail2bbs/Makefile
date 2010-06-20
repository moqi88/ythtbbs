.PHONY:
BASEPATH = ..
include $(BASEPATH)/Makefile.Base

MAIL2BBS=mail2bbs$(EXEEXT)
PERLHELP=bbsmail bbsmail_wrap bbsmail_wrap2
INSTALLDIR=$(BBSHOME)/bin

all: mail2bbs$(EXEEXT) $(PERLHELP)

$(MAIL2BBS): mail2bbs.c qp_conv.c
	$(CC) -g $(CFLAGS) $(CPPFLAGS) $(OS_DEF) -o $(MAIL2BBS) $^ $(BBSLIB) $(LIBS) $(LIBSQL)

install_mail2bbs: $(MAIL2BBS)
	test -d $(INSTALLDIR) || mkdir -p $(INSTALLDIR)
	-install -g $(BBSGRP) -m 4750 -o $(BBSUID) $(MAIL2BBS) $(INSTALLDIR)

install: install_mail2bbs
	-install -g $(BBSGRP) -m  750 -o $(BBSUID) $(PERLHELP) $(INSTALLDIR)

bbsmail_wrap2: bbsmail_wrap2.in
	@echo "new bbsmail_wrap2.in,  please run ../configure"

bbsmail_wrap: bbsmail_wrap.in
	@echo "bbsmail_wrap.in newer than bbsmail_wrap,  please run ../configure"
