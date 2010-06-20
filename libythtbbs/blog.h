#ifndef __BLOG_H
#define __BLOG_H

#define MAXTAG 5
#define BLOG2BOARD 1

struct BlogSubject {
	int count;
	unsigned int cate;
	unsigned char hide:2, nouse1:6;
	char title[31];
	char nouse2[24];
};

struct BlogTag {
	int count;
	int nouse0;
	unsigned char nouse1;
	char title[31];
	char nouse2[24];
};

struct BlogLink {
	char title[61];
	unsigned char hide:2, noues1:6;
	char nouse2[2];
	char url[121], noues3[3];
	int count;
	char intro[64];
};

struct BlogHeader {
	time_t fileTime;
	time_t modifyTime;
	short nComment;
	short subject;//
	short tag[MAXTAG];
	unsigned char hasAbstract:1, hide:2, blocked:1;
	unsigned char hasAttach:1;
	unsigned char nouse1:3;
	char nouse2;//
	char title[81], poster[IDLEN+1];
	short nRead;//
	int nouse5[2];
};

struct BlogCommentHeader {
	time_t commentTime;
	time_t thread;
	unsigned char blocked: 1, nouse2: 7;
	char nouse[7];
};

struct BlogConfig {
	time_t createTime;
	time_t postTime, lwTime, commentTime;
	int nouse1;
	int bonus;
	char useridUTF8[31];
	char useridEN[37];
	//
	char title[61];
	char defaultSubject;
	unsigned char hl:2, lw:2, model:4;
	unsigned char cl:2, relatedboards:1, tmp:5;
	//
	unsigned int category;
	unsigned int visit;
	char media[10][6];
	time_t top;
	char nouse3[796];
};

struct blogCategory {
	unsigned int category;
	char *desc;
};

struct Blog {
	char userid[IDLEN+1];
	struct mmapfile configFile;
	struct mmapfile indexFile;
	struct mmapfile draftFile;
	struct mmapfile subjectFile;
	struct mmapfile tagFile;
	struct mmapfile linkFile;
	struct mmapfile lwFile;
	struct mmapfile bitFile;
	struct BlogConfig *config;
	struct BlogSubject *subject;
	struct BlogHeader *index;
	struct BlogHeader *draft;
	struct BlogHeader *lw;
	struct BlogTag *tag;
	struct BlogLink *link;
	int nSubject, nIndex, nDraft, nTag, nLink, nLw;
	int lockfd;
	char wantWrite, wantDraft, amode, cgi;
	int nouse[21];
};

#define BLOG_DIR_PATH	MY_BBS_HOME "/blog"

extern const struct blogCategory blogc[];

void setBlogFile(char *buf, char *userid, char *file);
void setBlogPost(char *buf, char *userid, time_t fileTime);
void setBlogAbstract(char *buf, char *userid, time_t fileTime);
void setBlogCommentIndex(char *buf, char *userid, time_t fileTime);
void setBlogComment(char *buf, char *userid, time_t fileTime, time_t commentTime);
int createBlog(char *userid);
int openBlog(struct Blog *blog, char *userid);
int openBlogW(struct Blog *blog, char *userid);
int openBlogD(struct Blog *blog, char *userid);
void closeBlog(struct Blog *blog);
void reopenBlog(struct Blog *blog);
int blogPost(struct Blog *blog, char *poster, char *title, char *tmpfile, 
	int subject, int tags[], int nTag, int mode, int hide, int attach);
int blogSaveAbstract(struct Blog *blog, time_t fileTime, char *abstract);
int blogUpdatePost(struct Blog *blog, time_t fileTime, char *title, char *tmpfile, int subject, int tags[], int nTag);
int blogPostDraft(struct Blog *blog, char *title, char *tmpfile, int draftID);
int deleteDraft(struct Blog *blog, int draftID);
int blogComment(struct Blog *blog, time_t fileTime, char *tmpfile);
int blogCheckMonth(struct Blog *blog, char buf[32], int year, int month);
int blogModifySubject(struct Blog *blog, int subjectID, char *title, int hide);
int blogModifyTag(struct Blog *blog, int tagID, char *title);
int findBlogArticle(struct Blog *blog, time_t fileTime);
int canReadBlogItem(unsigned int hlevel, char *readid, char *blogid);
void blogLog(char *fmt, ...);
#endif //__BLOG_H
