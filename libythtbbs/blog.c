#include <time.h>
#include <sys/msg.h>
#include "ythtbbs.h"


const struct blogCategory blogc[] = {
	{0x00000001, "日记"}, {0x00000002, "随笔"}, 
	{0x00000004, "连载"}, {0x00000008, "幽默"},
	{0x00000010, "星座"}, {0x00000020, "旅游"}, 
	{0x00000040, "购物"}, {0x00000080, "时尚"},
	{0x00000100, "美食"}, {0x00000200, "情感"}, 
	{0x00000400, "同性"}, {0x00000800, "健康"},
	{0x00001000, "居家"}, {0x00002000, "评论"}, 
	{0x00004000, "体育"}, {0x00008000, "游戏"},
	{0x00010000, "人物"}, {0x00020000, "娱乐"}, 
	{0x00040000, "时事"}, {0x00080000, "关注"},
	{0x00100000, "求学"}, {0x00200000, "职场"}, 
	{0x00400000, "音乐"}, {0x00800000, "动漫"},
	{0x01000000, "影视"}, {0x02000000, "财经"}, 
	{0x04000000, "科技"}, {0x08000000, "军事"},
	{0x10000000, "读书"}, {0x20000000, "相册"}, 
	{0x40000000, "数码"}, {0x80000000, NULL}
};

void
setBlogFile(char *buf, char *userid, char *file)
{
	sprintf(buf, MY_BBS_HOME "/blog/%c/%s/%s", mytoupper(userid[0]),
		userid, file);
}

void
setBlogPost(char *buf, char *userid, time_t fileTime)
{
	sprintf(buf, MY_BBS_HOME "/blog/%c/%s/%d", mytoupper(userid[0]),
		userid, (int) fileTime);
}

void
setBlogAbstract(char *buf, char *userid, time_t fileTime)
{
	sprintf(buf, MY_BBS_HOME "/blog/%c/%s/%d.A", mytoupper(userid[0]),
		userid, (int) fileTime);
}

void
setBlogCommentIndex(char *buf, char *userid, time_t fileTime)
{
	sprintf(buf, MY_BBS_HOME "/blog/%c/%s/%d.C", mytoupper(userid[0]),
		userid, (int) fileTime);
}

void
setBlogComment(char *buf, char *userid, time_t fileTime, time_t commentTime)
{
	sprintf(buf, MY_BBS_HOME "/blog/%c/%s/%d.%d", mytoupper(userid[0]),
		userid, (int) fileTime, (int) commentTime);
}

int
createBlog(char *userid)
{
	int fd;
	char buf[256];
	struct BlogConfig blogConfig;
	struct BlogSubject blogSubject = { count: 0, hide: 0, title:"默认栏目"};
	struct BlogTag blogTag = { count: 0, title:"默认Tag"};

	setBlogFile(buf, userid, "");
	mkdir(buf, 0770);

	bzero(&blogConfig, sizeof (blogConfig));
	blogConfig.createTime = time(NULL);
	strsncpy(blogConfig.title, "尚未确定Blog标题",
		 sizeof (blogConfig.title));
	gb2utf8(blogConfig.useridUTF8, sizeof (blogConfig.useridUTF8), userid);
	strsncpy(blogConfig.useridEN, urlencode(userid),
		 sizeof (blogConfig.useridEN));

	setBlogFile(buf, userid, "config");
	fd = open(buf, O_CREAT | O_WRONLY | O_EXCL, 0660);
	if (fd >= 0) {
		write(fd, &blogConfig, sizeof (blogConfig));
		close(fd);
	}

	setBlogFile(buf, userid, "index");
	close(open(buf, O_CREAT | O_WRONLY | O_EXCL, 0660));
	setBlogFile(buf, userid, "draft");
	close(open(buf, O_CREAT | O_WRONLY | O_EXCL, 0660));
	setBlogFile(buf, userid, "link");
	close(open(buf, O_CREAT | O_WRONLY | O_EXCL, 0660));
	setBlogFile(buf, userid, "tag");
	fd = (open(buf, O_CREAT | O_WRONLY | O_EXCL, 0660));
	if(fd >= 0) {
		write(fd, &blogTag, sizeof (blogTag));
		close(fd);
	}
	setBlogFile(buf, userid, "subject");
	fd = open(buf, O_CREAT | O_WRONLY | O_EXCL, 0660);
	if (fd >= 0) {
		write(fd, &blogSubject, sizeof (blogSubject));
		close(fd);
	}
	setBlogFile(buf, userid, "leaveword");
	close(open(buf, O_CREAT | O_WRONLY | O_EXCL, 0660));
	setBlogFile(buf, userid, "bit");
	close(open(buf, O_CREAT | O_WRONLY | O_EXCL, 0660));
	return 0;
}

static int
_openBlog(struct Blog *blog, char *userid, int wantWrite, int wantDraft)
{
	char buf[256];
	int (*mmapfunc) (char *, struct mmapfile *);
	int lockop;
	if (wantWrite) {
		mmapfunc = mmapfilew;
		lockop = LOCK_EX;
	} else {
		mmapfunc = mmapfile;
		lockop = LOCK_SH;
	}
	bzero(blog, sizeof (*blog));
	strsncpy(blog->userid, userid, sizeof (blog->userid));

	setBlogFile(buf, userid, "config");
#ifdef BLOG_SAFE_MODE
	blog->lockfd = open(buf, O_RDONLY, 0660);
	if (blog->lockfd < 0)
		return -1;
	if(flock(blog->lockfd, lockop) < 0) {
		close(blog->lockfd);
		return -1;
	}
#endif
	mmapfunc(buf, &blog->configFile);
	if (blog->configFile.size < sizeof (struct BlogConfig)) {
		mmapfile(NULL, &blog->configFile);
#ifdef BLOG_SAFE_MODE
		flock(blog->lockfd, LOCK_UN);
		close(blog->lockfd);
#endif
		return -1;
	}
	blog->config = (struct BlogConfig *) blog->configFile.ptr;

	setBlogFile(buf, userid, "index");
	mmapfunc(buf, &blog->indexFile);
	blog->index = (struct BlogHeader *) blog->indexFile.ptr;
	blog->nIndex = blog->indexFile.size / sizeof (struct BlogHeader);

	setBlogFile(buf, userid, "leaveword");
	mmapfunc(buf, &blog->lwFile);
	blog->lw = (struct BlogHeader *) blog->lwFile.ptr;
	blog->nLw = blog->lwFile.size / sizeof (struct BlogHeader);

	if (wantDraft) {
		setBlogFile(buf, userid, "draft");
		mmapfile(buf, &blog->draftFile);
		blog->draft = (struct BlogHeader *) blog->draftFile.ptr;
		blog->nDraft = blog->draftFile.size / sizeof (struct BlogHeader);
	}

	setBlogFile(buf, userid, "subject");
	mmapfunc(buf, &blog->subjectFile);
	blog->subject = (struct BlogSubject *) blog->subjectFile.ptr;
	blog->nSubject = blog->subjectFile.size / sizeof (struct BlogSubject);

	setBlogFile(buf, userid, "tag");
	mmapfunc(buf, &blog->tagFile);
	blog->tag = (struct BlogTag *) blog->tagFile.ptr;
	blog->nTag = blog->tagFile.size / sizeof (struct BlogTag);

	setBlogFile(buf, userid, "link");
	mmapfunc(buf, &blog->linkFile);
	blog->link = (struct BlogLink *) blog->linkFile.ptr;
	blog->nLink = blog->linkFile.size / sizeof (struct BlogLink);

	setBlogFile(buf, userid, "bit");
	mmapfunc(buf, &blog->bitFile);
	blog->wantWrite = wantWrite;
	return 0;
}

int
openBlog(struct Blog *blog, char *userid)
{
	return _openBlog(blog, userid, 0, 0);
}

int
openBlogW(struct Blog *blog, char *userid)
{
	return _openBlog(blog, userid, 1, 0);
}

int
openBlogD(struct Blog *blog, char *userid)
{
	return _openBlog(blog, userid, 1, 1);
}

void
closeBlog(struct Blog *blog)
{
#ifdef BLOG_SAFE_MODE
	flock(blog->lockfd, LOCK_UN);
	close(blog->lockfd);
#endif
	mmapfile(NULL, &blog->configFile);
	mmapfile(NULL, &blog->indexFile);
	mmapfile(NULL, &blog->draftFile);
	mmapfile(NULL, &blog->subjectFile);
	mmapfile(NULL, &blog->tagFile);
	mmapfile(NULL, &blog->linkFile);
	mmapfile(NULL, &blog->bitFile);
	mmapfile(NULL, &blog->lwFile);
}

void
reopenBlog(struct Blog *blog)
{
	int wantWrite, wantDraft;
	char userid[sizeof (blog->userid)];
	strcpy(userid, blog->userid);
	wantWrite = blog->wantWrite;
	wantDraft = blog->wantDraft;
	closeBlog(blog);
	_openBlog(blog, userid, wantWrite, wantDraft);
}

int
blogPost(struct Blog *blog, char *poster, char *title, char *tmpfile,
	 int subject, int tags[], int nTag, int flag, int hide, int attach)
{
	int fileTime;
	int i, j, t;
	char filePath[256];
	struct BlogHeader blogHeader;

	setBlogFile(filePath, blog->userid, "");
	fileTime = trycreatefile(filePath, "%d", time(NULL), 100);
	if (fileTime < 0)
		return -1;

	if (copyfile(tmpfile, filePath) < 0) {
		unlink(filePath);
		return -1;
	}

	bzero(&blogHeader, sizeof (blogHeader));
	blogHeader.fileTime = fileTime;
	strsncpy(blogHeader.title, title, sizeof (blogHeader.title));
	utf8cut(blogHeader.title, sizeof (blogHeader.title));

	blogHeader.hide = hide;
	if(flag) { 
		strsncpy(blogHeader.poster, poster, sizeof(blogHeader.poster));
	}
	if(flag == 2) { // leave word
		setBlogFile(filePath, blog->userid, "leaveword");
		goto DOPOST;
	}
	if (subject >= blog->nSubject || subject == 0)
		subject = 0;
	blogHeader.subject = subject;
	blog->subject[subject].count++;

	for (i = 0; i < MAXTAG; i++) {
		blogHeader.tag[i] = -1;
	}
	if (tags != NULL && nTag > 0) {
		for (i = 0, j = 0; i < nTag && j < MAXTAG; i++) {
			t = tags[i];
			if (t < 0 || t >= blog->nTag)
				continue;
			blogHeader.tag[j] = t;
			blog->tag[t].count++;
			j++;
		}
	}
	if(attach)
		blogHeader.hasAttach = 1;
	setBlogFile(filePath, blog->userid, "index");
      DOPOST:
	append_record(filePath, &blogHeader, sizeof (blogHeader));
	reopenBlog(blog);
	return fileTime;
}

int
blogSaveAbstract(struct Blog *blog, time_t fileTime, char *abstract)
{
	int n;
	char filePath[256];
	struct BlogHeader *blogHeader;

	if (fileTime < 0)
		return -1;
	n = findBlogArticle(blog, fileTime);
	if (n < 0)
		return -1;
	blogHeader = &blog->index[n];
	setBlogAbstract(filePath, blog->userid, fileTime);
	if (abstract) {
		f_write(filePath, abstract);
		if (!blogHeader->hasAbstract)
			blogHeader->hasAbstract = 1;
	} else {
		unlink(filePath);
		if (blogHeader->hasAbstract)
			blogHeader->hasAbstract = 0;
	}
	return 0;
}

int
blogUpdatePost(struct Blog *blog, time_t fileTime, char *title, char *tmpfile,
	       int subject, int tags[], int nTag)
{
	int i, j, n;
	char filePath[256];
	struct BlogHeader blogHeader;

	if (fileTime < 0)
		return -1;
	n = findBlogArticle(blog, fileTime);
	if (n < 0)
		return -1;
	blogHeader = blog->index[n];
	blogHeader.modifyTime = time(NULL);

	if (tmpfile) {
		setBlogPost(filePath, blog->userid, fileTime);
		if (copyfile(tmpfile, filePath) < 0) {
			return -1;
		}
	}

	if (subject < blog->nSubject && subject >= 0) {
		blog->subject[blogHeader.subject].count--;
		blogHeader.subject = subject;
		blog->subject[blogHeader.subject].count++;
	}

	if (tags && nTag) {
		for (i = 0, j = 0; i < nTag && j < MAXTAG; i++) {
			int t = tags[i];
			if (t < 0 || t >= blog->nTag)
				break;
			blog->tag[blogHeader.tag[j]].count--;
			blogHeader.tag[j] = t;
			blog->tag[t].count++;
			j++;
		}
	}
	if (title) {
		strsncpy(blogHeader.title, title, sizeof (blogHeader.title));
		utf8cut(blogHeader.title, sizeof (blogHeader.title));
	}
	blog->index[n] = blogHeader;
	reopenBlog(blog);
	return fileTime;
}

int
blogPostDraft(struct Blog *blog, char *title, char *tmpfile, int draftID)
{
	int fileTime;
	int i;
	char filePath[256];
	struct BlogHeader blogHeader;
	struct mmapfile mf = { ptr:NULL };

	if (draftID > 0) {
		fileTime = draftID;
		setBlogFile(filePath, blog->userid, "D");
		sprintf(filePath + strlen(filePath), "%d", draftID);
		if (!file_exist(filePath))
			draftID = -1;
	}
	if (draftID <= 0) {
		setBlogFile(filePath, blog->userid, "");
		fileTime = trycreatefile(filePath, "D%d", time(NULL), 100);
		if (fileTime < 0)
			return -1;
	}
	if (copyfile(tmpfile, filePath) < 0) {
		unlink(filePath);
		return -1;
	}

	bzero(&blogHeader, sizeof (blogHeader));

	blogHeader.fileTime = fileTime;
	for (i = 0; i < 6; i++)
		blogHeader.tag[i] = -1;
	blogHeader.subject = -1;
	strsncpy(blogHeader.title, title, sizeof (blogHeader.title));
	utf8cut(blogHeader.title, sizeof (blogHeader.title));

	setBlogFile(filePath, blog->userid, "draft");
	while (draftID > 0) {
		struct BlogHeader *blh;
		int i, count;
		if (mmapfilew(filePath, &mf) < 0) {
			draftID = -1;
			break;
		}
		blh = (struct BlogHeader *) mf.ptr;
		count = mf.size / sizeof (*blh);
		for (i = 0; i < count; i++) {
			if (blh[i].fileTime == draftID) {
				memcpy(&blh[i], &blogHeader, sizeof (*blh));
				break;
			}
		}
		mmapfile(NULL, &mf);
		if (i >= count)
			draftID = -1;
		break;
	}
	if (draftID <= 0) {
		append_record(filePath, &blogHeader, sizeof (blogHeader));
	}
	return fileTime;
}

int
deleteDraft(struct Blog *blog, int draftID)
{
	char filePath[80], buf[80];
	struct mmapfile mf = { ptr:NULL };
	struct BlogHeader *blh;
	int i, count;

	sprintf(buf, "D%d", draftID);
	setBlogFile(filePath, blog->userid, buf);
	unlink(filePath);
	setBlogFile(filePath, blog->userid, "draft");
	if (mmapfilew(filePath, &mf) < 0)
		return -1;
	blh = (struct BlogHeader *) mf.ptr;
	count = mf.size / sizeof (*blh);
	for (i = 0; i < count; i++) {
		if (blh[i].fileTime == draftID) {
			break;
		}
	}
	mmapfile(NULL, &mf);
	if (i >= count)
		return -1;
	delete_record(filePath, sizeof (*blh), i + 1);
	return 0;
}

int
blogComment(struct Blog *blog, time_t fileTime, char *tmpfile)
{
	struct BlogCommentHeader comment;
	char filePath[80], filePath2[80], buf[20];
	time_t commentTime;
	int fd;
	int n;

	setBlogFile(filePath, blog->userid, "");
	snprintf(buf, sizeof (buf), "%d.%%d", (int) fileTime);
	commentTime = trycreatefile(filePath, buf, time(NULL), 100);
	if (commentTime < 0)
		return -1;

	if (copyfile(tmpfile, filePath) < 0) {
		unlink(filePath);
		return -1;
	}
	//setBlogComment(filePath, blog->userid, fileTime, now);
	setBlogCommentIndex(filePath2, blog->userid, fileTime);
	fd = open(filePath2, O_WRONLY | O_APPEND | O_CREAT, 0660);
	if (fd < 0) {
		unlink(filePath);
		return -1;
	}
	comment.commentTime = commentTime;
	if (safewrite(fd, &comment, sizeof (comment)) < 0) {
		unlink(filePath);
		return -1;
	}
	close(fd);

	n = findBlogArticle(blog, fileTime);
	if (n < 0) {
		unlink(filePath);
		//unlink(filePath2);
		return -1;
	}
	blog->index[n].nComment++;
	return n;
}

int
blogCheckMonth(struct Blog *blog, char buf[32], int year, int month)
{

	return 0;
}

int
blogModifySubject(struct Blog *blog, int subjectID, char *title, int hide)
{
	int fd, isAdd = 0;
	struct BlogSubject *blogSubject, aBlogSubject;
	char buf[256];
	
	if (subjectID >= 64)
		return -1;
	if (subjectID > blog->nSubject || subjectID < 0)
		return -1;
	if (subjectID == blog->nSubject) {
		bzero(&aBlogSubject, sizeof (aBlogSubject));
		blogSubject = &aBlogSubject;
		isAdd = 1;
	} else {
		blogSubject = &blog->subject[subjectID];
	}

	strsncpy(blogSubject->title, title, sizeof (blogSubject->title));
	utf8cut(blogSubject->title, sizeof (blogSubject->title));
	if (hide)
		blogSubject->hide = 3;
	else
		blogSubject->hide = 0;
	if (!isAdd)
		return 0;
	setBlogFile(buf, blog->userid, "subject");
	fd = open(buf, O_WRONLY | O_CREAT, 0660);
	if (fd >= 0) {
		lseek(fd, subjectID * sizeof (struct BlogSubject), SEEK_SET);
		write(fd, blogSubject, sizeof (struct BlogSubject));
		close(fd);
		reopenBlog(blog);
		return 0;
	}
	return -1;
}

int
blogModifyTag(struct Blog *blog, int tagID, char *title)
{
	int fd, isAdd = 0;
	struct BlogTag *blogTag, aBlogTag;
	char buf[256];
	if (tagID >= 128)
		return -1;
	if (tagID > blog->nTag || tagID < 0)
		return -1;
	if (tagID == blog->nTag) {
		bzero(&aBlogTag, sizeof (aBlogTag));
		blogTag = &aBlogTag;
		isAdd = 1;
	} else {
		blogTag = &blog->tag[tagID];
	}

	strsncpy(blogTag->title, title, sizeof (blogTag->title));
	utf8cut(blogTag->title, sizeof (blogTag->title));
	if (!isAdd)
		return 0;
	setBlogFile(buf, blog->userid, "tag");
	fd = open(buf, O_WRONLY | O_CREAT, 0660);
	if (fd >= 0) {
		lseek(fd, tagID * sizeof (struct BlogTag), SEEK_SET);
		write(fd, blogTag, sizeof (struct BlogTag));
		close(fd);
		reopenBlog(blog);
		return 0;
	}
	return -1;
}

int
findBlogArticle(struct Blog *blog, time_t fileTime)
{
	int n;
		
	for (n = 0; n <blog->nIndex; n++) {
		if (blog->index[n].fileTime == fileTime)
			return n;
	}
	return -1;
}

//hide level:
//0 open
//1 open to friends
//2 blocked by the manager, can only be read by the owner the manager
//3 absolutely hide to others
int canReadBlogItem(unsigned int hlevel, char *readid, char *blogid) {
	if(hlevel == 0)	// open to public
		return 1;
	if(!strcmp(blogid, readid)) // myself, always read
		return 1;
	if(hlevel == 1)
		return inoverride(readid, blogid, "friends");
	return 0;
}


static void
blogTrace(s)
char *s;
{
	char buf[512];
	static int disable = 0;
	static int msqid = -1;
	struct mymsgbuf *msgp = (struct mymsgbuf *) buf;

	if (disable > 3)
		return;
	snprintf(msgp->mtext, sizeof (buf) - sizeof (msgp->mtype),
		 "%d %s\n", (int) time(NULL), s);
	msgp->mtype = 1;
	if (msqid < 0) {
		msqid = initmsq(BLOGLOG_MSQ);
		if (msqid < 0) {
			disable++;
			return;
		}
	}
	msgsnd(msqid, msgp, strlen(msgp->mtext), IPC_NOWAIT | MSG_NOERROR);
}

void
blogLog(char *fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);
	blogTrace(buf);
}

