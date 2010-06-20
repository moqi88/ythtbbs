include $(top_srcdir)/Makerule.base

CFILE = bbs.c record.c delete.c main.c stuff.c mail.c register.c \
           xyz.c talk.c read.c comm_lists.c namecomplete.c chat.c \
           maintain.c edit.c more.c help.c bcache.c boards.c \
           smth_screen.c io.c term.c userinfo.c vote.c announce.c sendmsg.c\
	   bm.c list.c goodbye.c bbsgopher.c fileshm.c postheader.c \
	   convcode.c tmachine.c addressbook.c\
	   backnumber.c common.c sec_hand.c regular.c 1984.c\
	   editboard.c power_select.c friendtest.c\
	   posttmpl.c killer.c select.c\
	   mc/mc.c mc/bank.c mc/macao.c mc/stock.c mc/shop.c\
	   mc/beggar.c mc/gang.c mc/police.c mc/task.c mc/exp.c mc/map.c

proto.h:
	echo '#include "bbs.h"' > proto.h
	echo '#include "bbstelnet.h"' >> proto.h
	cproto $(CFILE) $(DEFS) -DBACK_DELETE_RANG -I $(top_srcdir)/include -I $(top_srcdir)/ythtlib -I $(top_srcdir)/libythtbbs >> proto.h
BUILT_SOURCES = ythtlib proto.h

noinst_PROGRAMS = bbs bbstest bbs.chatd bbsd telnet bbsnet
bbs_SOURCES = $(CFILE) bbstelnet.h mc/mc.h
bbs_CFLAGS = $(AM_CFLAGS)  -DBACK_DELETE_RANGE -include proto.h
bbs_LDADD = $(LDADD) $(LIBS) -lm
bbstest_SOURCES = $(CFILE) bbstelnet.h
bbstest_CFLAGS = $(CFLAGSTEST) -DBACK_DELETE_RANGE -include proto.h
bbstest_LDADD = $(LDADD) $(LIBS) -lm

telnet_SOURCES = telnet0.c
bbs_chatd_SOURCES = station.c

install-exec-local: $(noinst_PROGRAMS)
	$(INSTALL) -m 550 -g $(BBS_GID) -o $(BBS_UID) $(noinst_PROGRAMS) $(BBS_HOME)/bin

