BASEPATH = ../..
include $(BASEPATH)/Makefile.Base

BIN:= an anall
.PHONY:all install clean
all:$(BIN)
an: an.c
	$(CC) -o $@ $(CFLAGS) $< $(BBSLIB)
anw.exe: an_win.c
	i586-mingw32msvc-gcc -o $@ $(CFLAGS) $< -lwsock32
	i586-mingw32msvc-strip $@

an_print_no_hide: an_print_no_hide.c
	$(CC) -o $@ $(CFLAGS) $< $(BBSLIB)

anall:anall.c
	$(CC) -o $@ $< $(CFLAGS)

all: $(BIN)
#install: all
#	cp $(BIN) /home/bbs/bin
clean:
	rm -f $(BIN)
