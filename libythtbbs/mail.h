/* mail.c */
#ifndef __MAIL_H
#define __MAIL_H
#define BASE_MAIL_HOLD (1000)
#define MAX_SYSOPMAIL_HOLD (5000)
int update_mailsize_up(struct fileheader *, char *userid);
int update_mailsize_down(struct fileheader *, char *userid);
int max_mailsize(struct userec *lookupuser);
int DIR_do_editmail(struct fileheader *fileinfo, struct fileheader *newfileinfo, char *userid);
int get_mailsize(struct userec *lookupuser);
int mail_buf(char *buf, int size, char *userid, char *title, char *sender); //this api check mailsize
int mail_file(char *filename, char *userid, char *title, char *sender); //this api check mailsize
int system_mail_buf(char *buf, int size, char *userid, char *title, char *sender); //this api don't check mailsize
int system_mail_file(char *filename, char *userid, char *title, char *sender); //this api don't check mailsize
int system_mail_link(char *filename, char *userid, char *title, char *sender); //don't check mailsize. symbolic link
int calc_mailsize(struct userec *user, int needlock);
char * check_mailperm(struct userec *lookupuser);
#ifdef INTERNET_EMAIL
int bbs_sendmail(char *fname, char *title, char *receiver, char *sender, int filter_ansi);
int bbs_sendmail_noansi(char *fname, char *title, char *receiver, char *sender);
#endif
int invalidaddr(char *addr);
#endif
