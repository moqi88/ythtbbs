#include "bbslib.h"

int
bbsboa_main()
{
	//struct boardmem *x;
	//int i, total = 0;
	char *secstr;
	char ydq[3];
	const struct sectree *sec;
	struct brcinfo *brcinfo;

	brcinfo = brc_readinfo(currentuser->userid);

	secstr = getparm("secstr");
	sec = getsectree(secstr);

	if (secstr[0] != '*' && !strcmp(sec->basestr, brcinfo->lastsec)) {
		if (cache_header
		    (max(thisversion, file_time(MY_BBS_HOME "/wwwtmp")), 120))
			return 0;
	}

	if (secstr[0] != '*' && strcmp(sec->basestr, brcinfo->lastsec)) {
		strsncpy(brcinfo->lastsec, sec->basestr,
			 sizeof (brcinfo->lastsec));
		brc_saveinfo(currentuser->userid, brcinfo);
	}

	html_header(1);
	printf("<title>%s %s</title>",MY_BBS_NAME, BBS_WWW_INTRO);
	//check_msg();
	printf("<style type=text/css>A {color: #0000f0}</style>");
	printf("<script src=" BBSJS "></script>\n");
	printf("<script src=" BBSBOAJS "></script>\n");
	changemode(SELECT);
	printf("<script>checkFrame(%d);</script>", 
			atoi(getparm("redirect")));
	printf("</head>"); 

	if (secstr[0] == '*') {
		printf("<body>\n<div id='head'>\n");
		showsechead(&sectree);
		printf("</div>\n");
               if (secstr[1]=='0') {
                       printf("<h1>预定讨论区总览</h1>\n<hr noshade>");
               } else {
                       printf("<h1>预定文件夹总览</h1>\n<hr noshade>");
               }
               sprintf(ydq,"%s",secstr);
               showboardlist(ydq, 0);
		printf("<hr noshade>");
		return 0;
	}
	printf("<body topmargin=0 leftMargin=1 MARGINWIDTH=1 MARGINHEIGHT=0>" WWWLEFT_DIV);
	showsecpage(sec);
	printf(WWWFOOT_DIV "</body></html>");
	return 0;
}

int
showsecpage(const struct sectree *sec)
{
	FILE *fp;
	char buf[1024], *param, *ptr;
	int init_digest = 1;
	sprintf(buf, "wwwtmp/secpage.sec%s", sec->basestr);
	fp = fopen(buf, "rt");
	if (!fp)
		return showdefaultsecpage(sec);
	while (fgets(buf, sizeof (buf), fp)) {
		if (buf[0] != '#') {
			fputs(buf, stdout);
			continue;
		}
		ptr=strtok(buf, " \t\r\n");
		param=strtok(NULL, " \t\r\n");
		if (!strcmp(ptr, "#showfile")&&param) {
			showfile(param);
		} else if (!strcmp(ptr, "#showblist")) {
			showboardlist(sec->basestr, 0);
		} else if (!strcmp(ptr, "#showsecintro"))
			showsecintro(sec);
		else if (!strcmp(ptr, "#showsecnav"))
			showsecnav(sec);
		else if (!strcmp(ptr, "#showstarline")&&param)
			showstarline(param);
		else if (!strcmp(ptr, "#showhotboard")&&param)
			showhotboard(sec, param);
		else if (!strcmp(ptr, "#showsechead"))
			showsechead(sec);
		else if (!strcmp(ptr, "#showsecmanager"))
			showsecmanager(sec);
		else if(!strcmp(ptr,"#showboardlistscript"))
			showboardlistscript(sec->basestr);
		else if(!strcmp(ptr, "#showsecdigest")&&param)
			showsecdigest(sec, param, &init_digest);
		else if(!strcmp(ptr, "#adjustdigest")&&param)
			digestadjust(param);
		else if(!strcmp(ptr, "#showExLinks"))
			showExLinks(sec, 10);
		else if(!strcmp(ptr, "#showSecTop10"))
			showSecTop10(sec);
		else if(!strcmp(ptr, "#showmyposts"))
			showmyposts(currentuser->userid);
		else if(!strcmp(ptr, "#showspecialall"))
			showspecialall(sec);
		else if(!strcmp(ptr, "#showadv")&&param)
			showadv(param);
	}
	fclose(fp);
	return 0;
}

int
showdefaultsecpage(const struct sectree *sec)
{
	printf("<div id=\"head\">\n");
	showsechead(sec);
	printf("</div>\n<div class=\"clear\"></div>");
	//printf("<h2>%s</h2>", nohtml(sec->title));
	//printf("<div align=right>");
	//showsecmanager(sec);
	//printf("</div>");
	printf("<div id=\"boa_nav\">"
			"<a id=\"show_right\" href=\"#\" "
			"onclick=\"document.getElementById('main_right').style.display='';"
			"document.getElementById('main_left').style.width='78%%';\" "
			"style=\"display: none;\">显示右侧菜单</a></div>");
	printf("<hr noshade>");
	printf("<div id=\"main\">\n\t<div id=\"main_left\">");
	if (showsecintro(sec) == 0)
		printf("<div class=\"clear\"></div>");
	showboardlist(sec->basestr, 0);
	printf("</div><div id=\"main_right\" style=\"display: '';\">\n"
			"<div class=\"annbox\" align=\"center\">"
			"<a href=\"#\" title=\"隐藏右侧菜单\" onclick=\""
			"document.getElementById('main_right').style.display='none';"
			"document.getElementById('main_left').style.width='98%%';\">"
			"隐藏右侧菜单</a></div>\n");
	printf(	"<div id=\"hotboard\" class=\"annbox\">\n"
			"<h2>热门版面</h2>\n");
	showhotboard(sec, "10");
	printf("</div>\n<div class=\"annbox\">\n"
			"<h2>所有版面</h2>\n");
	showboardlist(sec->basestr, 1);
	printf("</div>\n</div>\n");
	printf("<div class=\"clear\"><hr noshade></div>");
	//showSecTop10(sec);
	return 0;
}

int
showsechead(const struct sectree *sec)
{
	int i, pos;

	printf("\n<script language=\"javascript\">\n"
			"\tvar sectree = new Array(\n"
			"\t\tnew init_sec_tree(0, \"\", \"导读首页\", %d), \n", 
			sectree.nsubsec);
	pos = 0;
	for (i = 1; i <= sectree.nsubsec; i++) {
		if (sectree.subsec[i - 1] == sec)
			pos = i;
		printf("\t\tnew init_sec_tree(%d, \"%s\", \"%s\", %d), \n", 
				i, sectree.subsec[i - 1]->basestr, 
				nohtml(sectree.subsec[i - 1]->title), 
				sectree.subsec[i - 1]->nsubsec);
	}
	printf("\t\tnew init_sec_tree(-1, \".\", \".\", 0)\n\t);\n"
			"\tshow_sec_head(%d);\n</script>\n\n", pos);
	/*printf("<table border=0 class=colortb1><tr>");
	if (sec == &sectree)
		printf("<td align=center class=f1><b>首页导读</b>&nbsp;</td>");
	else
		printf
		    ("<td align=center class=f1><a href=boa?secstr=? class=blk>%s</a>&nbsp;</td>",
		     nohtml(sectree.title));
	ntr = sectree.nsubsec / 9 + 1;
	for (i = 0; i < sectree.nsubsec; i++) {
		sec1 = sectree.subsec[i];
		if (sec1 == sec)
			printf
			    ("<td align=center class=f1><b>%s</b>&nbsp;</td>",
			     nohtml(sec1->title));
		else if (sec1 == sec2)
			printf
			    ("<td align=center class=f1><b><a href=boa?secstr=%s class=blk>%s</a></b>&nbsp;</td>",
			     sec1->basestr, nohtml(sec1->title));
		else
			printf
			    ("<td align=center class=f1><a href=boa?secstr=%s class=blk>%s</a>&nbsp;</td>",
			     sec1->basestr, nohtml(sec1->title));
		if (i != sectree.nsubsec - 1
		    && (i + 2) % (sectree.nsubsec / ntr + 1) == 0)
			printf("</tr><tr>");
	}
	printf("</tr></table>");*/
	return 0;
}

int
showstarline(char *str) {
	printf("<tr><td class=tb2_blk><font class=star>★</font>"
	       "&nbsp;%s</td></tr>", str);
	return 0;
}

int
showExLinks(const struct sectree *sec, int num)
{
	char bname[30], buf[256], *line, *ptr;
	struct boardmem *x1;
	FILE *fp;
	char sitename[32], url[STRLEN], logo[32];
	
	if(sec == &sectree || !strcmp(sec->basestr,"0"))
		strcpy(bname, "sysop");
	else
		sprintf(bname, "%sadmin", sec->basestr);
	
	x1 = getboard2(bname);
	if(!x1 || !x1->wwwlink)
		return 0;
	sprintf(buf, MY_BBS_HOME "/ftphome/root/boards/%s/link/config.ini", bname);
	fp = fopen(buf, "r");
	if(fp == NULL)
		return 0;
	printf("<script language=javascript>\n\tvar links = new Array(\n");
	while(fgets(buf, sizeof(buf), fp)) {
		if(strchr("\r\n#", buf[0]))
			continue;
		line = buf;
		ptr = strchr(line, '\t');
		if(ptr) {
			*ptr = 0;
			strncpy(sitename, line, sizeof(sitename)-1);
		} else
			continue;
		line = ++ptr;
		ptr = strchr(line, '\t');
		if(ptr) {
			*ptr = 0;
			strncpy(url, line, sizeof(url)-1);
		} else
			continue;
		line = ++ptr;
		ptr = strchr(line, '\r');  // in case of windows format \r\n
		if(ptr == NULL)
			ptr = strchr(line, '\n');
		if(ptr)
			*ptr = 0;
		strncpy(logo, line, sizeof(logo)-1);

		printf("\tnew aLink('%s', '%s', '%s'),\n", sitename, url, logo);
	}
	printf("\tnew aLink(-1, -1, -1)\n);\n");
	printf("printExLinks(%d, %d);\n", getbnumx(x1), num);
	printf("</script>");
	fclose(fp);
	return 0;
}

int
showsecnav(const struct sectree *sec)
{
	char buf[256];
	printf("<table width=100%%>");
	sprintf(buf,
		"近日精彩话题推荐 &nbsp;(<a href=bbsshownav?secstr=%s class=blk>"
		"查看全部</a>)", sec->basestr);
	showstarline(buf);
	printf("<tr><td>");
	shownavpart(0, sec->basestr);
	printf("</td></tr></table>");
	return 0;
}

int
genhotboard(struct hotboard *hb, const struct sectree *sec, int max)
{
	int count = 0, i, j, len;
	struct boardmem *bmem[MAXHOTBOARD], *x, *x1;
	if (max < 3 || max > MAXHOTBOARD)
		max = 10;
	if (max == hb->max && hb->uptime > shm_bcache->uptime)
		return hb->count;
	len = strlen(sec->basestr);
	for (i = 0; i < MAXBOARD && i < shm_bcache->number; i++) {
		x = &(shm_bcache->bcache[i]);
		if (x->header.filename[0] <= 32 || x->header.filename[0] > 'z')
			continue;
		if (hideboard_x(x))
			continue;
		if (strncmp(sec->basestr, x->header.sec1, len) &&
		    strncmp(sec->basestr, x->header.sec2, len))
			continue;
		for (j = 0; j < count; j++) {
			if (x->score > bmem[j]->score)
				break;
			if (x->score == bmem[j]->score
			    && x->inboard > bmem[j]->inboard)
				break;
		}
		for (; j < count; j++) {
			x1 = bmem[j];
			bmem[j] = x;
			x = x1;
		}
		if (count < max)
			bmem[count++] = x;
	}
	for (i = 0; i < count; i++) {
		strsncpy(hb->bname[i], bmem[i]->header.filename,
			 sizeof (hb->bname[0]));
		strsncpy(hb->title[i], bmem[i]->header.title,
			 sizeof (hb->title[0]));
		hb->bnum[i] = getbnumx(bmem[i]);
	}
	hb->max = max;
	hb->count = count;
	hb->uptime = now_t;
	hb->sec = sec;
	return count;
}

int
showhotboard(const struct sectree *sec, char *s)
{
	static struct hotboard *(hbs[50]);
	static int count = 0;
	struct hotboard *hb = NULL;
	int i;
	for (i = 0; i < count; i++) {
		if (hbs[i]->sec == sec) {
			hb = hbs[i];
			break;
		}
	}
	if (!hb) {
		if (count >= 50)
			return -1;
		hbs[count] = calloc(1, sizeof (struct hotboard));
		hb = hbs[count];
		count++;
	}
	genhotboard(hb, sec, atoi(s));
	for (i = 0; i < hb->count; i++) {
		printf("<li><a href=\"/HT/home?B=%d\" title=\"%s\">"
				"%s</a></li>\n",
		       hb->bnum[i], void1(nohtml(hb->title[i])), 
		       void1(nohtml(hb->title[i])));
	}
	return 0;
}

int
showfile(char *fn)
{
	struct mmapfile mf = { ptr:NULL };
	int retv = 0;
	MMAP_TRY {
		if (mmapfile(fn, &mf) < 0) {
			MMAP_RETURN(-1);
		}
		retv = fwrite(mf.ptr, 1, mf.size, stdout);
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);
	return retv;
}

int
showsecintro(const struct sectree *sec)
{
	char filename[80];
	int i;
	if (!sec->introstr[0])
		return -1;
	sprintf(filename, "wwwtmp/lastmark.js.sec%s", sec->basestr);
	printf("<script>");
	if (showfile(filename) < 0) {
		printf("</script>\n");
		printf("目前尚没有分类讨论区 Mark 文章列表生成。<br>"
			"请检查 ~/wwwtmp/lastmark/ 目录是否已建立。<br>"
			"或检查 searchLastMark 及 printSecLastMark 是否已加入 "
			"crontab 列表。<br>"
			"下面是可以选择的分区：<br>");
		for (i = 0; i < sec->nsubsec; i++) {
			printf
			    ("<li><a href=/HT/bbsboa?secstr=%s>%s</a>",
			     sec->subsec[i]->basestr, sec->subsec[i]->title);
		}
		printf("如果不能按照上面的提示正确配置，并且不希望继续看到本栏目，请在 "
				"~/wwwtmp/secpage.sec 中去掉 #showsecintro 行。");
	} else
		printf("</script>\n");

	return 0;
}

int
showboardlistscript(const char *secstr)
{
#if 0
	printf("<script src=boardlistscript?secstr=%s></script>", secstr, secstr);
	return 1;
#else
  char ydq[3];
  sprintf(ydq,"%s",secstr);
	struct boardmem *(data[MAXBOARD]);
	int total;
	if (secstr[0] == '*') {
		total=listmybrd(data);
	} else {
		//const struct sectree *sec=getsectree(secstr);
		total=makeboardlist(getsectree(secstr), data);
	}
	printf("<script>var boardlistscript=");
	boardlistscript(data, total);
	printf("</script>");
	return total;
#endif
}

int
showboardlist(const char *secstr, int mode)
{
	//char *cgi = "home", *ptr;
	char var[20];
	int total;
	if(secstr[0] == '*')
		strcpy(var, "boardlistmybrd");
	else
		snprintf(var, sizeof(var), "boardlist%s", secstr); 
	total = showboardlistscript(secstr);
	if(total == 0)
		return 0;
	printf("<script>var %s=boardlistscript;\n", var);
	if(mode == 0)
		printf("fullBoardList(%s);</script>\n", var);
	else
		printf("boardIndex(%s);</script>\n", var);
	return total;
}

int
showsecmanager(const struct sectree *sec)
{
	struct secmanager *secm;
	int i;
	if (!sec->basestr[0] || !(secm = getsecm(sec->basestr)) || !secm->n)
		return -1;
	printf("区长:");
	for (i = 0; i < secm->n; i++) {
		printf(" <a href=/HT/qry?U=%s>%s</a>", secm->secm[i],
		       secm->secm[i]);
	}
	return 0;
}

int showSecTop10(const struct sectree *sec) {
	char path[256];
	struct mmapfile mf = { ptr:NULL };
	int retv=0;

	if(sec->parent == NULL)
		return 0;
	sprintf(path, "wwwtmp/%c_topten", sec->basestr[0]);

	MMAP_TRY {
		if (mmapfile(path, &mf) < 0) {
			MMAP_RETURN(-1);
		}
		retv = fwrite(mf.ptr, 1, mf.size, stdout);
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);
	return retv;				
}
	
int showmyposts(char *id) {
	char buf[11][1024], logname[STRLEN], title[STRLEN], boardname[STRLEN], *line, *ptr;
	int filetime, i = 0, total;
	FILE *fp;

	if (!strcmp(id, "guest"))
		return 0;
	sethomefile(logname, id, "wwwnewpost");

	printf("<div id='my_post_box'>\n");
	printf("<h2>我最新发表的十篇文章</h2>");
	if (NULL != (fp = fopen(logname, "r"))) {
		while (fgets(buf[10], STRLEN, fp)) {
			if (strchr("\r\n", buf[10][0]))
				continue;
			line = buf[10];
			ptr = strchr(line, '\t');
			if (ptr) {
				*ptr = 0;
				filetime = atoi(line);
			} else
				continue;
			line = ++ptr;
			ptr = strchr(line, '\t');
			if (ptr) {
				*ptr = 0;
				strncpy(boardname, line, STRLEN - 1);
			} else
				continue;
			line = ++ptr;
			ptr = strchr(line, '\n');
			if (ptr)
				strncpy(title, line, STRLEN - 1);
			else
				continue;
			if (hideboard(boardname))
				continue;
			sprintf(buf[i], "<li><a href=\"/HT/con_%d_M.%d.A.htm\""
					" title=\"%s\" target=\"_self\">%s</a> "
					"[<a href=\"/HT/home?B=%d\" title\"%s\">"
					"%s</a> %s]", 
					getbnum(boardname), filetime, 
					title, title, 
					getbnum(boardname), boardname, 
					boardname, Ctime(filetime));
			i++;
		}
		fclose(fp);
		total = i;
		printf("<div id=\"my_post_box_left\">");
		for (i = 0; i < total; i++) {
			printf("%s\n", buf[i]);
			if (i == total / 2 - 1)
				printf("</div><div id=\"my_post_box_right\">");
		}
		printf("</div>\n");
	} else {
		printf("<div align=\"center\">没有发现您的发文"
			"记录，欢迎到感兴趣的版面挥毫泼墨泼墨！</div>\n");
	}
	printf("</div>\n<div class=\"clear\"></div><hr noshade />\n");

	return 1;
}

int
showadv(char *parm) {
	int pos = atoi(parm);
	if (pos == 1 && !bbsadv_show(pos))
		showsechead(NULL);
	return 1;
}
