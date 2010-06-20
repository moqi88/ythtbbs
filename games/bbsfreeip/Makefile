#ylsdd 2002/11/19
BASEPATH = ../..
include $(BASEPATH)/Makefile.Base
PROGNAME = cgifreeip freeip qqip sortip
.SUFFIXES: .o .c

.c.o:   ;   gcc -g -c $*.c $(CFLAGS)

all: $(PROGNAME)

ifeq (.depend,$(wildcard .depend))
include .depend
endif

depend dep:
	gcc $(CFLAGS) -M *.c >.depend
qqip:qqip.o
	$(CC) -o $@ $^
displayip.o:ipnums.h
	$(CC) $(CFLAGS) -c -o $@ displayip.c 	
cgifreeip: cgifreeip.o displayip.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)
freeip: freeip.o displayip.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)
sortip:sortip.o
	$(CC) $(CFLAGS) -o $@ $^ 
QQWry.Dat:
	wget -O qqwry.rar http://96hk.y365.com/soft/qqwry.rar
	unrar x -o+ qqwry.rar QQWry.Dat
getip:qqip QQWry.Dat
	./qqip
	rm qqwry.rar QQWry.Dat -f	
ipnums.h:sortip 
	./sortip > ipnums.h
install:ip_arrange_sort.txt cgifreeip freeip
	cp freeip $(BBSHOME)/bin
	cp ip_arrange_sort.txt $(BBSHOME)/etc
	cp cgifreeip $(CGIPATH)
clean:
	rm ip_arr*.txt *.o
	rm $(PROGNAME)
