include $(top_srcdir)/Makerule.base

PROGNAME=www$(EXEEXT)
PROGNAMEBIG5=wwwbig5$(EXEEXT)
WWWLIBS=$(WWW_LIBS) -lcrypto
WWWCFLAGS = $(AM_CFLAGS) 
WWWLDFLAGS = $(AM_LDFLAGS)
#ifdef GPROF
#WWWLIBS	= -lghthash -lfcgi -lz
#WWWCFLAGS = $(AM_CFLAGS) -DGPROF -pg -g
#WWWLDFLAGS = $(AM_LDFLAGS) -static
#endif
#ifdef GPROF2
#WWWLIBS = -lghthash -lfcgi -lz 
#WWWCFLAGS = $(AM_CFLAGS) -DGPROF -pg -g
#WWWLDFLAGS = $(AM_LDFLAGS) 
#endif
.PHONEY: convertEncoding
convertEncoding:
	$(MAKE) -C $(top_srcdir)/local_utl convertEncoding
proto.h:
	cproto -DMAKE_PROTO -D__GNUC__ $(CFILE) $(OTHERS) $(CPPFLAGS) $(WWW_CPPFLAGS) -I$(BASEPATH)/libythtbbs -I$(BASEPATH)/ythtlib -I$(BASEPATH)/include > proto.h

BUILT_SOURCES = ythtlib convertEncoding

CFILE = BBSLIB.c bbsupdatelastpost.c boardrc.c deny_users.c bbsred.c \
	bbsmain.c bbstop10.c bbsdoc.c bbscon.c bbsboa.c index2009.c bbsall.c bbsanc.c \
	bbs0an.c bbslogout.c bbsleft.c bbslogin.c bbsbadlogins.c bbsqry.c \
	bbsnot.c bbsfind.c bbsfadd.c bbsfdel.c bbsfall.c bbsfriend.c \
	bbsfoot.c bbsform.c bbspwd.c bbsplan.c bbsinfo.c bbsmybrd.c bbssig.c \
	bbspst.c bbsgcon.c bbsgdoc.c bbsdel.c bbsdelmail.c bbsmailcon.c \
	bbsmail.c bbsmypic.c bbsdelmsg.c bbssnd.c bbsnotepad.c bbsmsg.c \
	bbssendmsg.c bbsreg.c bbsemailreg.c bbsmailmsg.c bbssndmail.c \
	bbsnewmail.c bbspstmail.c bbsgetmsg.c bbscloak.c bbsmdoc.c bbsnick.c \
	bbstfind.c bbsadl.c bbstcon.c bbsdoreg.c bbsmywww.c bbsccc.c \
	bbsclear.c bbsstat.c bbsedit.c bbsman.c bbsparm.c bbsfwd.c bbsmnote.c \
	bbsdenyall.c bbsdenydel.c bbsdenyadd.c bbstopb10.c bbsbfind.c bbsx.c \
	bbseva.c bbsvote.c bbsshownav.c bbsbkndoc.c bbsbknsel.c bbsbkncon.c \
	bbshome.c bbsindex.c bbssechand.c bbslform.c regreq.c \
	bbsselstyle.c bbscon1.c bbsattach.c bbskick.c bbsrss.c\
	bbsscanreg.c bbsshowfile.c bbsdt.c bbslt.c bbsincon.c bbssetscript.c \
	bbscccmail.c bbsfwdmail.c bbsscanreg_findsurname.c bbsnt.c \
	bbstopcon.c bbsdrawscore.c bbsmyclass.c bbssearchboard.c \
	bbslastip.c bbsucss.c bbsdefcss.c bbsself_photo_vote.c bbsspam.c \
	bbsspamcon.c bbssouke.c bbsboardlistscript.c bbsolympic.c bbsicon.c \
	bbsnewattach.c bbsupload.c bbspsttmpl.c bbspassport.c \
	bbslpassport.c bbsdlprepare.c bbsinviteconfirm.c \
	bbsemailconfirm.c bbsshowdigest.c bbsinvite.c bbstshirt.c bbsspecial.c\
	bbsseo.c bbsajaxtest.c bbsadv.c bbsfootball.c bbsbug.c bbstestssl.c bbsregcheck.c
if BLOG
CFILE+= blogblog.c blogread.c blogpost.c blogsend.c blogeditabstract.c \
	blogeditconfig.c blogeditsubject.c \
	blogedittag.c blogeditlink.c blogdraft.c blogdraftread.c \
	blogcomment.c blogsetup.c blogpage.c blogblock.c\
	blogrss2.c blogatom.c blogeditfriend.c blogmedia.c \
	blogdelete.c blogtemplate.c  blogpic.c
endif

CFILEBACKUP = bbsdt.c bbslt.c bbsmovie.c bbsusr.c bbst.c bbsalluser.c bbsufind.c
#OTHERS	= usesql.h BBSLIB.c boardrc.c deny_users.c bbsupdatelastpost.c bbsred.c tshirt.h

noinst_PROGRAMS = www wwwbig5 upload big5upload regimage 
www_SOURCES = $(CFILE)
www_CFLAGS =$(AM_CFLAGS) $(WWWCFLAGS) $(WWW_CPPFLAGS) -DHTMPATH='"$(HTMPATH)"'
www_LDADD = $(LDADD) $(WWWLIBS)
www_LDFLAGS = $(WWWLDFLAGS)
wwwbig5_SOURCES = $(CFILE)
wwwbig5_CFLAGS =$(AM_CFLAGS) $(WWWCFLAGS) $(WWW_CPPFLAGS) -DHTMPATH='"$(HTMPATH)"' -DUSEBIG5
wwwbig5_LDADD = $(LDADD) $(WWWLIBS)
upload_SOURCES = upload.c
big5upload_SOURCES = big5upload.c
regimage_SOURCES = regimage.c
regimage_LDADD = $(LDADD) -lgd
big5upload.c: upload.c
	cat upload.c | $(top_srcdir)/local_utl/convertEncoding s > big5upload.c

JSCSSHTML = $(wildcard html/*.css html/*.js)
BIG5JSCSSHTML := $(patsubst html/%,html/big5/%,$(JSCSSHTML))
$(BIG5JSCSSHTML): $(JSCSSHTML)
	if (! test -e html/big5); then mkdir html/big5; fi
	-for i in $(patsubst html/%,%,$(JSCSSHTML)); do cat html/$$i | $(top_srcdir)/local_utl/convertEncoding | sed -es/"gb2312"/"big5"/ > html/big5/$$i; done

install-exec-local: $(noinst_PROGRAMS) $(BIG5JSCSSHTML)
	echo $(JSCSSHTML)
	-mv $(CGIPATH)/$(PROGNAME) $(CGIPATH)/$(PROGNAME).old
	cp $(PROGNAME) $(CGIPATH)/$(PROGNAME)
	cd $(CGIPATH);ln $(PROGNAME) attach$(EXEEXT) -f
	-mv $(CGIPATH)/upload$(EXEEXT) $(CGIPATH)/upload.old
	cp upload$(EXEEXT) $(CGIPATH)/upload$(EXEEXT)
	-mv $(CGIPATH)/regimage$(EXEEXT) $(CGIPATH)/regimage.old
	cp regimage$(EXEEXT) $(CGIPATH)/regimage$(EXEEXT)
	-cp -u html/* $(HTMPATH)
#ifndef NOBIG5
	-mv $(CGIPATH)/$(PROGNAMEBIG5) $(CGIPATH)/$(PROGNAMEBIG5).old
	cp $(PROGNAMEBIG5) $(CGIPATH)/$(PROGNAMEBIG5)
	-mv $(CGIPATH)/big5upload$(EXEEXT) $(CGIPATH)/big5upload.old
	cp big5upload$(EXEEXT) $(CGIPATH)/big5upload$(EXEEXT)
	-mkdir $(HTMPATH)/big5
	-cp -u html/big5/* $(HTMPATH)/big5
#endif
	-mkdir $(HTMPATH)/images
	-cp -u html/images/* $(HTMPATH)/images
	-mkdir $(HTMPATH)/smilies
	-cp -u html/smilies/*.gif html/smilies/smilies.html $(HTMPATH)/smilies
	-mkdir $(HTMPATH)/jsMath
	-cp -u html/jsMath/*.gif html/jsMath/*.js $(HTMPATH)/jsMath

	-for i in $(PROGNAME) $(PROGNAMEBIG5) attach$(EXEEXT) upload$(EXEEXT) big5upload$(EXEEXT) regimage$(EXEEXT);do chown $(BBS_UID):$(BBS_GID) $(CGIPATH)/$$i; chmod +s $(CGIPATH)/$$i; done
	@echo "Finished!"
	-$(CGIPATH)/$(PROGNAME) &> /dev/null
#	killall -SIGHUP $(PROGNAME)
#	killall -SIGHUP $(PROGNAMEBIG5)
	@echo "Updated!"

