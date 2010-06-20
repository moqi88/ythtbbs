include $(top_srcdir)/Makerule.base
BUILT_SOURCES = ythtlib
noinst_PROGRAMS =  mergeb averun showuser refriend2 autoundeny autoclear \
	auto_rm_junk userip tianqi nbstat expire_2nd bbsstatlog bbsstatproclog \
	clear_junk clear_attach searchLastMark sortdir printSecLastMark \
	searchDIR auto_rm_fakedir bbslogd MakeIndex3 bm bm2 filter \
	find_lost_mail find_rm_lost fixdir id_boards id_distr logfreq near \
	ptyexec reminder save_brc searchauther selpersonal showwhoperm \
	sumcgitime tuijian upersonal cpersonal transuu2bin bbspop3d bbsevad \
	combine_arc repsync upnew newtop10 newboards printSecLastUpdate \
	autoBanBoards brcBoards boardRelate undeletedir watchman autounprison \
	bm7day shminit checkshm auto_kickout auto_deluser new_regkey \
	relinkdigest bbstop convertEncoding fixmailsize autocross checkonline \
	euro2004 autoexecutioner jingcai listboardfile printSecLastDigest \
	basketball moredir init_key scanmark get_online_data_from_bbslists \
	get_cmdline_from_core diskfree \
	mail_jingcai aid weather statIP friends google_sm_maker tv \
	showspecialboards transythttodiscuzx 
if BLOG
noinst_PROGRAMS += bloglogd blogstat htmlfilter
endif

nbstat_SOURCES = nbstat.c bmstat.c amstat.c boardstat2.c boardscore.c bbslists.c sysrecord.c 
nbstat_LDADD = $(LDADD) -lm
diskfree_SOURCES = diskfree.c sysrecord.c
mergeb_SOURCES = mergeb.c record.c
expire_2nd_SOURCES = 2nd_expire.c record.c
#chboard_SOURCES = chboard.c record.c
bbslogd_LDADD = $(LDADD) -lghthash
if BLOG
bloglogd_LDADD = $(LDADD) -lghthash
blogstat_LDADD = $(LDADD) -lghthash -ljson
endif
friends_LDADD = $(LDADD) -lghthash
ptyexec_LDADD = $(LDADD) $(UTIL)
autoundeny_SOURCES = autoundeny.c sysrecord.c
autoclear_SOURCES =  autoclear.c sysrecord.c
bbsevad_LDADD = $(LDADD) $(LIBSQL)
jingcai_LDADD = $(LDADD) $(LIBSQL)
newtop10_LDADD = $(LDADD) -lghthash
autounprison_SOURCES = autounprison.c sysrecord.c
selpersonal_LDADD = $(LDADD) -lm
init_key_LDADD = $(LDADD) -lcrypto
autoexecutioner_SOURCES = autoexecutioner.c sysrecord.c

SHELL_UTILITY := daily.sh bbsheavywork.sh MakeIndex3.sh
#UTILITY := $(UTILITY:=$(EXEEXT))
#usesql.h: $(BBSHOME)/etc/sysconf.ini
#	echo "#include <mysql/mysql.h>" > usesql.h
#	cat $(BBSHOME)/etc/sysconf.ini|grep SQL|sed -e 's/^/#define /;s/=//g' >> usesql.h

.PHONY: install-exec-local
install-exec-local: $(noinst_PROGRAMS) $(SHELL_UTILITY)
	$(INSTALL) -m 550 -g $(BBS_GID) -o $(BBS_UID) $(noinst_PROGRAMS) $(BBS_HOME)/bin
	$(INSTALL) -m 550 -g $(BBS_GID) -o $(BBS_UID) $(SHELL_UTILITY) $(BBS_HOME)/bin

noinst_HEADERS = amstat.h bbslists.h bbsstatlog.h bmstat.h boardscore.h \
	boardstat2.h nbstat.h record.h sysrecord.h www.h
