#include "bbs.h"
// change 31 days to 10 years
#define MAX_LIVE 10 * 12 * 31 * 24 * 3600 
#define HTMPATH "/var/www"

struct bbsinfo bbsinfo;
struct BCACHE *shm_bcache;

int print_smindex_head(FILE *fp) {
	if (!fp)
		return -1;
	fprintf(fp, "<?xml version='1.0' encoding='UTF-8'?>\n"
		"<sitemapindex xmlns=\""
		"http://www.google.com/schemas/sitemap/0.84\">\n\n");
	return 1;
}

int validboard(struct boardmem *x) {
	if (!x)
		return 0;
	if (!(x->header.filename[0]))
		return 0;
	if (x->header.flag & CLUB_FLAG)
		return 0;
	if (x->header.level == 0)
		return 1;
	if (x->header.level & (PERM_POSTMASK | PERM_NOZAP))
		return 1;
	return 0;
}

int print_sm_item(FILE *fp, struct fileheader fh, int num, time_t now) {
	time_t filetime = (time_t) fh.filetime;
	struct tm *lastmod = localtime(&filetime);

	if (!fp)
		return -1;
	fprintf(fp, "<url>\n"
		"\t<loc>http://" MY_BBS_DOMAIN "/" SMAGIC 
		"/con_%d_M.%d.A.htm</loc>\n"
		"\t<lastmod>%d-%02d-%02dT%02d:%02d:%02d+08:00</lastmod>\n"
		"\t<changefreq>never</changefreq>\n"
		"</url>\n", num, fh.filetime, 
		lastmod->tm_year + 1900, lastmod->tm_mon + 1, lastmod->tm_mday,
		lastmod->tm_hour, lastmod->tm_min, lastmod->tm_sec);
#ifdef DEBUG
	printf("writing %d %s\n", fh.filetime, fh.title);
#endif
	return 1;
}

int print_sm_board_head(FILE *fp) {
	if (!fp)
		return -1;
	fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<urlset xmlns=\"http://www.google.com/"
		"schemas/sitemap/0.84\">\n\n");
	return 1;
}

int print_sm_board_foot(FILE *fp) {
	if (!fp)
		return -1;
	fprintf(fp, "\n</urlset>");
	return 1;
}

int print_smindex_item(FILE *smindex, int num, time_t now) {
	struct tm *lastmod = localtime(&now);
	
	if (!smindex)
		return -1;
	fprintf(smindex, "<sitemap>\n"
		"\t<loc>http://" MY_BBS_DOMAIN "/google_sm_%d.xml.gz</loc>\n"
		"\t<lastmod>%d-%02d-%02dT%02d:%02d:%02d+08:00</lastmod>\n"
		"</sitemap>\n\n", num, 
		lastmod->tm_year + 1900, lastmod->tm_mon + 1, lastmod->tm_mday, 
		lastmod->tm_hour, lastmod->tm_min, lastmod->tm_sec);
	return 1;
}

int print_sm_board(char *filename, int num, FILE *smindex) {
	char buf[STRLEN];
	FILE *fp;
	int fd, total, i, count = 0;
	struct fileheader fh;
	time_t now = time(NULL);

	if (!smindex)
		return -1;

	sprintf(buf, HTMPATH "/google_sm_%d.xml", num);
	if (!(fp = fopen(buf, "w")))
		return -1;

	print_sm_board_head(fp);
	
	sprintf(buf, MY_BBS_HOME "/boards/%s/.DIR", filename);
	if ((fd = open(buf, O_RDONLY, 0)) == -1) {
		fclose(fp);
		return -1;
	}
	total = file_size(buf) / sizeof(fh);

#ifdef DEBUG
	printf("processing [%d]%s, %d articles in total.\n", 
			num, filename, total);
#endif

	for (i = total - 1; i >= 0; i--) {
		if (lseek(fd, i * sizeof(fh), SEEK_SET) < 0)
			break;
		if (read(fd, &fh, sizeof(fh)) != sizeof(fh))
			break;
		if (now - fh.filetime > MAX_LIVE)
			break;
		if (fh.accessed & FH_1984)
			continue;
		//if (fh.filetime != fh.thread)
		//	continue;
		print_sm_item(fp, fh, num, now);
		count++;
	}

	print_sm_board_foot(fp);
	fclose(fp);

	sprintf(buf, "gzip -9fq " HTMPATH "/google_sm_%d.xml", num);
	system(buf);

	if (count)
		print_smindex_item(smindex, num, now);
	return 1;
}

int print_sm_user_head(FILE *fp) {
	return print_sm_board_head(fp);
}

int print_sm_user_foot(FILE *fp) {
	return print_sm_board_foot(fp);
}

int print_sm_user(FILE *smindex) {
	char buf[STRLEN], userid[STRLEN];
	FILE *fp;
	struct userec rec;
	int fd, i;
	time_t now = time(NULL);
	struct tm *lastmod = localtime(&now);
	
	if (!smindex)
		return -1;

	strncpy(buf, HTMPATH "/google_sm_user.xml", sizeof(buf));
	if (!(fp = fopen(buf, "w")))
		return -1;
	
	if ((fd = open(MY_BBS_HOME "/" PASSFILE, O_RDONLY, 0600)) == -1)
		return -1;

	print_sm_user_head(fp);

	for (i = 0; i < MAXUSERS; i++) {
		if (read(fd, &rec, sizeof(rec)) != sizeof (struct userec))
			break;
		if (!(rec.userid[0]))
			continue;
		gb2utf8(userid, sizeof(userid), rec.userid);
		fprintf(fp, "<url>\n"
			"\t<loc>http://" MY_BBS_DOMAIN "/" SMAGIC 
			"/qry?U=%s</loc>\n"
			"\t<lastmod>%d-%02d-%02dT%02d:%02d:%02d+08:00"
			"</lastmod>\n"
			"\t<changefreq>never</changefreq>\n"
			"</url>\n", userid, 
			lastmod->tm_year + 1900, 
			lastmod->tm_mon + 1, lastmod->tm_mday,
			lastmod->tm_hour, lastmod->tm_min, lastmod->tm_sec);
#ifdef DEBUG
		printf("Processing User [%s]\n", rec.userid);
#endif
	}
	close(fd);
	print_sm_user_foot(fp);
	fclose(fp);

	sprintf(buf, "gzip -9fq " HTMPATH "/google_sm_user.xml");
	system(buf);
	
	fprintf(smindex, "<sitemap>\n"
		"\t<loc>http://" MY_BBS_DOMAIN "/google_sm_user.xml.gz</loc>\n"
		"\t<lastmod>%d-%02d-%02dT%02d:%02d:%02d+08:00</lastmod>\n"
		"</sitemap>\n\n", 
		lastmod->tm_year + 1900, lastmod->tm_mon + 1, lastmod->tm_mday, 
		lastmod->tm_hour, lastmod->tm_min, lastmod->tm_sec);

	return 1;
}

int print_smindex_foot(FILE *smindex) {
	if (!smindex)
		return -1;

	fprintf(smindex, "</sitemapindex>");
	return 1;
}

#ifdef ENABLE_BLOG
int print_sm_blog_head(FILE *fp) {
	return print_sm_board_head(fp);
}

int print_sm_blog_foot(FILE *fp) {
	return print_sm_board_foot(fp);
}

int print_sm_blog_user(FILE *smindex, int i, char *userid) {
	FILE *fp;
	char buf[STRLEN];
	struct Blog blog;
	struct BlogHeader *bh;
	int count = 0, j;
	time_t filetime, modifytime, now;
	struct tm *lastmod;

	if (i < 1 || i > MAXUSERS || NULL == smindex)
		return -1;
	
	if (openBlog(&blog, userid) < 0)
		return -1;

	if (!canReadBlogItem(blog.config->hl, "guest", blog.userid))
		return -1;

	sprintf(buf, HTMPATH "/google_sm_blog_%d.xml", i + 1);
	if (NULL == (fp = fopen(buf, "w"))) {
		closeBlog(&blog);
		return -1;
	}

	sprintf(buf, MY_BBS_HOME "/blog/%c/%s/index", mytoupper(userid[0]), 
			userid);

	print_sm_blog_head(fp);
	for (j = 0; j < blog.nIndex; j++) {
		bh = &(blog.index[j]);
		if (bh->hide != 0 || bh->blocked)
			continue;
#ifdef DEBUG
		printf("Title: %s / Time: %d\n", bh.title, (int)bh.fileTime);
#endif
		filetime = (time_t) bh->fileTime;
		modifytime = bh->modifyTime ? 
			(time_t) bh->modifyTime : filetime;
		lastmod = localtime(&modifytime);
		fprintf(fp, "<url>\n"
			"\t<loc>http://" MY_BBS_DOMAIN "/" SMAGIC 
			"/blogread?U=%d&amp;T=%d</loc>\n"
			"\t<lastmod>%d-%02d-%02dT%02d:%02d:%02d+08:00"
			"</lastmod>\n"
			"\t<changefreq>never</changefreq>\n"
			"</url>\n", i + 1, (int) filetime,  
			lastmod->tm_year + 1900, 
			lastmod->tm_mon + 1, lastmod->tm_mday,
			lastmod->tm_hour, lastmod->tm_min, lastmod->tm_sec);
		count++;
	}
	print_sm_blog_foot(fp);
	if (!count) {
		fclose(fp);
		closeBlog(&blog);
		return 0;
	}
	now = time(NULL);
	lastmod = localtime(&now);
	fclose(fp);
	closeBlog(&blog);
	
	sprintf(buf, "gzip -9fq " HTMPATH "/google_sm_blog_%d.xml", i + 1);
	system(buf);

	fprintf(smindex, "<sitemap>\n"
		"\t<loc>http://" MY_BBS_DOMAIN "/google_sm_blog_%d.xml.gz"
		"</loc>\n"
		"\t<lastmod>%d-%02d-%02dT%02d:%02d:%02d+08:00</lastmod>\n"
		"</sitemap>\n\n", i + 1,
		lastmod->tm_year + 1900, lastmod->tm_mon + 1, lastmod->tm_mday, 
		lastmod->tm_hour, lastmod->tm_min, lastmod->tm_sec);
	return 1;
}

int print_sm_blog(FILE *smindex) {
	struct userec rec;
	int fd, size;
	int i;

	if (!smindex)
		return -1;

	if ((fd = open(MY_BBS_HOME "/" PASSFILE, O_RDONLY, 0600)) == -1)
		return -1;

	size = file_size(MY_BBS_HOME "/" PASSFILE) / sizeof(struct userec);

	for (i = 0; i < size && i < MAXUSERS; i++) {
		if (read(fd, &rec, sizeof(struct userec)) != 
				sizeof(struct userec))
			break;
		if (!(rec.userid[0]) || !rec.hasblog)
			continue;
#ifdef DEBUG
		printf("Process blog of [%s]..\n", rec.userid);
#endif
		print_sm_blog_user(smindex, i, rec.userid);
	}
	return 1;
}
#endif

int main() {
	int i;
	FILE *smindex;
	struct boardmem x;
	char buf[512];

	if (initbbsinfo(&bbsinfo) < 0)
		return -1;
	sprintf(buf, HTMPATH "/googlesmindex.xml");
	if (!(smindex = fopen(buf, "w")))
		return -1;
	print_smindex_head(smindex);
	shm_bcache = bbsinfo.bcacheshm;
	for (i = 0; i < shm_bcache->number; i++) {
		x = shm_bcache->bcache[i];
		if (!validboard(&x))
			continue;
		print_sm_board(x.header.filename, i, smindex);
	}
#ifdef ENABLE_BLOG
	print_sm_blog(smindex);
#endif
	print_sm_user(smindex);
	print_smindex_foot(smindex);
	fclose(smindex);

#ifndef DEBUG
	sprintf(buf, "wget -q --spider "
		"http://www.google.com/webmasters/sitemaps/"
		"ping?sitemap=http%%3A%%2F%%2F" MY_BBS_DOMAIN 
		"%%2Fgooglesmindex.xml");
	system(buf);
#endif
	return 0;
}
