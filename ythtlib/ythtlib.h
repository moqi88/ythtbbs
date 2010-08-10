#ifndef __YTHTLIB_H
#define __YTHTLIB_H
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <ctype.h>

#ifndef sizeof
#define sizeof(x) ((int)sizeof(x))
#endif

#ifndef ATTACHCACHE
#define ATTACHCACHE "/home/bbsattach/cache"
#endif

#ifndef ERRLOG
#define ERRLOG	MY_BBS_HOME "/deverrlog"
#endif

// define log file for writesyslog
#define ERRORLOG 0
#define ADMINLOG 1
#define TESTLOG 2
#define TESTLOGLOG 3

struct hword {
	char str[80];
	void *value;
	struct hword *next;
};

typedef struct hword *diction[26 * 26];

int getdic(diction dic, size_t size, void **mem);
struct hword *finddic(diction dic, char *key);
struct hword *insertdic(diction dic, struct hword *cell);
int mystrtok(char *str, int delim, char *result[], int max);
struct stat *f_stat(const char *file);
struct stat *l_stat(const char *file);
int uudecode(FILE * fp, char *outname);
int fakedecode(FILE * fp);
char *attachdecode(FILE * fp, char *articlename, char *filename);
void uuencode(FILE * fr, FILE * fw, int len, char *filename);
void _errlog(char *fmt, ...)
    __attribute__ ((format(printf, 1, 2)));
#define errlog(format, args...) _errlog(__FILE__ ":%s line %d " format, __FUNCTION__,__LINE__ , ##args)
void writelog(char* logfilename, char* log);
void writesyslog(int logtype, char* log);

#define file_size(x) ({f_stat(x)->st_size;})
#define file_time(x) ({f_stat(x)->st_mtime;})
#define file_rtime(x) ({f_stat(x)->st_atime;})
#define file_exist(x) (access(x, F_OK)==0)
#define file_isdir(x) (S_ISDIR(f_stat(x)->st_mode))
#define file_isfile(x) (S_ISREG(f_stat(x)->st_mode))
#define lfile_isdir(x) (S_ISDIR(l_stat(x)->st_mode))
#define lfile_islnk(x) (S_ISLNK(l_stat(x)->st_mode))

unsigned char numbyte(int n);
int bytenum(unsigned char c);

int myatoi(unsigned char *buf, int len);
void myitoa(int a, unsigned char *buf, int len);
void normalize(char *buf);
void strsncpy(char *s1, const char *s2, int n);
char *strltrim(char *s);
char *strrtrim(char *s);
#define strtrim(s) strltrim(strrtrim(s))
void filteransi(char *line);
char *utf8cut(char *str, int maxsize);
char *gb2utf8(char *to, int tolen, char *from);
char *utf82gb(char *to0, int tolen0, char *from0);
int code_convert(const char *from_charset,const char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen);
int ascii2hex(char* ascii, char* hex );
int int2ascii(int num, char* ascii);

void *try_get_shm(int key, int size, int flag);
void *get_shm(int key, int size);
#define get_old_shm(x,y) try_get_shm(x,y,0)
//Copy from Linux 2.4 kernel...
#define min(x,y) ({ \
	const typeof(x) _x = (x);	\
	const typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x < _y ? _x : _y; })

#define max(x,y) ({ \
	const typeof(x) _x = (x);	\
	const typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x > _y ? _x : _y; })
//end
#include "fileop.h"
#include "named_socket.h"
#include "net_socket.h"
#include "crypt.h"
#include "limitcpu.h"
#include "timeop.h"
#include "md5.h"
#include "compatible.h"
#include "convcode.h"
#include "strlib.h"
#include "htmlfunc.h" //先放这里面，也行应该独立出去，因为有些static的字符串
#include "szm.h"
#include "setproctitle.h"
#include "base64.h"
#include "mime.h"
#include "cuser.h"
#define BAD_WORD_NOTICE "您的文章可能含有按照国家有关规定" \
                       "不适宜在公共场合发表的内容\n本文须" \
                       "经过有关人士审查后发表"
#define DO1984_NOTICE  "按照有关部门要求,本版文章必须经过有关" \
                       "人士审查后发表,请耐心等候!"

int reload_badwords(char *wordlistf, char *imgf);
int filter_file(char *checkfile, struct mmapfile *badword_img);
int filter_string(char *string, struct mmapfile *badword_img);
int filter_article(char *title, char *filename, struct mmapfile *badword_img);
void delete_redundant_word( char *str, char const *sub_str);

#endif
