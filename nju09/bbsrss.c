#include "bbsrss.h"

static void
rss_print_str(char *str) {
	while (*str) {
		switch (*str) {
			case '<':
				printf("&lt;");
				break;
			case '>':
				printf("&gt;");
				break;
			case '&':
				printf("&amp;");
				break;
			default:
				printf("%c", *str);
		}
		str++;
	}
}

static void
bbsrss_header(char *encode)
{
	printf("Content-type: text/xml\r\n\r\n"
			"<?xml version=\"1.0\" encoding=\"%s\"?>"
			"<!DOCTYPE rss [<!ENTITY %% HTMLlat1 PUBLIC \""
			"-//W3C//ENTITIES Latin 1 for XHTML//EN\" "
			"\"http://www.w3.org/TR/xhtml1/"
			"DTD/xhtml-lat1.ent\">]>", encode);
}

static void
rss_head(char *title, char *link, char *description)
{
	printf("<rss version=\"2.0\">"
			"<channel>"
			"<title>[" MY_BBS_ID "] ");
	rss_print_str(title);
	printf("</title><link>http://" MY_BBS_DOMAIN "/" SMAGIC "/");
	rss_print_str(link);
	printf("</link><description>");
	rss_print_str(description);
	printf("</description><language>zh-cn</language>"
			"<webMaster>" ADMIN_EMAIL "</webMaster>"
			"<copyright>Copyright 2006, " 
			MY_BBS_NAME ".</copyright>");
}

static void
rss_end(void)
{
	printf("</channel>");
	printf("</rss>");
}

static void
rss_failed(char *msg, int retv) {
	printf("<item>"
			"<title>RSS 更新失败</title>"
			"<description><![CDATA["
			"RSS 来源更新失败，可能的原因：<br /><br />");
	if (msg)
		printf("0 %s<br />", msg);
	printf("1 您的 feed 地址有误。<br />"
			"2 本站 RSS 系统出现暂时故障。<br />"
			"3 本站 RSS 系统存在 BUG 。<br /><br />"
			"如果您有任何意见或建议，请"
			"<a href=\"mailto:" ADMIN_EMAIL "\">联络我们"
			"</a>。]]></description>"
			"</item>");
	if (retv) {
		rss_end();
		http_quit();
	}
}

static int
rss_print_article (int bnum, char *boardname, struct fileheader *fh, 
		int *pub, int *edit) {
	char path[STRLEN], filename[24];
	struct mmapfile mf = { ptr:NULL };
	
	if (!fh || !boardname || !boardname[0])
		return 0;

	sprintf(path, MY_BBS_HOME "/boards/%s/M.%d.A", 
			boardname, fh->filetime);
	sprintf(filename, "M.%d.A", fh->filetime);
	if (mmapfile(path, &mf) < 0)
		return 0;

	MMAP_TRY {
		printf("\n<item><title><![CDATA[");
		rss_print_str(fh->title);
		printf("]]></title><link>http://" MY_BBS_DOMAIN 
				"/" SMAGIC "/con_%d_"
				"%s.htm?redirect=1</link>", 
				bnum, filename);
		if (fh->owner[0] != '.')
			printf("<author>%s.bbs@" MY_BBS_DOMAIN "</author>", 
					fh->owner);
		printf("<pubDate>");
		rss_date(Ctime((time_t)(fh->filetime)));
		printf("</pubDate><description><![CDATA[");
		mem2rss(stdout, mf.ptr, mf.size, filename);
		printf("]]></description></item>");
		*pub = *pub < fh->filetime ? fh ->filetime : *pub;
		*edit = *edit < fh->edittime ? fh->edittime : *edit;
	} MMAP_CATCH {
	} MMAP_END mmapfile(NULL, &mf);
	return 1;
}

static void
rss_show_digest(char sec) {
	char boardname[24], path[STRLEN]; 
	char seclist[24], buf[512];
	struct mmapfile mf = { ptr:NULL }, dirmf = { ptr:NULL };
	struct digestitem {
		int bnum;
		int ftime;
		char title[60];
		char owner[14];
		char bname[24];
		char sec;
		int eva;
	} *pdigest = NULL;
	int i, retv, pub = 0, edit = 0, size = 0, count = 0, lastbnum = -1; 
	struct boardmem *x;
	struct fileheader *fh = NULL;
	FILE *fp;

	rss_head("精彩话题", "boa?secstr=&redirect=1",
			MY_BBS_DOMAIN "文摘及精彩话题");

	if (mmapfile(MY_BBS_HOME "/wwwtmp/digest", &mf) < 0)
		rss_failed("系统无法访问文摘记录1。", 1);

	if (NULL == (fp = fopen(MY_BBS_HOME "/wwwtmp/digestcolum", "r")))
		rss_failed("系统无法访问文摘记录2。", 1);

	seclist[0] = 0;
	while (fgets(buf, sizeof(buf), fp)) {
		if (buf[0] != '#' && buf[0] == sec) {
			sscanf(buf, "%*d\t%*s\t%s\t%*c\t%*d\t%*s\n", seclist);
			break;
		}
	}
	fclose(fp);

	if (!seclist[0])
		rss_failed("您订阅的文摘栏目不存在。", 1);
	
	MMAP_TRY {
		while (count < RSS_MAX_DIGEST_ITEM && 
				size <= mf.size - sizeof(struct digestitem)) {
			retv = 0;
			pdigest = (struct digestitem *) (mf.ptr + size);
			if (!strchr(seclist, pdigest->sec)) {
				size += sizeof(struct digestitem);
				continue;
			}

			if (lastbnum != pdigest->bnum || dirmf.size == 0) {
				mmapfile(NULL, &dirmf);
				sprintf(boardname, "%d", pdigest->bnum);
				if (!(x = getboard(boardname))) {
					size += sizeof(struct digestitem);
					continue;
				}
				sprintf(path, MY_BBS_HOME "/boards/%s/.DIR", 
						boardname);
				if (mmapfile(path, &dirmf) < 0) {
					size += sizeof(struct digestitem);
					mmapfile(NULL, &dirmf);
					continue;
				}
			}
			i = 0;
			while ((i + 1)* sizeof(struct fileheader) <= 
					dirmf.size) {
				fh = (struct fileheader *)
					(dirmf.ptr + dirmf.size 
					 - (i + 1) * 
					 sizeof(struct fileheader));
				if (fh->filetime == pdigest->ftime) {
					retv = 1;
					break;
				}
				i++;
			}

			if (!retv) {
				size += sizeof(struct digestitem);
				continue;
			}

			if (!rss_print_article(pdigest->bnum, 
						boardname, fh, &pub, &edit))
				rss_failed("本文不存在或已被删除。", 0);
			else
				count++;
			size += sizeof(struct digestitem);
			lastbnum = pdigest->bnum;
		}
		mmapfile(NULL, &dirmf);
	} MMAP_CATCH{
	} MMAP_END mmapfile(NULL, &mf);
	if (!count)
		rss_failed("当前栏目没有文章。", 1);
	edit = edit < pub ? pub : edit;
	printf("<pubDate>");
	rss_date(Ctime((time_t)pub));
	printf("</pubDate><lastBuildDate>");
	rss_date(Ctime((time_t)edit));
	printf("</lastBuildDate>");
	rss_end();
	return;
}

static void 
rss_show_board (struct boardmem *x, int mode){
	char boardname[24], path[STRLEN];
	char channeltitle[STRLEN], channelurl[STRLEN]; 
	struct mmapfile dirmf = {ptr: NULL};
	struct fileheader *fh;
	int i = 0, bnum = getbnumx(x), count = 0, pub = 0, edit = 0;

	strncpy(boardname, x->header.filename, sizeof(boardname));
	sprintf(path, MY_BBS_HOME "/boards/%s/.DIR", boardname);

	sprintf(channeltitle, "%s - %s", 
			x->header.title, x->header.filename);
	sprintf(channelurl, "doc?B=%d&M=%d&redirect=1", 
			getbnumx(x), mode);
	
	rss_head(channeltitle, channelurl, getboardaux(bnum)->intro);

	if (mmapfile(path, &dirmf) < 0)
		rss_failed("无法读取版面文章列表。", 1);
	if (dirmf.size == 0)
		rss_failed("当前版面没有文章。", 2);

	MMAP_TRY{
		while ((i + 1) * sizeof(struct fileheader) <= dirmf.size 
				&& count < RSS_MAX_BOARD_ITEM) {
			fh = (struct fileheader *) (dirmf.ptr + dirmf.size - 
					(i + 1) * sizeof (struct fileheader));

			if (mode && fh->filetime != fh->thread) {
				i++;
				continue;
			}

			if (!rss_print_article(bnum, 
						boardname, fh, &pub, &edit))
				rss_failed("本文不存在或已被删除。", 0);
			else
				count++;
			i++;
		}
	} MMAP_CATCH{
	} MMAP_END mmapfile(NULL, &dirmf);

	edit = edit < pub ? pub : edit;
	printf("<pubDate>");
	rss_date(Ctime((time_t)pub));
	printf("</pubDate><lastBuildDate>");
	rss_date(Ctime((time_t)edit));
	printf("</lastBuildDate>");
	rss_end();
}

int
bbsrss_main(void)
{
	char rssid[32], sec[4];
	struct boardmem *x;
	int mode = atoi(getparm("m"));

	strsncpy(rssid, getparm("rssid"), sizeof (rssid));
	strsncpy(sec, getparm("sec"), sizeof(sec));

	if (strcmp(rssid, "digest") == 0 && sec) {
		bbsrss_header("gb2312");
		rss_show_digest(sec[0]);
		return 0;
	} else if ((x = getboard(rssid))) {
		bbsrss_header("gb2312");
		rss_show_board(x, mode);
		return 0;
	} else {
		bbsrss_header("gb2312");
		rss_head("RSS 系统", "boa?secstr=&redirect=1",
			MY_BBS_DOMAIN " RSS 系统");
		rss_failed(NULL, 1);
	}
	return 0;
}
