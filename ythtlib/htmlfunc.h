/* html.c */
#ifndef __HTMLFUNC_H
#define __HTMLFUNC_H
char *scriptstr(const unsigned char *s);
char *void1(unsigned char *s);
char *urlencode(char *str);
char *hex_encode(unsigned char *, int, char *);
char *hex_decode(char *, unsigned char *);
#endif
