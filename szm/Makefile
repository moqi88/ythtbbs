CC=gcc
CFLAGS = -g -Wall -D_GNU_SOURCE
LDFLAGS = -lgd -lpng -ljpeg

all:szmd szm.o

szm.c:../ythtlib/szm.c
	ln -s $^ $@
szm.h:../ythtlib/szm.h
	ln -s $^ $@
szmd: szmd.c szmd.h szm.h bmplib.c bmplib.h
	$(CC) $(CFLAGS) $(LDFLAGS) szmd.c bmplib.c -o szmd
test: testclient.c szm.c
	$(CC) -g -Wall -o $@ $^
bm: bm.c bmplib.c
	$(CC) -g -Wall -o $@ $^

clean:
	rm -f *.o *~ szmd
