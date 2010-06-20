#include "ythtlib.h"
#include "bbs.h"
#include <ght_hash_table.h>
#include "www.h"
#include <json/json.h>

#define _TOPN	32
#define max_blog 4093
#define max_read 1021
// Blog预览的最大字数，注意不要设置太大，防止溢出
// 详情看 getBlogPreview()
#define max_blog_preview 35
#define BLOGANNOUNCE	MY_BBS_HOME "/0Announce/groups/GROUP_0/Blog/announce"

#define F_BLOG	1
#define F_POST	2

//#define BLOG_DEBUG

static int backday;

static time_t now_t;
struct bbsinfo bbsinfo; //全局变量

struct hotBlog {
	char userid[STRLEN];
	int unum;
	int filetime;
};
struct data_blog {
	ght_hash_table_t *user_hash;
	struct hotBlog hb;
};
//struct hotBlog *hb;

struct cateBlogNav {
	struct hotBlog item[20];
	short count_hot, count_new;
};
struct cateBlogNav CBNav[32];

static int genHotBlog4bbs(struct hotBlog *hb, int count);
static int genHotPost4bbs(struct hotBlog *hb, int count);

#define MAXFILTER 100
char filterstr[MAXFILTER][60];
int nfilterstr = 0;

int
initfilter(char *filename)
{
	FILE *fp;
	char *ptr, *p;
	fp = fopen(filename, "r");
	if (!fp)
		return -1;
	for (nfilterstr = 0; nfilterstr < MAXFILTER; nfilterstr++) {
		ptr = filterstr[nfilterstr];
		if (fgets
		    (filterstr[nfilterstr], sizeof (filterstr[nfilterstr]),
		     fp) == NULL)
			break;
		if ((p = strchr(ptr, '\n')) != NULL)
			*p = 0;
		if ((p = strchr(ptr, '\r')) != NULL)
			*p = 0;
		while (ptr[0] == ' ')
			memmove(&ptr[0], &ptr[1], strlen(ptr));
		while (ptr[0] != 0 && ptr[strlen(ptr) - 1] == ' ')
			ptr[strlen(ptr) - 1] = 0;
		if (!ptr[0])
			nfilterstr--;
	}
	fclose(fp);
	return 0;
}

int
dofilter(char *str)
{
	int i;
	if (!nfilterstr)
		return 0;
	for (i = 0; i < nfilterstr; i++) {
		if (strcasestr(str, filterstr[i]))
			break;
	}
	if (i < nfilterstr)
		return 1;
	return 0;
}

static int genNewBlog() {
	return system("tail -n10 blog/blog.create | sort +1 -1 -r |filter /home/bbs/etc/filtertitle > blog/blog.create.www");
}

static int genNewPost() {
	struct tm *t;
	char buf[512], filename[256];

	t = localtime(&now_t);

	sprintf(filename, "blog/POST/%d-%d.post", t->tm_year + 1900, t->tm_yday / 7);
	sprintf(buf, "tail -n%d %s | sort +1 -1 -r > blog/blog.newpost.www", 
			_TOPN, filename);
	buf[sizeof(buf) - 1] = '\0';
	return system(buf);
}

int getBlogPreview(struct Blog *blog, char *userid, int idx, char *buf) {
	char *ptr, *str, filepath[STRLEN];
	struct mmapfile mf = { ptr: NULL };
	struct BlogHeader *blh;
	int i, length, count = 0, slen = strlen("\nbeginbinaryattach");
	unsigned int mask, flag = 0x1;

	if (NULL == blog || idx < 0 || idx >= blog->nIndex)
		return 0;

	blh = &(blog->index[idx]);
	if (blh->hasAbstract)
		setBlogAbstract(filepath, userid, blh->fileTime);
	else
		setBlogPost(filepath, userid, blh->fileTime);

	if (mmapfile(filepath, &mf) < 0)
		return 0;

	str = mf.ptr;
	while (*str && str < mf.ptr + (int)mf.size && count < max_blog_preview) {
		if (*str == '<') {
			if (NULL == (ptr = strchr(str, '>')))
				break;
			str = ++ptr;
			continue;
		} else if (!strncmp(str, "\nbeginbinaryattach", slen)) {
			if (NULL == (ptr = strchr(str + slen, '\n')))
				break;
			str = ptr++;
			continue;
		} else if (*str == '\r' || *str == '\n') {
			str++;
			continue;
		} else if (*str == '{' || *str == '}' || 
			*str == '[' || *str == ']') {
		    	str++;
			continue;
		}
		mask = 0x80;
		for (i = 6; i > 1; i--) {
			mask |= flag << i;
			if ((*str & mask ) != mask)
				break;
		}
		length = 7 - i;		// 获得 UTF8 字符的长度
		strncat(buf, str, length);
		if (length > 1)
			count++;
		else if (*str == ' ')
			count++;
		str += length;
	}
	mmapfile(NULL, &mf);
	return 1;
}

static int new2JSON() {
	char buf[STRLEN], *ptr, *str, preview[1024];
	char useridutf8[STRLEN], userid[IDLEN + 1];
	struct json_object *JObject, *tmpJObject;
	struct Blog blog;
	FILE *fp;
	int idx, fileTime, i = 0;
	const static struct file2JSON {
		char *filePath;
		char *tagFilePath;
		char *varName;
		int hasPreview;
	} list [] = {
		{
			MY_BBS_HOME "/blog/blog.newpost.www", 
			MY_BBS_HOME "/blog/blog.newpost.www.js", 
			"blogpage_newPostJSON", 
			1
		}, { 
			MY_BBS_HOME "/blog/blog.hotpost.www", 
			MY_BBS_HOME "/blog/blog.hotpost.www.js", 
			"blogpage_hotPostJSON", 
			1
		}, {
			MY_BBS_HOME "/blog/blog.create.www",
			MY_BBS_HOME "/blog/blog.create.www.js",
			"blogpage_newBlogJSON",
			0
		}, {
			MY_BBS_HOME "/blog/blog.hotblog.www", 
			MY_BBS_HOME "/blog/blog.hotblog.www.js",
			"blogpage_hotBlogJSON", 
			0
		}, {
			NULL, NULL, NULL, 0
		}
	};

	while (list[i].filePath) {
		if (NULL == (fp = fopen(list[i].filePath, "r"))) {
			i++;
			continue;
		}
		JObject = json_object_new_array();
		while (fgets(buf, sizeof(buf), fp)) {
			tmpJObject = json_object_new_object();
			if (NULL == (ptr = strchr((str = buf), '\t')))
				continue;
			*ptr++ = 0;
			fileTime = atoi(str);
			// Add Filetime (FT)
			json_object_object_add(tmpJObject, "FT", 
						json_object_new_int(fileTime));
			if (NULL == (ptr = strchr((str = ptr), '\t')))
				continue;
			*ptr++ = 0;
			// Add Author Id in Has (AE)
			json_object_object_add(tmpJObject, "AH", 
					json_object_new_string(str));
			if (NULL == (ptr = strchr((str = ptr), '\t')))
				continue;
			*ptr++ = 0;
			// Add Author id (AI)
			json_object_object_add(tmpJObject, "AI", 
					json_object_new_string(str));
			strncpy(useridutf8, str, sizeof(useridutf8));
			utf82gb(userid, sizeof(userid), useridutf8);
			if (NULL != (ptr = strchr((str = ptr), '\r')))
				*ptr = 0;
			if (NULL != (ptr = strchr(str, '\n')))
				*ptr = 0;
			// Add Blog Artilce Title
			json_object_object_add(tmpJObject, "AT", 
					json_object_new_string(str));
			if (openBlogW(&blog, userid) < 0) {
				json_object_put(tmpJObject);
				continue;
			}
			json_object_object_add(tmpJObject, "BT", 
					json_object_new_string(blog.config->title));
			// Add Blog Article Preview
			if (!list[i].hasPreview)	{	//新建BLOG或热门BLOG没有预览
				json_object_array_add(JObject, tmpJObject);
				closeBlog(&blog);
				continue;
			}
			if ((idx = findBlogArticle(&blog, fileTime)) < 0) {
				json_object_put(tmpJObject);
				continue;
			}
			bzero(preview, sizeof(preview));
			getBlogPreview(&blog, userid, idx, preview);
			json_object_object_add(tmpJObject, "AP", 
					json_object_new_string(preview));
			closeBlog(&blog);
			json_object_array_add(JObject, tmpJObject);
		}
		fclose(fp);
		if (NULL == (fp = fopen(list[i].tagFilePath, "w"))) {
			json_object_put(JObject);
			i++;
			continue;
		}
		fprintf(fp, "<script>\n%s = %s;\n</script>\n", 
				list[i].varName, json_object_to_json_string(JObject));
		json_object_put(JObject);
		fclose(fp);
		i++;
	}
	return 1;
}

static int ann2JSON() {
	char buf[1024], *str, *ptr, filepath[STRLEN]; 
	struct mmapfile mf = { ptr: NULL };
	struct json_object *annObj;
	FILE *fp;

	strncpy(filepath, BLOGANNOUNCE ".utf8", sizeof(filepath));
	snprintf(buf, sizeof(buf), "iconv -f gbk -t utf8 -c %s > %s", 
			BLOGANNOUNCE, filepath);
	system(buf);
	if (mmapfile(filepath, &mf) < 0)
		return 0;
	annObj = json_object_new_array();
	str = mf.ptr; 
	while (str < mf.ptr + (int) mf.size && *str) {
		bzero(buf, sizeof(buf));
		strncpy(buf, str, sizeof(buf) - 1);
		if (NULL == (ptr = strchr(buf, '\0')))
			break;
		ptr--;
		if (ptr <= buf)
			break;
		while (((unsigned char)*ptr | 0xBF) == 0xBF && 
				((unsigned char)*ptr & 0x80) == 0x80) {
			ptr--;
		}
		if (((unsigned char )*ptr | 0x7F ) != 0x7F) {
			*ptr = 0;
			str += ptr - buf;
		} else
			str += ptr - buf + 1;
		json_object_array_add(annObj, json_object_new_string(buf));
	}
	mmapfile(NULL, &mf);
	strncpy(filepath, MY_BBS_HOME "/blog/blog.announce.www.js", 
			sizeof(filepath));
	if (NULL == (fp = fopen(filepath, "w"))) {
		json_object_put(annObj);
		return 0;
	}
	fprintf(fp, "<script>\nblogpage_annJSON = %s;\n</script>\n", 
			json_object_to_json_string(annObj));
	fclose(fp);
	json_object_put(annObj);
	return 1;
}

static char *getfilename(int i)
{
	static char logf[256];
	time_t dtime;
	struct tm *t;

	dtime = now_t - i * 86400;
	t = localtime(&dtime);
	sprintf(logf, MY_BBS_HOME "/newtrace/%d-%02d-%02d.blog",
		1900 + t->tm_year, 1 + t->tm_mon, t->tm_mday);
	return logf;
}

static int
cmphb(struct hotBlog *a, struct hotBlog *b)
{
        return b->unum - a->unum;
}

static int 
genHotItemFile(struct hotBlog *hb, int count, int flag) {
	FILE *fp;
	int i,  n;
	char tmpfile[STRLEN*2], title[STRLEN * 2];
	struct Blog blog;
	char filename[][STRLEN*2] = {"", "blog/blog.hotblog.www", 
					 "blog/blog.hotpost.www"};

	sprintf(tmpfile, "%s.tmp", filename[flag]);
	fp = fopen(tmpfile, "w");
	if(fp == NULL) 
		return -1;

	//format:score[tab]userid[tab]username[tab]blogtitle[\n]
	for(i=0; i < count && i < _TOPN; i++) {
		if(openBlog(&blog, hb[i].userid) < 0)
			continue;
		if(flag == F_BLOG)
			strncpy(title, blog.config->title, sizeof(title));
		else if(flag == F_POST) {
			n = findBlogArticle(&blog, hb[i].filetime);
			if(n < 0)
				continue; // find the next top post
			if(blog.index[n].hide)
				continue; // non-public post
			strcpy(title, (blog.index[n]).title);
		} else	
			sprintf(title, "%s的网路日记", hb[i].userid);
		title[sizeof(title)-1] = 0;
		if(dofilter(title))
			continue;
		fprintf(fp, "%d\t%s\t%s\t%s\n", hb[i].filetime, blog.config->useridEN, 
				blog.config->useridUTF8, title);
	}
	fclose(fp);
	rename(tmpfile, filename[flag]);
	return 0;
}

static int insertItem(ght_hash_table_t *p_table, char *blogid,  char *readid,  int filetime, int flag) {
	int *usernum;
	struct data_blog *data;
	void *key1, *key2;
	int sk1, sk2;

	if(flag == F_BLOG) {	//hot blog
		key1 = blogid;
		key2 = readid;
		sk1 = strlen(blogid);
		sk2 = strlen(readid);
	} else if(flag == F_POST) { //hot post
		key1 = &filetime;
		key2 = readid;
		sk1 = sizeof(filetime);
		sk2 = strlen(readid);
	} else
		return 0;
	data = ght_get(p_table, sk1, key1);
	if (data == NULL) { // new item
		if((data = malloc(sizeof(struct data_blog))) == NULL) {
			errlog("malloc failed");
			exit(-1);
		}
		if((usernum = malloc(sizeof(int))) == NULL) {
			errlog("malloc failed");
			exit(-1);
		}
		data->user_hash = NULL;
		data->user_hash = ght_create(max_read);
		data->hb.unum = 1;
		data->hb.filetime = filetime;
		strsncpy(data->hb.userid, blogid, STRLEN);
		*usernum = 0;
		ght_insert(data->user_hash, usernum, sk2, key2);
		ght_insert(p_table, data, sk1, key1);
		//printf("a: #%s# %d\n", readid, *usernum);
		return 0;
	}
	usernum = ght_get(data->user_hash, sk2, key2);
	if(usernum == NULL) { // new reader
		(data->hb.unum)++;
		if((usernum = malloc(sizeof(int))) == NULL) {
			errlog("malloc failed");
			exit(-1);
		}
		*usernum = 0;
		ght_insert(data->user_hash, usernum, sk2, key2);
		//printf("b: #%s#\n", readid);
	} else {
		//one reader for the same item
		(*usernum)++;
		//printf("c: #%s# %d\n", readid, *usernum);
	}
	return 0;
}


static int bonus(struct hotBlog *hb, int num) {
	int i, exp;
	char buf[512], content[1024], title[80], userid[20];
	time_t now;
	int fd;
	struct userdata data;

	fd = open("etc/blog_bonus", O_RDWR | O_CREAT, 0660);
	if(fd < 0)
		return -1;
	read(fd, &now, sizeof(now));
	if(now_t - now < 86400*backday) {
		close(fd);
		return 1;
	}
	
	for(i = 0; i < num; i++) {
		if(hb[i].userid[0] == '\0')
			continue;
		exp = MAX(10-i, 1);
		if(loaduserdata(hb[i].userid, &data) < 0) {
			printf("failed to open blog:%s\n", hb[i].userid);
			continue;
		}
	        data.extraexp += exp;
	        saveuserdata(hb[i].userid, &data);
		snprintf(buf, sizeof(buf), "恭喜，您写Blog实在是太有才了！ "
				"您获得 %d 精华值的奖励。\n\n\n", exp);
		utf82gb(content, sizeof(content), buf);
		snprintf(buf, sizeof(buf), "Blog奖励通知");
		utf82gb(title, sizeof(title), buf);
		utf82gb(userid, sizeof(userid), hb[i].userid);
		system_mail_buf(content, strlen(content), userid, title, "deliver");
//		printf("%s\t%d\n", hb[i].userid, exp);
	}

	now = now_t;
	lseek(fd, 0, SEEK_SET);
	write(fd, &now, sizeof(now));
	close(fd);
	return 0;
}

				
static int topItem(ght_hash_table_t *p_table, int flag) {
	int j=0, *usernum;
	void *key, *key1;
        struct data_blog *data;
	struct hotBlog *hb, *hb1;
        ght_iterator_t iterator, iterator1;

	hb = malloc(sizeof(struct hotBlog) * max_blog);
	if (hb == NULL) {
		errlog("malloc failed");
		exit(-1);
	}
	bzero(hb, sizeof(sizeof(struct hotBlog) * max_blog));
	hb1 = hb;

	for (data = ght_first(p_table, &iterator, &key); data;
	     data = ght_next(p_table, &iterator, &key)) {
		memcpy(hb1, &(data->hb), sizeof (struct hotBlog));
		hb1++;
		j++;
		for (usernum = ght_first(data->user_hash, &iterator1, &key1);usernum;
		     usernum = ght_next(data->user_hash, &iterator1, &key1)) {
			free(usernum);
		}
		ght_finalize(data->user_hash);
		data->user_hash = NULL;
		free(data);
	}
	ght_finalize(p_table);
	p_table = NULL;
	qsort(hb, j, sizeof (struct hotBlog), (void *) cmphb);
	
	genHotItemFile(hb, j, flag);
	if(flag == F_BLOG)
		genHotBlog4bbs(hb, MIN(j, 10));
	if(flag == F_POST) {
		genHotPost4bbs(hb, MIN(j, 6));
		bonus(hb, MIN(j, 8));
	}

#ifdef BLOG_DEBUG
	{
		int i;
		printf("\n%d\n", flag);
		for(i=0; i<j; i++) {
			if(flag == F_BLOG)
				printf("%s %d\n", hb[i].userid, hb[i].unum);
			else if(flag == F_POST)
				printf("%d %d\n", hb[i].filetime, hb[i].unum);
		}
	}
#endif
	return 0;
}


static int blogstat() {
	int i, filetime;
	char *filename, *ptr, *start, buf[512], blogid[STRLEN], readid[STRLEN];
	FILE *fp;
	ght_hash_table_t *p_table = NULL, *b_table = NULL;
	time_t from_t;

	from_t = now_t - backday * 86400;
	//printf("from time:%ld\n", from_t);
	p_table = ght_create(max_blog);
	b_table = ght_create(max_blog);

	for (i = backday; i >= 0; i--) {
		filename = getfilename(i);
		//printf("%s\n", filename);
		fp = fopen(filename, "r");
		if(fp == NULL)
			continue;
		while(fgets(buf, sizeof(buf), fp)) {
			//printf("%s ", buf);
			start = buf;	// log time
			ptr = strchr(start, ' ');
			if(ptr == NULL)
				continue;
			start = ++ptr;	// action
			ptr = strchr(start, ' ');
			if(ptr == NULL)
				continue;
			*ptr = 0;
			if(strncasecmp("Read", start, 4))
				continue;
			start = ++ptr;	// blogid
			ptr = strchr(start, ' ');
			if(ptr == NULL)
				continue;
			*ptr = 0;
			strcpy(blogid, start);
				
			start = ++ptr;	// filetime
			ptr = strchr(start, ' ');
			if(ptr == NULL)
				continue;
			filetime = atoi(start);

			start = ++ptr; // readid(ip)
			ptr = strchr(start, '\n');
			if(ptr)
				*ptr = 0;			
			strcpy(readid, start);

#ifdef BLOG_DEBUG
			printf("logtime:%d\tblog:%s\tfile:%d\tvisitor:%s\n",
				atoi(buf), blogid, filetime, readid);
#endif

			if(filetime >= from_t) { //hotpost
				insertItem(p_table, blogid, readid, filetime, F_POST);
			}
			insertItem(b_table, blogid, readid, filetime, F_BLOG); //hotblog
		} //end of while
		fclose(fp);
	} //end of for
	topItem(p_table, F_POST);
	topItem(b_table, F_BLOG);
	return 0;
}


static int genHotBlog4bbs(struct hotBlog *hb, int count) {
	FILE *fp;
	int i;
	char tmpfile[STRLEN*2], cmd[1024];
	struct Blog blog;
	const char *filename ="blog/bbs.hotblog.www";

	sprintf(tmpfile, "%s.tmp", filename);
	fp = fopen(tmpfile, "w");
	if(fp == NULL) 
		return -1;

	//format:score[tab]userid[tab]username[tab]blogtitle[\n]
	for(i=0; i < count && i < _TOPN; i++) {
		if(hb[i].userid[0] == '\0')
			continue;
		if(openBlog(&blog, hb[i].userid) < 0)
			continue;
		if(dofilter(blog.config->title))
			continue;
		fprintf(fp, "<li><a href=\"/HT/blog?U=%s\" target=_self>%s</a></li>\n", 
				blog.config->useridUTF8, blog.config->title);
	}
	fprintf(fp, "<li><a href=/HT/blogsetup class=red "
			"title=\"简单好用的Blog，就是这里了！\">"
			"我要申请Blog！</a></li>");
	fclose(fp);
	snprintf(cmd, sizeof(cmd), "iconv -f utf-8 -t gbk %s > %s", tmpfile, filename);
	system(cmd);
	return 0;
}


static int genHotPost4bbs(struct hotBlog *hb, int count) {
	FILE *fp;
	int i, n;
	char tmpfile[STRLEN*2], cmd[1024];
	struct Blog blog;
	const char *filename ="blog/bbs.hotpost.www";

	sprintf(tmpfile, "%s.tmp", filename);
	fp = fopen(tmpfile, "w");
	if(fp == NULL) 
		return -1;

	//format:score[tab]userid[tab]username[tab]blogtitle[\n]
	fprintf(fp, "<div class=\"digest_box\">");
	for(i=0; i < count && i < _TOPN; i++) {
		if(hb[i].userid[0] == '\0')
			continue;
		if(openBlog(&blog, hb[i].userid) < 0)
			continue;
		n = findBlogArticle(&blog, hb[i].filetime);
		if(n < 0||dofilter((blog.index[n]).title))
			continue; // find the next top post
		fprintf(fp, "<div class=\"overflow\"><li><a href=\"/HT/blogread?U=%s&T=%d\" title=\"%s\">%s</a>"
			"[<a href=\"/HT/blog?U=%s\" class=blk>%s</a>]</li></div>\n", 
			blog.config->useridUTF8, hb[i].filetime, (blog.index[n]).title, (blog.index[n]).title,
			blog.config->useridUTF8, blog.config->title);
	}
	fprintf(fp, "</div>");
	fclose(fp);
	snprintf(cmd, sizeof(cmd), "iconv -f utf-8 -t gbk %s > %s", tmpfile, filename);
	system(cmd);
	return 0;
}


void accessStat() {
	FILE  *fp;
	char buf[64], *ptr, *fpath = BLOG_DIR_PATH "/access.log";
	struct Blog blog;

	fp = fopen(fpath, "r");
	if(fp == NULL)
		return;
	while(fgets(buf, sizeof(buf), fp)) {
		ptr = strchr(buf, ' ');
		if(ptr == NULL)
			continue;
		*ptr = 0;
		if(openBlogW(&blog, buf) < 0)
			continue;
		blog.config->visit += atoi(++ptr);
		closeBlog(&blog);
	}
	fclose(fp);
	unlink(fpath);
}

int main(int argc, char **argv) {
	
	if(initbbsinfo(&bbsinfo) < 0) {
		printf("Init bbs info failed.\n");
		return -1;
	}

	now_t = time(NULL);
	if(argc == 2) {
		backday = atoi(argv[1]);
		backday = MIN(backday, 7);
		backday = MAX(backday, 1);
	} else
		backday = 1;

	chdir(MY_BBS_HOME);
	initfilter(MY_BBS_HOME "/etc/filtertitle");
	if(mkdir("blog", 0770) == 0) {	// init dir here
		int i;
		char dirpath[80];

		for(i = 'A'; i <= 'Z'; i++) {
			sprintf(dirpath, "blog/%c", i);
			mkdir(dirpath, 0770);
		}
		mkdir("blog/CATEGORY", 0770);
		mkdir("blog/POST", 0770);
	}
	genNewBlog();
	genNewPost();
	blogstat();
	accessStat();
	new2JSON();
	ann2JSON();
	return 0;
}

