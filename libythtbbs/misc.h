/* misc.c */
#ifndef __MISC_H
#define __MISC_H
void getrandomint(unsigned int *s);
void getrandomstr(unsigned char *s);
void getrandomval(unsigned char *s, size_t size);
struct mymsgbuf {
	long int mtype;
	char mtext[1];
};
void newtrace(char *s);
int initmsq(key_t key);
void tracelog(char *fmt, ...) __attribute__((format(printf, 1, 2)));
void ailog(char *str);
void friendslog(char *str);
int deltree(const char *dst);
#endif
