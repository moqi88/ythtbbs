CC=gcc
CFLAGS = -g -Wall -D_GNU_SOURCE
LDFLAGS = -L/home/bbs/bin -lytht

all:cuserd

cuser.c:../ythtlib/cuser.c
	ln -s $^ $@
cuser.h:../ythtlib/cuser.h
	ln -s $^ $@
cuserd: cuserd.c cuserd.h cuser.h
	$(CC) -I../include -I../ythtlib -I../libythtbbs $(CFLAGS)  cuserd.c -o cuserd $(LDFLAGS)
test: testclient.c cuser.c
	$(CC) -I../include -I../ythtlib -I../libythtbbs -g -Wall -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *~ cuserd
