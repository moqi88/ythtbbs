BASEPATH = ..
include $(BASEPATH)/Makefile.Base
.PHONY:all install clean
bin:=bbsfindout$(EXEEXT) bbsinnd$(EXEEXT) bbsoutd$(EXEEXT)
inst=$(patsubst %,$(BBSHOME)/bin/%,$(bin))
all:bbsfindout$(EXEEXT) bbsinnd$(EXEEXT) bbsoutd$(EXEEXT)
bbsinnd$(EXEEXT): bbsinnd.c
	$(CC) $(CFLAGS) $(OS_DEF) -o $@ $^ $(BBSLIB) $(LIBS)
bbsoutd$(EXEEXT): bbsoutd.c
	$(CC) $(CFLAGS) $(OS_DEF) -o $@ $^ $(BBSLIB)
bbsfindout$(EXEEXT): bbsfindout.c
	$(CC) $(CFLAGS) $(OS_DEF) -o $@ $^ $(BBSLIB) $(LIBS)
install:$(inst)
clean:
	rm -fr $(bin)
