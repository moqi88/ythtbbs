/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    Firebird Bulletin Board System
    Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
                        Peng Piaw Foong, ppfoong@csie.ncu.edu.tw

    Copyright (C) 1999, KCN,Zhou Lin, kcn@cic.tsinghua.edu.cn
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include "bbs.h"
#include "bbsgopher.h"
#include "bbstelnet.h"

#define PATHLEN         1024
//modified by ylsdd #define A_PAGESIZE      (t_lines - 5)
#define A_PAGESIZE      (t_lines - 4)
#define ADDITEM         0
#define ADDGROUP        1
#define ADDMAIL         2
#define ADDGOPHER       3


extern void a_prompt();		/* added by netty */
extern int can_R_endline;
typedef struct {
	char title[72];		//题目
	char fname[80];		//文件名
	char *host;		//所有人
	int port;
} ITEM;

int a_fmode = 1;
int copymode;
int inlink = 0;

typedef struct {
	ITEM *item[MAXITEMS];
	char mtitle[STRLEN];
	char *path;
	int num, page, now;
	int level;
} MENU;


void a_menu();
static int chk_currBM_Personal(char *BMstr, struct boardmem *bp);
static int a_journal(MENU *pm);
static void hsprintfLite(char *s, const char *s0);
static int encodeMIME(char *src, char *dest);


static void
freeitem(MENU * me)
{
	int i;
	for (i = 0; i < me->num; i++)
		free(me->item[i]);
	return;
}

int
valid_fname(str)	//判断是否为合法的文件名
char *str;
{
	char ch;

	while ((ch = *str++) != '\0') {
		if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
		    strchr("0123456789-_", ch) != NULL))
				return 0;
	}
	return 1;
}

static void
a_showmenu(pm)		//显示pm指向的menu
MENU *pm;
{
	struct stat st;
	struct tm *pt;
	char title[STRLEN * 2], *kind;
	char fname[STRLEN];
	char ch, *ptr;
	char buf[PATHLEN], pathbuf[PATHLEN];
	time_t mtime;
	int n;
	int visit[2];
	int overlen;

	clear();
	if (chkmail()) {
		prints("\033[1;5;36m");
		sprintf(pathbuf, "[您有信件,请按 w 查看信件]");
	}
	else {
		prints("\033[1;37m");
		strcpy(pathbuf, pm->mtitle);
	}
	sprintf(buf, "%*s", (int) (80 - strlen(pathbuf)) / 2, "");
	prints("\033[1;44m%s%s%s\033[m\n", buf, pathbuf, buf);

	if (!(ptr = strstr(pm->path, "Personal_Corpus")))
		ptr = strstr(pm->path, "groups/GROUP_");
	if (ptr && countstr(ptr, "/") > 3) {	// 深度大于2的目录不显示浏览统计
		prints("\033[1;31m[%s]\033[m \033[1;32mF\033[37m 寄回信箱"
		       "\033[32m↑↓\033[37m移动 \033[32m→ r \033[37m"
		       "读取 \033[32m← q\033[37m 离开 \033[1;32mh\033[37m 求助\n",
		       (pm->level & PERM_BOARDS) ? "版  主" : "功能键");
	} else {
		getvisit(visit, pm->path);
		prints("\033[1;31m[%s]\033[m \033[1;32mF\033[37m 寄回信箱"
		       "\033[32m↑↓\033[37m移动 \033[32m→ r \033[37m读取 \033[32m"
		       "← q\033[37m 离开 \033[1;32mh\033[37m 求助 \033[33m浏览统计：%4d次%4d%s\n",
		       (pm->level & PERM_BOARDS) ? "版  主" : "功能键",
		       visit[0],
		       (visit[1] >
			100000) ? (visit[1] / 3600) : (visit[1] / 60),
		       (visit[1] > 100000) ? "时" : "分");
	}

	prints("\033[1;44;37m 编号 %-45s 整  理           %8s \033[m",
	       "[类别] 标    题", (a_fmode == 2
				   && (pm->level & PERM_BOARDS) !=
				   0) ? "档案名称" : "编辑日期");
	prints("\n");
	if (pm->num == 0)
		prints("      << 目前没有文章 >>\n");
	for (n = pm->page; n < pm->page + A_PAGESIZE && n < pm->num; n++) {
		overlen = 0;
		strsncpy(title, pm->item[n]->title, sizeof (title));
		if (a_fmode) {
			snprintf(fname, STRLEN, "%s", pm->item[n]->fname);
			if (snprintf(pathbuf, PATHLEN, "%s/%s", pm->path, fname)
			    > PATHLEN - 1)
				overlen = 1;
			/*加上对level的检测, 使得出到非自己的管辖区时看不到文件名 ylsdd */
			if (a_fmode == 2 && (pm->level & PERM_BOARDS) != 0) {
				ch = (dashf(pathbuf) ? ' '
				      : (dashd(pathbuf) ? '/' : ' '));
				fname[10] = '\0';
			} else {
				if (dashf(pathbuf) || dashd(pathbuf)) {
					stat(pathbuf, &st);
					mtime = st.st_mtime;
				} else
					mtime = time(0);

				pt = localtime(&mtime);
				sprintf(fname,
					"\033[1m%04d\033[m.\033[1m%02d\033[m.\033[1m%02d\033[m",
					1900 + pt->tm_year, pt->tm_mon + 1,
					pt->tm_mday);
				ch = ' ';
			}
			if (overlen) {
				kind = "[\033[1;32m太深\033[m]";
			} else if (pm->item[n]->host != NULL) {
				if (pm->item[n]->fname[0] == '0')
					kind = "[\033[1;32m连文\033[m]";
				else
					kind = "[\033[1;33m连目\033[m]";
			} else if (dashf(pathbuf)) {
				if (dashl(pathbuf))
					kind = "[\033[1;36m连文\033[m]";
				else
					kind = "[\033[1;36m文件\033[m]";
			} else if (dashd(pathbuf)) {
				if (dashl(pathbuf))
					kind = "[\033[1m连目\033[m]";
				else
					kind = "[\033[1m目录\033[m]";
			} else {
				kind = "[\033[1;32m错误\033[m]";
			}
			if (!strncmp(title, "[目录] ", 7)
			    || !strncmp(title, "[文件] ", 7)
			    || !strncmp(title, "[连目] ", 7)
			    || !strncmp(title, "[连文] ", 7))
				sprintf(pathbuf, "%-s %-55.55s%-s%c", kind,
					title + 7, fname, ch);
			else
				sprintf(pathbuf, "%-s %-55.55s%-s%c", kind,
					title, fname, ch);
			strncpy(title, pathbuf, STRLEN * 2);
			title[STRLEN * 2 - 1] = '\0';
		}
		prints("  %3d %s\n", n + 1, title);
	}
	clrtobot();
	move(t_lines - 1, 0);
	update_endline();
}

static void
a_additem(pm, title, fname, host, port) //添加一个新项目
MENU *pm;
char *title, *fname, *host;
int port;
{
	ITEM *newitem;
	char *ptr;

	if (countstr(pm->path, "/") < countstr(fname, ".."))	//在path中"/"比".."少，说明在上层目录
		return;
	ptr = fname;
	while (*ptr) {	//在有非法字符时return
		if (strchr(";|,$%^&*(!@=+\\/[]", *ptr) != NULL)
			return;
		ptr++;
	}

	if (pm->num < MAXITEMS) {
		newitem = (ITEM *) malloc(sizeof (ITEM));
		strsncpy(newitem->title, title, sizeof (newitem->title));
		if (host != NULL) {
			newitem->host =
			    (char *) malloc(sizeof (char) * (strlen(host) + 1));
			strcpy(newitem->host, host);
		} else
			newitem->host = host;
		newitem->port = port;
		strsncpy(newitem->fname, fname, sizeof (newitem->fname));
		pm->item[(pm->num)++] = newitem;
	}
}

int
countstr(char *s0, char *s1)	//检索s0中出现s1的次数
{
	int i = 0, j;
	char *ptr = s0;
	j = strlen(s1);
	while ((ptr = strstr(ptr, s1)) != NULL) {
		i++;
		ptr += j;
	}
	return i;
}

static int
a_loadnames(pm)
MENU *pm;
{
	FILE *fn;
	ITEM litem;
	char buf[PATHLEN], *ptr;
	char hostname[STRLEN];

	pm->num = 0;
	if (snprintf(buf, PATHLEN, "%s/.Names", pm->path) > PATHLEN - 1)
		return 0;
	if ((fn = fopen(buf, "r")) == NULL)
		return 0;
	hostname[0] = '\0';
	while (fgets(buf, sizeof (buf), fn) != NULL) {
		if ((ptr = strchr(buf, '\n')) != NULL)
			*ptr = '\0';
		if (strncmp(buf, "Name=", 5) == 0) {
			bzero(litem.title, sizeof (litem.title));	/*add by ylsdd */
			strncpy(litem.title, buf + 5, 72);
			litem.title[71] = '\0';
		} else if (strncmp(buf, "Path=", 5) == 0) {
			if (strncmp(buf, "Path=~/", 7) == 0)
				strncpy(litem.fname, buf + 7, 80);
			else
				strncpy(litem.fname, buf + 5, 80);
			litem.fname[79] = '\0';
			if (((!strstr(litem.title + 38, "(BM: BMS)")
			      || USERPERM(currentuser, PERM_BOARDS))
			     && (!strstr(litem.title + 38, "(BM: SYSOPS)")
				 || USERPERM(currentuser, PERM_SYSOP))
			     && (strstr(litem.title, "<HIDE>") != litem.title))
			    || (pm->level & PERM_BOARDS)) {	/*modified by ylsdd */
				if (strstr(litem.fname, "!@#$%")) {
					char *ptr1, *ptr2, gtmp[STRLEN];

					strcpy(gtmp, litem.fname);
					ptr1 = strtok(gtmp, "!#$%@");
					strcpy(hostname, ptr1);
					ptr2 = strtok(NULL, "@");
					strcpy(litem.fname, ptr2);
					litem.port = atoi(strtok(NULL, "@"));
				}
				a_additem(pm, litem.title, litem.fname,
					  (strlen(hostname) ==
					   0) ? NULL : hostname, litem.port);
			}
			hostname[0] = '\0';
		} else if (strncmp(buf, "# Title=", 8) == 0) {
			if (pm->mtitle[0] == '\0') {
				strsncpy(pm->mtitle, buf + 8,
					 sizeof (pm->mtitle));
			}
		} else if (strncmp(buf, "Host=", 5) == 0) {
			strncpy(hostname, buf + 5, STRLEN);
			hostname[STRLEN - 1] = 0;
		} else if (strncmp(buf, "Port=", 5) == 0) {
			litem.port = atoi(buf + 5);
		}
	}
	fclose(fn);
	return 1;
}

static void
a_savenames(pm)
MENU *pm;
{
	FILE *fn;
	ITEM *item;
	char fpath[PATHLEN];
	int n;

	if (snprintf(fpath, PATHLEN, "%s/.Names", pm->path) > PATHLEN - 1)
		return;
	if ((fn = fopen(fpath, "w")) == NULL)
		return;
	fprintf(fn, "#\n");
	if (!strncmp(pm->mtitle, "[目录] ", 7)
	    || !strncmp(pm->mtitle, "[文件] ", 7)
	    || !strncmp(pm->mtitle, "[连目] ", 7)
	    || !strncmp(pm->mtitle, "[连文] ", 7)) {
		fprintf(fn, "# Title=%s\n", pm->mtitle + 7);
	} else {
		fprintf(fn, "# Title=%s\n", pm->mtitle);
	}
	fprintf(fn, "#\n");
	for (n = 0; n < pm->num; n++) {
		item = pm->item[n];
		if (!strncmp(item->title, "[目录] ", 7)
		    || !strncmp(item->title, "[文件] ", 7)
		    || !strncmp(item->title, "[连目] ", 7)
		    || !strncmp(item->title, "[连文] ", 7)) {
			fprintf(fn, "Name=%s\n", item->title + 7);
		} else
			fprintf(fn, "Name=%s\n", item->title);
		if (item->host != NULL) {
			fprintf(fn, "Host=%s\n", item->host);
			fprintf(fn, "Port=%d\n", item->port);
			fprintf(fn, "Type=1\n");
			fprintf(fn, "Path=%s\n", item->fname);
		} else
			fprintf(fn, "Path=~/%s\n", item->fname);
		fprintf(fn, "Numb=%d\n", n + 1);
		fprintf(fn, "#\n");
	}
	fclose(fn);
	chmod(fpath, 0664);
}

void
a_prompt(bot, pmt, buf, len)
int bot;
char *pmt, *buf;
int len;
{
	move(t_lines + bot, 0);
	clrtoeol();
	getdata(t_lines + bot, 0, pmt, buf, len, DOECHO, YEA);
}

/* added by netty to handle post saving into (0)Announce */
int
a_Save(path, key, fileinfo, nomsg)
char *path, *key;
struct fileheader *fileinfo;
int nomsg;
{

	char board[80];
	int ans = NA;
	FILE *fps, *fpd;
	char line[200];
	int fs1, fs2;
	if (!nomsg) {
		sprintf(genbuf, "确定将 [%-.40s] 存入暂存档吗",
			fileinfo->title);
		if (askyn(genbuf, YEA, YEA) == NA)
			return FULLUPDATE;
	}
	setuserfile(board, "tmpsave");
	if (dashf(board)) {
		if (nomsg)
			ans = YEA;
		else
			ans = askyn("要附加在旧暂存档之后吗", YEA, YEA);
	}
	if (in_mail) {
		sprintf(genbuf,
			"mail/%c/%s/%s",
			mytoupper(currentuser->userid[0]), currentuser->userid,
			fh2fname(fileinfo));
	} else {
		sprintf(genbuf, "boards/%s/%s", key, fh2fname(fileinfo));
	}
	if (ans) {
		fs1 = file_size(genbuf);
		fs2 = file_size(board);
		if (fs1 + fs2 > 2000000) {
			if (nomsg)
				return FULLUPDATE;
			a_prompt(-1,
				 "暂存档容量不足，添加失败，请按<Enter>继续...",
				 genbuf, 2);
			return FULLUPDATE;
		}
	}
	fps = fopen(genbuf, "r");
	if (fps == NULL)
		return FULLUPDATE;
	fpd = fopen(board, (ans) ? "a+" : "w");
	if (fpd == NULL) {
		fclose(fps);
		return FULLUPDATE;
	}
	keepoldheader(fps, SKIPHEADER);
	fputs
	    ("\033[1;36m───────────────────────────────────────\033[0m\n",
	     fpd);
	fprintf(fpd, "\033[1;32m 作者 %-14s 时间 %-52s\033[0m\n",
		fh2owner(fileinfo), Ctime(fileinfo->filetime));
	fputs
	    ("\033[1;36m───────────────────────────────────────\033[0m\n",
	     fpd);
	while (fgets(line, sizeof (line), fps) != NULL) {
		if (transferattach(line, sizeof (line), fps, fpd))
			continue;
		if (!strcmp(line, "--\n") || !strcmp(line, "--\r\n"))
			break;
		if (!strncmp(line, ": : ", 4))
			continue;
		fputs(line, fpd);
	}
	fclose(fps);
	fclose(fpd);
	if (!nomsg)
		a_prompt(-1, "已将该文章存入暂存档, 请按<Enter>继续...", genbuf,
			 2);
	return FULLUPDATE;
}

/* added by netty to handle post saving into (0)Announce */

char Importname[PATHLEN];

int
check_import(char *anboard)
{
	struct boardmem *bp;
	char *ptr;
	if (!strncmp(Importname, "0Announce/groups/GROUP_", 23)) {
		strncpy(anboard, &(Importname[25]), STRLEN);
		anboard[STRLEN - 1] = 0;
		ptr = index(anboard, '/');
		if (ptr != NULL)
			*ptr = 0;
		if (!strcmp(anboard, "Personal_Corpus"))
			return 0;
		bp = getbcache(anboard);
		if (bp == NULL)
			return -1;
		if (!chk_currBM(&(bp->header), 1))
			return -2;
		return 0;
	} else if (USERPERM(currentuser, PERM_BLEVELS)) {
		strcpy(anboard, "noboard");
		return 0;
	} else
		return -2;
}

int
an_log(char *action, char *path)
{
	char anboard[STRLEN];
	strcpy(anboard, getbfroma(path));
	if (!anboard[0])
		strcpy(anboard, "noboard");
	tracelog("%s %s %s %s", currentuser->userid, action, anboard, path);
	return 0;
}

int
a_Import(direct, fileinfo, nomsg)
char *direct;
struct fileheader *fileinfo;
int nomsg;
{

	char fname[STRLEN], *ip, bname[PATHLEN];
	char ans[5], filepath[PATHLEN];
	MENU pm;
	char anboard[STRLEN], tmpboard[STRLEN];

	if (!nomsg) {
		if (select_anpath() < 0)
			return 0;
		switch (check_import(anboard)) {
		case 0:
			break;
		case -1:
			a_prompt(-1,
				 "你设定的该丝路发生内部错误,请通知系统维护",
				 ans, 2);
			return 0;
		default:
			a_prompt(-1, "你设定的该丝路已失效,请重新设置", ans, 2);
			return 0;
		}
	}

	if (!dashd(Importname)) {
		if (!nomsg)
			a_prompt(-1, "你设定的该丝路已丢失,请重新设置", ans, 2);
		return 0;
	}
	if (!fileinfo->filetime) {
		if (!nomsg)
			a_prompt(-1, "无效文件,无法放入精华区", ans, 2);
		return 0;
	}
	if ((!strcmp(currboard, anboard)) || nomsg == 2) {
		fileinfo->accessed |= FH_ANNOUNCE;
	}

	modify_user_mode(DIGEST);
	pm.path = Importname;
	strcpy(pm.mtitle, "");
	pm.level |= PERM_BOARDS;	/* add by ylsdd */
	strcpy(fname, fh2fname(fileinfo));
	if (snprintf(bname, PATHLEN, "%s/%s", pm.path, fname) > PATHLEN - 1)
		return -1;

	a_loadnames(&pm);
	ip = &fname[strlen(fname) - 1];
	while (dashf(bname)) {
		if (*ip == 'Z')
			ip++, *ip = 'A', *(ip + 1) = '\0';
		else
			(*ip)++;
		sprintf(bname, "%s/%s", pm.path, fname);
	}
	sprintf(genbuf, "%-38.38s %s ", fileinfo->title, currentuser->userid);
	a_additem(&pm, genbuf, fname, NULL, 0);
	a_savenames(&pm);
	/*if(in_mail)
	   sprintf( genbuf, "/bin/cp -r mail/%c/%s/%s %s 1>/dev/null 2>/dev/null", 
	   mytoupper(currentuser->userid[0]), currentuser->userid, fileinfo->filename , bname );
	   else
	   sprintf( genbuf, "/bin/cp -r boards/%s/%s %s 1>/dev/null 2>/dev/null", key , fileinfo->filename , bname );
	 */
	directfile(filepath, direct, fh2fname(fileinfo));
	copyfile(filepath, bname);
	if (0)
		if (!nomsg) {
			a_prompt(-1, "已将该文章放进精华区, 请按<Enter>继续...",
				 ans, 2);
		}
	if (!nomsg) {
		strcpy(tmpboard, currboard);
		strcpy(currboard, anboard);
	}
	tracelog("%s import %s %s %s", currentuser->userid, currboard,
		 fileinfo->owner, fileinfo->title);
	if (!nomsg)
		strcpy(currboard, tmpboard);
	freeitem(&pm);
	return 1;
}

static void
a_search(pm, offset)
MENU *pm;
int offset;
{
	int i;
	static char title[STRLEN];
	char ans[STRLEN], pmt[STRLEN];
	strcpy(ans, title);
	sprintf(pmt, "%s搜寻标题 [%.16s]: ", offset > 0 ? "往后" : "往前", ans);
	move(t_lines - 1, 0);
	clrtoeol();
	getdata(t_lines - 1, 0, pmt, ans, 46, DOECHO, YEA);
	if (*ans != '\0')
		strcpy(title, ans);
	for (i = pm->now + offset; (offset == 1) ? (i < pm->num) : (i >= 0);
	     i += offset) {
		strcpy(genbuf, pm->item[i]->title);
		genbuf[38] = 0;
		if (strstr(genbuf, title)) {
			pm->now = i;
			break;
		}
	}
	update_endline();
}

int
a_menusearch(path, key, level)
char *path, *key;
int level;
{
	char bname[20];
	char buf[PATHLEN];
	struct boardmem *bp;
	struct stat st;
	if (key == NULL) {
		move(t_lines, 0);
		clrtoeol();
		prints("输入讨论区名 (按空白键自动搜寻): ");
		make_blist();
		namecomplete((char *) NULL, bname);
		FreeNameList();
		setbpath(buf, bname);
		if ((*bname == '\0') || (stat(buf, &st) == -1)
		    || !(S_ISDIR(st.st_mode))) {
			move(t_lines, 0);
			clrtoeol();
			prints("不正确的讨论区.\n");
			pressreturn();
			return FULLUPDATE;
		}
		key = bname;
	}
	bp = getbcache(key);
	if (!bp)
		return 0;
	snprintf(buf, sizeof (buf), "0Announce/groups/GROUP_%c/%s",
		 bp->header.sec1[0], bp->header.filename);
	a_menu("", buf, level, 0);
	return 1;
}

static void
a_forward(path, pitem, mode)
char *path;
ITEM *pitem;
int mode;
{
	char fname[PATHLEN], *mesg;

	if (snprintf(fname, PATHLEN, "%s/%s", path, pitem->fname) < PATHLEN
	    && dashf(fname)) {
		switch (doforward(fname, pitem->title, mode)) {
		case 0:
			mesg = "文章转寄完成!\n";
			break;
		case -1:
			mesg = "System error!!.\n";
			break;
		case -2:
			mesg = "Invalid address.\n";
			break;
		default:
			mesg = "取消转寄动作.\n";
		}
		prints(mesg);
	} else {
		move(t_lines - 1, 0);
		prints("无法转寄此项目.\n");
	}
	sleep(2);
	pressanykey();
}

static void
a_newitem(pm, mode)
MENU *pm;
int mode;
{
	char uident[STRLEN];
	char board[STRLEN], title[STRLEN];
	char fname[STRLEN], fpath[PATHLEN], fpath2[PATHLEN];
	FILE *pn;
	char ans[10];
	time_t now;
	int count, oldlevel, addflag = 0;;

	pm->page = 9999;
	switch (mode) {
	case ADDITEM:
	case ADDGROUP:
		break;
	case ADDMAIL:
		setuserfile(board, "tmpsave");
		if (!dashf(board)) {
			a_prompt(-1,
				 "请先至该讨论区将文章存入暂存档, 按<Enter>继续...",
				 ans, 2);
			return;
		}
		break;
	case ADDGOPHER:
		{
			int gport;
			char ghost[STRLEN], gtitle[STRLEN], gfname[STRLEN];

			a_prompt(-2, "连线的位置：", ghost, STRLEN - 14);
			if (ghost[0] == '\0')
				return;
			a_prompt(-2, "连线的目录：", gfname, STRLEN - 14);
			if (gfname[0] == '\0')
				return;
			a_prompt(-2, "连线的Port：", ans, 6);
			if (ans[0] == '\0')
				return;
			a_prompt(-2, "标题：", gtitle, 70);
			if (gtitle[0] == '\0')
				return;
			gport = atoi(ans);
			a_additem(pm, gtitle, gfname, ghost, gport);
			a_savenames(pm);
			return;
		}
	}
	sprintf(genbuf, "请输入%s之中文名称： ",
		(mode != ADDGROUP) ? "文件" : "目录");
	a_prompt(-1, genbuf, title, 35);
	if (*title == '\0')
		return;

	time(&now);
	count = 0;

	do {
		if (count++ > MAX_POSTRETRY)
			return;
		sprintf(fname, "M%d", (int) now++);
		if (snprintf(fpath, PATHLEN, "%s/%s", pm->path, fname) >
		    PATHLEN - 1)
			return;
	} while (dashf(fpath) || dashd(fpath));

	switch (mode) {
	case ADDITEM:
		clear();
		do_delay(1);
		if (vedit(fpath, 0, YEA) == -1)
			return;
		addflag = 1;
		chmod(fpath, 0660);
		break;
	case ADDGROUP:
		mkdir(fpath, 0770);
		chmod(fpath, 0770);
		break;
	case ADDMAIL:
		crossfs_rename(board, fpath);
		break;
	}
	if (mode != ADDGROUP)
		sprintf(genbuf, "%-38.38s %s ", title, currentuser->userid);
	else {
/*Add by SmallPig*/
		if (USERPERM(currentuser, PERM_SYSOP)
		    || USERPERM(currentuser, PERM_ANNOUNCE)) {
			move(1, 0);
			clrtoeol();
/*$$$$$$$$ Multi-BM Input, Modified By Excellent $$$$$$$*/
			getdata(1, 0, "版主: ", uident, 24, DOECHO, YEA);
			if (uident[0] != '\0')
				sprintf(genbuf, "%-38.38s(BM: %s)", title,
					uident);
			else
				sprintf(genbuf, "%-38.38s", title);
		} else
			sprintf(genbuf, "%-38.38s", title);
	}

	oldlevel = pm->level;
	pm->level |= PERM_BOARDS;
	a_loadnames(pm);
	a_additem(pm, genbuf, fname, NULL, 0);
	a_savenames(pm);
	freeitem(pm);
	pm->level = oldlevel;
	a_loadnames(pm);
	an_log("additem", pm->path);
	{
		char aibuf[512];
		snprintf(aibuf, sizeof(aibuf), "3\t%s\t%s\t%s", 
			fpath, currentuser->userid, pm->item[(pm->num)-1]->title);
		ailog(aibuf);
	}
	if (mode == ADDGROUP) {
		if (snprintf(fpath2, PATHLEN, "%s/%s/.Names", pm->path, fname) <
		    PATHLEN && (pn = fopen(fpath2, "w")) != NULL) {
			fprintf(pn, "#\n");
			fprintf(pn, "# Title=%s\n", genbuf);
			fprintf(pn, "#\n");
			fclose(pn);
		}
	}
}

static void
a_moveitem(pm)
MENU *pm;
{
	ITEM *tmp;
	char newnum[STRLEN];
	int num, n;

	sprintf(genbuf, "请输入第 %d 项的新次序: ", pm->now + 1);
	a_prompt(-2, genbuf, newnum, 6);
	num = (newnum[0] == '$') ? 9999 : atoi(newnum) - 1;
	if (num >= pm->num)
		num = pm->num - 1;
	else if (num < 0)
		return;
	tmp = pm->item[pm->now];
	if (num > pm->now) {
		for (n = pm->now; n < num; n++)
			pm->item[n] = pm->item[n + 1];
	} else {
		for (n = pm->now; n > num; n--)
			pm->item[n] = pm->item[n - 1];
	}
	pm->item[num] = tmp;
	pm->now = num;
	a_savenames(pm);
	an_log("moveitem", pm->path);

}

static void a_delete(MENU *, int);
static int a_repair(MENU *);
static int a_rjunk(MENU *);

static void
a_copypaste(pm, paste)
MENU *pm;
int paste;			// -1:cut 0:copy have perm 1:paste 2:copy have no perm
{
	char title[STRLEN], filename[STRLEN], fpath[PATHLEN];
	ITEM *item;
	char newpath[PATHLEN], ans[3], realfpath[PATH_MAX + 1],
	    realnewpath[PATH_MAX + 1];
	FILE *fn;		//add by gluon for copypate in two window
	int x, y, hasc;
	MENU pmforcut;

	move(t_lines - 1, 0);
	if (paste != 1) {
		copymode = paste;
		paste = 0;
	}
	if (!paste) {
#define AN_PATH "0Announce/groups/GROUP_0/"
#define AN_PES_PATH AN_PATH "Personal_Corpus"
		item = pm->item[pm->now];
		strsncpy(title, item->title, sizeof (title));
		strsncpy(filename, item->fname, sizeof (filename));
		if (snprintf(fpath, PATHLEN, "%s/%s", pm->path, filename) >
		    PATHLEN - 1) {
			prints("档案路径过深, 无法复制");
			egetch();
			return;
		}
		if ((strlen(fpath) < sizeof (AN_PATH)
		     || !strncmp(fpath, AN_PES_PATH, sizeof (AN_PES_PATH) - 1))
		    && (USERPERM(currentuser, PERM_ANNOUNCE)
			|| USERPERM(currentuser, PERM_SYSOP)
			|| USERPERM(currentuser, PERM_OBOARDS))) {
			prints("站长不能拷贝个人文集, 如有需求, 联系系统维护");
			egetch();
			return;
		}
		//add by gluon
		sethomefile(genbuf, currentuser->userid, "copypaste");
		if ((fn = fopen(genbuf, "w+")) == NULL) {
			prints("设置档案标识出错, 请向站长报告. Thanks");
			egetch();
			return;
		}
		fwrite(title, sizeof (item->title), 1, fn);
		fwrite(filename, sizeof (item->fname), 1, fn);
		fwrite(fpath, sizeof (fpath), 1, fn);
		fwrite(&copymode, sizeof (int), 1, fn);
		fclose(fn);
		//end
		if (copymode >= 0)
			prints
			    ("档案标识完成. (注意! 如果删除或移动此档案，连目或连文将不能访问!)");
		else
			prints("档案标识完成. 粘贴后将被删除.");
		egetch();
	} else {
		//add by gluon
		sethomefile(genbuf, currentuser->userid, "copypaste");
		if ((fn = fopen(genbuf, "r")) == NULL) {
			prints("请先使用 copy 或 cut 命令再使用 paste 命令.");
			egetch();
			return;
		}
		fread(title, sizeof (item->title), 1, fn);
		fread(filename, sizeof (item->fname), 1, fn);
		fread(fpath, sizeof (fpath), 1, fn);
		fread(&copymode, sizeof (int), 1, fn);
		fclose(fn);
		//end
		if (snprintf(newpath, PATHLEN, "%s/%s", pm->path, filename) >
		    PATHLEN - 1) {
			prints("目录路径太深,无法粘贴");
			egetch();
		} else if (realpath(pm->path, realnewpath) == NULL
			   || realpath(fpath, realfpath) == NULL) {
			prints("系统内部错误,请联系系统维护!");
			egetch();
		} else if (*title == '\0' || *filename == '\0') {
			prints("请先使用 copy 或 cut 命令再使用 paste 命令. ");
			egetch();
			//add by gluon
		} else if (!(dashf(fpath) || dashd(fpath))) {
			prints("你拷贝的档案/目录不存在,可能被删除,取消粘贴.");
			egetch();
			//end
		} else if (dashf(newpath) || dashd(newpath)) {
			prints("%s 档案/目录已经存在. ", filename);
			egetch();
		} else if (strstr(newpath, fpath) != NULL
			   || strstr(realnewpath, realfpath) != NULL) {
			prints("无法将目录搬进自己的子目录中, 会造成死回圈.");
			egetch();
		} else {
			char defaultans[3];
			switch (copymode) {
			case -1:	//cut
				defaultans[0] = 'C';
				sprintf(genbuf,
					"是否粘贴: (C)粘贴 (N)取消 (C):");
				break;
			case 0:	//copy with perm
				defaultans[0] = 'C';
				sprintf(genbuf,
					"请你选择粘贴方式: (C)粘贴 (L)链接 (N)取消 (C):");
				break;
			case 2:	//copy without perm
				defaultans[0] = 'L';
				sprintf(genbuf,
					"是否创建连接: (L)链接 (N)取消 (L):");
				break;
			}
			getyx(&x, &y);
			getdata(x, y, genbuf, ans, 2, DOECHO, YEA);

			if (ans[0] != 'c' && ans[0] != 'C' && ans[0] != 'l'
			    && ans[0] != 'L' && ans[0] != 'n' && ans[0] != 'N')
				ans[0] = defaultans[0];
			hasc = 1;
			switch (ans[0]) {
			case 'N':
			case 'n':
				hasc = 0;
				break;
			case 'L':
			case 'l':
				if (copymode >= 0) {
					symlink(realfpath, newpath);
					break;
				}
			case 'C':
			case 'c':
				if (copymode == -1) {
					char oldpath[PATHLEN];
					char *ptr;
					int i;
					strncpy(oldpath, fpath, PATHLEN - 1);
					oldpath[PATHLEN - 1] = 0;
					ptr = strrchr(oldpath, '/');
					*ptr = 0;
					pmforcut.path = oldpath;
					pmforcut.mtitle[0] = 0;
					pmforcut.level |= PERM_BOARDS;
					a_loadnames(&pmforcut);
					for (i = 0; i < pmforcut.num; i++)
						if (!strcmp
						    (pmforcut.item[i]->fname,
						     filename)) {
							pmforcut.now = i;
							break;
						}
					if (i < pmforcut.num) {
						a_delete(&pmforcut, 1);
						rename(fpath, newpath);
						sethomefile(genbuf,
							    currentuser->userid,
							    "copypaste");
						unlink(genbuf);
					}
					freeitem(&pmforcut);
				} else {	/*if (copymode == 2) */

					symlink(realfpath, newpath);
				}
			}
			if (hasc) {
				a_additem(pm, title, filename, NULL, 0);
				a_savenames(pm);
				an_log("paste", newpath);
			}
		}
	}
	pm->page = 9999;
}

static void
a_delete(pm, mode)
MENU *pm;
int mode;
{
	ITEM *item;
	char fpath[PATHLEN];
	int n;

	item = pm->item[pm->now];
	move(t_lines - 2, 0);
	prints("%5d  %-50s\n", pm->now + 1, item->title);
	if (mode == 0) {
		if (item->host == NULL) {
			if (snprintf
			    (fpath, PATHLEN, "%s/%s", pm->path,
			     item->fname) > PATHLEN - 1) {
				prints("目录过深,请联系系统维护!");
				egetch();
				return;
			} else if (dashl(fpath)) {
				if (askyn("删除此连目, 确定吗", NA, YEA) == NA)
					return;
				an_log("delitem", fpath);
				unlink(fpath);
			} else if (dashf(fpath)) {
				if (askyn("删除此档案, 确定吗", NA, YEA) == NA)
					return;
				an_log("delitem", fpath);
				deltree(fpath);
			} else if (dashd(fpath)) {
				if (askyn
				    ("删除整个子目录, 别开玩笑哦, 确定吗", NA,
				     YEA) == NA)
					return;
				an_log("delitem", fpath);
				deltree(fpath);
			}
		} else {
			if (askyn("删除连线选项, 确定吗", NA, YEA) == NA)
				return;
		}
	}
	free(item);
	(pm->num)--;
	for (n = pm->now; n < pm->num; n++)
		pm->item[n] = pm->item[n + 1];
	a_savenames(pm);
}

static int
a_changemtitle(char *fpath, char *newmtitle)
{
	MENU pm;
	pm.path = fpath;
	pm.level |= PERM_BOARDS;
	a_loadnames(&pm);
	strsncpy(pm.mtitle, newmtitle, sizeof (pm.mtitle));
	a_savenames(&pm);
	freeitem(&pm);
	return 0;
}

static void
a_newname(pm)
MENU *pm;
{
	ITEM *item;
	char fname[STRLEN];
	char fpath[PATHLEN];
	char *mesg;

	item = pm->item[pm->now];
	a_prompt(-2, "新档名: ", fname, 16);
	if (*fname == '\0')
		return;
	if (snprintf(fpath, PATHLEN, "%s/%s", pm->path, fname) > PATHLEN - 1)
		mesg = "路径太深";
	//可以用中文ID一类的字符串作为目录名, 为个人文集.
	else if (!valid_fname(fname) && !goodgbid(fname)) {
		mesg = "不合法档案名称.";
	} else if (dashf(fpath) || dashd(fpath)) {
		mesg = "系统中已有此档案存在了.";
	} else {
		sprintf(genbuf, "%s/%s", pm->path, item->fname);
		if (rename(genbuf, fpath) == 0) {
			strcpy(item->fname, fname);
			a_savenames(pm);
			return;
		}
		mesg = "档名更改失败!!";
	}
	prints(mesg);
	egetch();
}

static void
a_manager(pm, ch)
MENU *pm;
int ch;
{
	char uident[STRLEN];
	ITEM *item = NULL;
	char fpath[PATHLEN], changed_T[STRLEN], ans[5], fname[STRLEN],
	    newfpath[PATHLEN], tmpfile[STRLEN], attach_path[STRLEN];
	time_t now;
	int count, ret;

	if (pm->num > 0) {
		item = pm->item[pm->now];
		if (snprintf(fpath, PATHLEN, "%s/%s", pm->path, item->fname) >
		    PATHLEN - 1)
			return;
	}
	if (!strchr("agiGpfsMDVvTEnxj", ch) && ch != Ctrl('r')
	    && ch != Ctrl('e'))
		return;
	modify_user_mode(EDITANN);
	switch (ch) {
	case 'a':
		a_newitem(pm, ADDITEM);
		break;
	case 'g':
		a_newitem(pm, ADDGROUP);
		break;
	case 'i':
		a_newitem(pm, ADDMAIL);
		break;
	case 'G':
		if (USERPERM(currentuser, PERM_SYSOP))
			a_newitem(pm, ADDGOPHER);
		break;
	case 'p':
		a_copypaste(pm, 1);
		break;
	/*收集丢失条目, add by ylsdd */
	case Ctrl('r'):
		sprintf(genbuf,
			"发现 %d 个丢失条目, 请按<Enter>继续...", a_repair(pm));
		a_prompt(-1, genbuf, ans, 2);
		pm->page = 9999;
		break;
	/*--- end add by ylsdd ---*/

	case Ctrl('e'):
		sprintf(genbuf,
			"捡回 %d 个垃圾,请按<Enter>继续...", a_rjunk(pm));
		a_prompt(-1, genbuf, ans, 2);
		pm->page = 9999;
		break;

	case 'f':
		pm->page = 9999;
		add_anpath(pm->mtitle, pm->path);
		break;
	}
	if (pm->num > 0)
		switch (ch) {
		case 's':
			if (++a_fmode >= 3)
				a_fmode = 1;
			pm->page = 9999;
			break;
		case 'M':
			a_moveitem(pm);
			pm->page = 9999;
			break;
		case 'D':
			a_delete(pm, 0);
			pm->page = 9999;
			break;
		case 'V':
		case 'v':
			if (USERPERM(currentuser, PERM_SYSOP)) {
				if (ch == 'v')
					sprintf(fpath, "%s/.Names", pm->path);
				else
					sprintf(fpath, "0Announce/.Search");

				if (dashf(fpath)) {
					vedit(fpath, 0, YEA);
				}
				pm->page = 9999;
			}
			break;
		case 'T':
			ret = 0;
			if (dashl(fpath))
				break;
			strsncpy(changed_T, item->title, 39);	//39, it IS 39
			{
				int i = strlen(changed_T) - 1;
				while (i > 0 && isspace(changed_T[i]))
					changed_T[i--] = 0;
			}
			move(t_lines - 2, 0);
			clrtoeol();
			getdata(t_lines - 2, 0, "新标题：", changed_T, 39,
				DOECHO, NA);
			pm->page = 9999;
			if (!*changed_T)
				break;
			/* modified by netty to properly handle title change,add bm by SmallPig */
			if (dashf(fpath)) {
				sprintf(genbuf, "%-38.38s %s ",
					changed_T, currentuser->userid);
				strsncpy(item->title, genbuf,
					 sizeof (item->title));
			} else if (dashd(fpath)) {
				if (USERPERM(currentuser, PERM_SYSOP)
				    || USERPERM(currentuser, PERM_ANNOUNCE)) {
					char *dir = fpath + 25;
					char *rcon;
					move(1, 0);
					clrtoeol();
					getdata(1, 0, "版主: ", uident,
						35, DOECHO, YEA);
					rcon =
					    malloc(strlen(dir) +
						   strlen(changed_T) + 100);
					sprintf(genbuf,
						"修改%.34s的标题为:%.34s,BM:%.34s",
						dir, changed_T, uident);
					if (NULL != rcon)
						sprintf(rcon,
							"修改%s的标题为:%s,BM:%s",
							dir, changed_T, uident);
					else
						rcon = "";
					securityreport(genbuf, rcon);
					if (strcmp(rcon, ""))
						free(rcon);
					if (uident[0] != '\0')
						sprintf(genbuf,
							"%-38.38s(BM: %s)",
							changed_T, uident);
					else
						sprintf(genbuf,
							"%-38.38s", changed_T);
				} else
					sprintf(genbuf, "%-38.38s", changed_T);
				if ((!strstr(changed_T, "<GUESTBOOK>")
				     && strstr(item->title, "<GUESTBOOK>"))
				    || (strstr(changed_T, "<HIDE>")
					&& !strstr(item->title, "<HIDE>"))) {
					//根据如上判断，需要rename目录
					time(&now);
					count = 0;
					do {
						if (count++ > MAX_POSTRETRY) {
							ret = -1;
							break;
						}
						sprintf(fname, "M%d",
							(int) now++);
						if (snprintf
						    (newfpath, PATHLEN, "%s/%s",
						     pm->path,
						     fname) > PATHLEN - 1) {
							ret = -2;
							break;
						}
					} while (dashf(newfpath)
						 || dashd(newfpath));
					if (ret == 0) {
						if (rename(fpath, newfpath) ==
						    0)
							strcpy(fpath, newfpath);
						strcpy(item->fname, fname);
					}
				}
				strsncpy(item->title, genbuf,
					 sizeof (item->title));
			} else if (pm->item[pm->now]->host != NULL)
				strsncpy(item->title, changed_T,
					 sizeof (item->title));
			if (ret == 0) {
				a_savenames(pm);
				a_changemtitle(fpath, genbuf);
			}
			pm->page = 9999;
			break;
		case 'E':
			if (dashl(fpath) || !dashf(fpath))
				break;
			sprintf(tmpfile, "boards/.tmp/editpost.%s.%05d",
				currentuser->userid, uinfo.pid);
			copyfile_attach(fpath, tmpfile);
			if (vedit(tmpfile, 0, YEA) < 0) {
				unlink(tmpfile);
				break;
			}
			sprintf(genbuf, "%-38.38s %s",
				item->title, currentuser->userid);
			strsncpy(item->title, genbuf, sizeof (item->title));
			a_savenames(pm);
			snprintf(attach_path, sizeof (attach_path),
				 PATHUSERATTACH "/%s", currentuser->userid);
			clearpath(attach_path);
			decode_attach(fpath, attach_path);
			insertattachments_byfile(fpath, tmpfile,
						 currentuser->userid);
			unlink(tmpfile);
			pm->page = 9999;
			break;
		case 'n':
			a_newname(pm);
			pm->page = 9999;
			break;
		case 'x':
			a_copypaste(pm, -1);
			break;
		case 'j':
			a_journal(pm);
			break;
		}
	modify_user_mode(DIGEST);
}

void
a_menu(maintitle, path, lastlevel, lastbmonly)
char *maintitle, *path;
int lastlevel, lastbmonly;
{
	MENU me;
	char fname[PATHLEN], tmp[STRLEN],webpath[STRLEN],fullpath[255];
	int ch,zf;
	char *bmstr;
	int bmonly;
	int number = 0;
	int bnum;
	int retvBM = 0;
	time_t timein = 0;
	int firstlink = 0;
	struct boardmem *bp = NULL;

	modify_user_mode(DIGEST);
	me.path = path;
	strncpy(me.mtitle, maintitle, STRLEN - 1);
	me.mtitle[STRLEN - 1] = 0;
	me.level = lastlevel;
	bmonly = lastbmonly;

	a_loadnames(&me);

	strsncpy(tmp, getbfroma(path), sizeof (tmp));
	if (tmp[0] && (bp = getbcache(tmp)) && !hasreadperm(&(bp->header))) {
		freeitem(&me);
		return;
	}

	bmstr = strstr(me.mtitle + 38, "(BM:");	/*buf+38是为了避免与标题内容混淆了 */
	if (bmstr != NULL) {
		retvBM = chk_currBM_Personal(bmstr + 4, bp);
		switch (retvBM) {
		case 3:	/*PERM_BLEVELS且该目录是不不属于自己的
				   个人目录, 不能进不属于自己的目录 */
			freeitem(&me);
			return;
		case 2:	/*是版主 */
		case 1:	/*是个人版, 并且当前ID是拥有者 */
			if ((me.level & PERM_BOARDS) == 0) {
				me.level |= PERM_BOARDS;
				freeitem(&me);
				a_loadnames(&me);
			}
			break;
		case -1:	/*是个人版, 并且当前ID不是拥有者 */
			if ((me.level & PERM_BOARDS) != 0) {
				me.level &= ~PERM_BOARDS;
				freeitem(&me);
				a_loadnames(&me);
			}
			break;
		default:
			break;
		}
		if (strstr(bmstr, "(BM: BMS)")
		    || strstr(bmstr, "(BM: SECRET)")
		    || strstr(bmstr, "(BM: SYSOPS)"))
			bmonly = 1;
	}
	if (bmonly == 1 && !(me.level & PERM_BOARDS)) {
		freeitem(&me);
		return;
	}

	if (!(me.level & PERM_BOARDS))	/*不是斑竹，进行访问统计 */
		time(&timein);

	if (strcmp(path, "0Announce") && dashl(path)) {
		if (!inlink)
			firstlink = 1;
		inlink = 1;
	}
	if (inlink) {
		me.level &= ~PERM_BOARDS;
		freeitem(&me);
		a_loadnames(&me);
	}
	me.page = 9999;
	me.now = 0;
	while (1) {
		if (me.now >= me.num && me.num > 0) {
			me.now = me.num - 1;
		} else if (me.now < 0) {
			me.now = 0;
		}
		if (me.now < me.page || me.now >= me.page + A_PAGESIZE) {
			me.page = me.now - (me.now % A_PAGESIZE);
			a_showmenu(&me);
		}
		move(3 + me.now - me.page, 0);
		prints("->");
		can_R_endline = 1;
		ch = egetch();
		can_R_endline = 0;
		move(3 + me.now - me.page, 0);
		prints("  ");
		if (ch == 'Q' || ch == 'q' || ch == KEY_LEFT || ch == EOF)
			break;
	      EXPRESS:		/* add by djq,990725 */
		switch (ch) {
		case KEY_UP:
		case 'K':
		case 'k':
			if (--me.now < 0)
				me.now = me.num - 1;
			break;
		case KEY_DOWN:
		case 'J':
		case 'j':
			if (++me.now >= me.num)
				me.now = 0;
			break;
		case KEY_PGUP:
		case Ctrl('B'):
			if (me.now >= A_PAGESIZE)
				me.now -= A_PAGESIZE;
			else if (me.now > 0)
				me.now = 0;
			else
				me.now = me.num - 1;
			break;
		case KEY_PGDN:
		case Ctrl('F'):
		case ' ':
			if (me.now < me.num - A_PAGESIZE)
				me.now += A_PAGESIZE;
			else if (me.now < me.num - 1)
				me.now = me.num - 1;
			else
				me.now = 0;
			break;
		case KEY_HOME:
			me.now = 0;
			break;
		case KEY_END:
			me.now = me.num - 1;
			break;
		case Ctrl('C'):
			if (me.num == 0)
				break;
			if (!USERPERM(currentuser, PERM_POST))
				break;
			if (inprison)
				break;
			if (currentuser->dieday)
				break;
			if (snprintf
			    (fname, PATHLEN, "%s/%s", path,
			     me.item[me.now]->fname) > PATHLEN - 1)
				break;	//add by ylsdd
			if (!dashf(fname))
				break;
			if (me.now >= me.num)
				break;
			{
				char bname[30];
				clear();
#if 0
				prints
				    ("\033[1m请注意：本站站规规定：内容相同或类似的文章严禁在\033[31m5(不含)\033[37m个以上讨论区重复张贴。\n");
				prints
				    ("\033[1m转贴超过5个讨论区者除所贴文章会被全部删除之外，还将被剥夺全站发表文章的权利。\n");
				prints
				    ("\033[1m             请大家共同维护 BBS 的环境，节省系统资源。谢谢合作。\n\033[0m");
#endif
				move(4, 0);
				if (!get_a_boardname
				    (bname, "请输入要转贴的讨论区名称: ")) {
					me.page = 999;
					break;
				}
				if (deny_me(bname)
				    && !USERPERM(currentuser, PERM_SYSOP)) {
					move(5, 0);
					clrtobot();
					prints
					    ("\n\n                 很抱歉，你被版主停止 POST 的权利。");
					pressreturn();
					me.page = 999;
					break;
				}
				bnum = getbnum(bname);
				if (!haspostperm(bnum)) {
					move(5, 0);
					clrtobot();
					prints
					    ("\n\n               您尚无权限在 %s 发表文章",
					     bname);
					pressreturn();
					me.page = 999;
					break;
				}
				if (club_board(bname, bnum)) {
					if (!clubtest(bname)
					    && !USERPERM(currentuser,
							 PERM_SYSOP)) {
						move(5, 0);
						clrtobot();
						prints
						    ("\n\n           %s为俱乐部版面，请向版务申请发文权限",
						     bname);
						pressreturn();
						me.page = 999;
						break;
					}
				}
				if (noadm4political(bnum)) {
					move(5, 0);
					clrtobot();
					prints
					    ("\n\n               对不起,因为没有版面管理人员在线,本版暂时封闭.");
					pressreturn();
					me.page = 999;
					break;
				}
				if (bbsinfo.bcache[bnum - 1].ban == 2) {
					move(5, 0);
					clrtobot();
					prints
					    ("\t对不起, 版面文章超限, 本版暂时封闭.");
					pressreturn();
					me.page = 999;
					break;
				}

				move(5, 0);
				sprintf(tmp, "你确定要转贴到 %s 版吗", bname);
				if (askyn(tmp, NA, NA) != 1) {
					me.page = 999;
					break;
				}
				if (postfile
				    (fname, bname,
				     me.item[me.now]->title, 3) != -1) {
					move(7, 0);
					sprintf(tmp,
						"\033[1m已经帮你转贴至 %s 版了\033[m",
						bname);
					prints(tmp);
				}
				refresh();
				sleep(1);
			}
			me.page = 9999;
			break;
		case 'w':
			if ((in_mail != YEA)
			    && USERPERM(currentuser, PERM_READMAIL))
				m_read();
			me.page = 9999;
			break;
		case 'h':
			show_help("help/announcereadhelp");
			me.page = 9999;
			break;
		case '\n':
		case '\r':
			if (number > 0) {
				me.now = number - 1;
				number = 0;
				continue;
			}
		case 'R':
		case 'r':
		case KEY_RIGHT:
			if (me.now < me.num) {
				if (me.item[me.now]->host != NULL) {
					if (me.item[me.now]->fname[0] == '0') {
						if (get_con
						    (me.item[me.now]->host,
						     me.item[me.now]->port)
						    != -1) {
							char
							 tmpfile[30];

							GOPHER tmp;
							extern GOPHER *tmpitem;

							tmpitem = &tmp;
							strcpy
							    (tmp.server,
							     me.item
							     [me.now]->host);
							strcpy(tmp.file,
							       me.item
							       [me.now]->fname);
							sprintf
							    (tmp.title,
							     "0%s",
							     me.item
							     [me.now]->title);
							tmp.port =
							    me.item[me.now]->
							    port;
							enterdir(me.
								 item[me.now]->
								 fname);
							setuserfile(tmpfile,
								    "gopher.tmp");
							savetmpfile(tmpfile);
							ansimore(tmpfile, YEA);
							unlink(tmpfile);
						}
					} else {
						gopher(me.item[me.now]->host,
						       me.item[me.now]->fname,
						       me.item[me.now]->port,
						       me.item[me.now]->title);
					}
					me.page = 9999;
					break;
				}
				if (snprintf
				    (fname, PATHLEN, "%s/%s", path,
				     me.item[me.now]->fname) > PATHLEN - 1)
					break;
				if (dashf(fname)) {
					ansimore_withzmodem(fname, NA,
							    me.item[me.now]->
							    title);
					for (zf=9;zf<strlen(path);zf++) {
            webpath[zf-9]=path[zf];
           }		    
					prints("全文链接：http://" MY_BBS_DOMAIN "/" SMAGIC "/bbsanc?path=%s\n&item=/%s\n",webpath,me.item[me.now]->fname);		
					prints
					    ("\033[1m\033[44m\033[31m[阅读精华区资料]  \033[33m结束 Q, ← │ 上一项资料 U,↑│ 下一项资料 <Enter>,<Space>,↓ \033[m");
					switch (ch = egetch()) {
					case KEY_DOWN:
					case ' ':
					case '\n':
						if (++me.now >= me.num)
							me.now = 0;
						ch = KEY_RIGHT;
						goto EXPRESS;
					case KEY_UP:
					case 'u':
					case 'U':
						if (--me.now < 0)
							me.now = me.num - 1;
						ch = KEY_RIGHT;
						goto EXPRESS;
					case 'h':
						goto EXPRESS;
					case Ctrl('Y'):
						zsend_file(fname,
							   me.item[me.now]->
							   title);
						break;
					default:
						break;
					}

				} else if (dashd(fname)) {
					a_menu(me.item[me.now]->title,
					       fname, me.level, bmonly);
				}
				me.page = 9999;
			}
			freeitem(&me);
			a_loadnames(&me);
			break;
		case '/':
			a_search(&me, 1);
			break;
		case '?':
			a_search(&me, -1);
			break;
		case 'F':
		case 'U':
			if (me.now < me.num
			    && USERPERM(currentuser, PERM_BASIC)) {
				a_forward(path, me.item[me.now], ch == 'U');
				me.page = 9999;
			}
			break;
		case '!':
			if (!Q_Goodbye())
				break;	/* youzi leave */
		case 'c':
			if (me.now < me.num) {
				if (snprintf
				    (fname, PATHLEN, "%s/%s", path,
				     me.item[me.now]->fname) > PATHLEN - 1)
					break;	//add by ylsdd
//                              if (dashf(fname) && (me.level & PERM_BOARDS))
//                                      a_copypaste(&me, 0);
//                              else
				a_copypaste(&me, 2);
			}
			break;
		case Ctrl('Y'):
			if (me.now < me.num) {
				if (me.item[me.now]->host != NULL) {
					me.page = 9999;
					break;
				} else
					sprintf(fname, "%s/%s", path,
						me.item[me.now]->fname);
				if (dashf(fname)) {
					zsend_file(fname,
						   me.item[me.now]->title);
					me.page = 9999;
				}
			}
			break;
		}
		if (ch >= '0' && ch <= '9') {
			number = number * 10 + (ch - '0');
			ch = '\0';
		} else {
			number = 0;
		}
		if (me.level & PERM_BOARDS)
			a_manager(&me, ch);
		else if (ch == 'a' && USERPERM(currentuser, PERM_POST)
			 && strstr(me.mtitle, "<GUESTBOOK>") == me.mtitle) {
			a_newitem(&me, ADDITEM);
		}
	}
	freeitem(&me);
	if (timein)
		logvisit(timein, path);
	if (firstlink)
		inlink = 0;
}

void
linkto(path, fname, title)
char *path, *title, *fname;
{
	MENU pm;

	bzero(&pm, sizeof (pm));	//Socrates
	pm.path = path;
	pm.level |= PERM_BOARDS;	/*add by ylsdd */
	a_loadnames(&pm);
	a_additem(&pm, title, fname, NULL, 0);
	a_savenames(&pm);
	freeitem(&pm);
}

int
add_grp(group, gname, bname, title)
char group[STRLEN], bname[STRLEN], title[STRLEN], gname[STRLEN];
{
	FILE *fn;
	char buf[PATHLEN];
	char searchname[STRLEN];
	char gpath[STRLEN * 2];
	char bpath[STRLEN * 2];

	sprintf(searchname, "%s: groups/%s/%s", bname, group, bname);
	sprintf(gpath, "0Announce/groups/%s", group);
	sprintf(bpath, "%s/%s", gpath, bname);
	if (!dashd("0Announce")) {
		mkdir("0Announce", 0770);
		chmod("0Announce", 0770);
		if ((fn = fopen("0Announce/.Names", "w")) == NULL)
			return -1;
		fprintf(fn, "#\n");
		fprintf(fn, "# Title=%s 精华区公布栏\n", MY_BBS_NAME);
		fprintf(fn, "#\n");
		fclose(fn);
	}
	if (!seek_in_file("0Announce/.Search", bname))
		addtofile("0Announce/.Search", searchname);
	if (!dashd("0Announce/groups")) {
		mkdir("0Announce/groups", 0777);
		chmod("0Announce/groups", 0777);

		linkto("0Announce", "groups", "讨论区精华");
	}
	if (!dashd(gpath)) {
		mkdir(gpath, 0777);
		chmod(gpath, 0777);
		linkto("0Announce/groups", group, gname);
	}
	if (!dashd(bpath)) {
		mkdir(bpath, 0770);
		chmod(bpath, 0770);
		linkto(gpath, bname, title);
		sprintf(buf, "%s/.Names", bpath);
		if ((fn = fopen(buf, "w")) == NULL) {
			return -1;
		}
		fprintf(fn, "#\n");
		fprintf(fn, "# Title=%s\n", title);
		fprintf(fn, "#\n");
		fclose(fn);
	}
	return 1;

}

int
del_grp(grp, bname, title)
char grp[STRLEN], bname[STRLEN], title[STRLEN];
{
	char buf[STRLEN], buf2[STRLEN], buf3[30];
	char gpath[STRLEN * 2];
	char bpath[STRLEN * 2];
	char check[30];
	int i, n;
	MENU pm;

	strncpy(buf3, grp, 29);
	buf3[29] = '\0';
	sprintf(buf, "0Announce/.Search");
	sprintf(gpath, "0Announce/groups/%s", buf3);
	sprintf(bpath, "%s/%s", gpath, bname);
	deltree(bpath);

	pm.path = gpath;
	pm.level |= PERM_BOARDS;	/*add by ylsdd */
	a_loadnames(&pm);
	for (i = 0; i < pm.num; i++) {
		strsncpy(buf2, pm.item[i]->fname, sizeof (buf2));
		strcpy(check, strtok(buf2, "/~\n\b"));
		if (strstr(pm.item[i]->title, title)
		    && !strcmp(check, bname)) {
			free(pm.item[i]);
			(pm.num)--;
			for (n = i; n < pm.num; n++)
				pm.item[n] = pm.item[n + 1];
			a_savenames(&pm);
			break;
		}
	}
	freeitem(&pm);
	return 0;
}

int
edit_grp(bname, grp, title, newtitle)
char bname[STRLEN], grp[STRLEN], title[STRLEN], newtitle[100];
{
	char buf[STRLEN], buf2[STRLEN], buf3[30];
	char gpath[STRLEN * 2];
	char bpath[STRLEN * 2];
	char check[30];
	int i;
	MENU pm;

	strncpy(buf3, grp, 29);
	buf3[29] = '\0';
	sprintf(buf, "0Announce/.Search");
	sprintf(gpath, "0Announce/groups/%s", buf3);
	sprintf(bpath, "%s/%s", gpath, bname);
	if (!seek_in_file(buf, bname))
		return 0;

	pm.path = gpath;
	pm.level |= PERM_BOARDS;	/*add by ylsdd */
	a_loadnames(&pm);
	for (i = 0; i < pm.num; i++) {
		strsncpy(buf2, pm.item[i]->fname, sizeof (buf2));
		strcpy(check, strtok(buf2, "/~\n\b"));
		if (strstr(pm.item[i]->title, title)
		    && !strcmp(check, bname)) {
			strsncpy(pm.item[i]->title, newtitle,
				 sizeof (pm.item[i]->title));
			break;
		}
	}
	a_savenames(&pm);
	freeitem(&pm);
	pm.path = bpath;
	a_loadnames(&pm);
	strsncpy(pm.mtitle, newtitle, sizeof (pm.mtitle));
	a_savenames(&pm);
	freeitem(&pm);
	return 0;
}

void
Announce()
{
	sprintf(genbuf, "%s 精华区公布栏", MY_BBS_NAME);
	a_menu(genbuf, "0Announce", (USERPERM(currentuser, PERM_ANNOUNCE)
				     || USERPERM(currentuser, PERM_SYSOP)) ?
	       PERM_BOARDS : 0, 0);
	clear();
}

//add by gluon, modified by ylsdd*/ 
void
Personal(cmd)
char *cmd;
{
	char buf[100], ch;
	/*strcpy( genbuf, "个人文集"); */
	strcpy(genbuf, "");
	switch (cmd[0]) {
	case '*':
		ch = currentuser->userid[0];
		if (((ch >= 'A') && (ch <= 'Z'))
		    || ((ch >= 'a') && (ch <= 'z'))) {
			ch = (ch >= 'A' && ch <= 'Z') ? ch : (ch + 'A' - 'a');
			sprintf(buf,
				"0Announce/groups/GROUP_0/Personal_Corpus/%c/%.15s",
				ch, currentuser->userid);
		} else {
			for (ch = 'A'; ch <= 'Z'; ch++) {
				sprintf(buf,
					"0Announce/groups/GROUP_0/Personal_Corpus/%c/%.15s",
					ch, currentuser->userid);
				if (dashd(buf))
					break;
			}
		}
		if (!dashd(buf)) {
			a_prompt(-1,
				 "没有找到您的个人文集, 请在【主选单 S 特别服务】自助申请，按<Enter>继续...",
				 genbuf, 2);
			clear();
			return;
		}
		break;
	case '$':
		ch = cmd[1];
		if (((ch >= 'A') && (ch <= 'Z'))
		    || ((ch >= 'a') && (ch <= 'z'))) {
			ch = (ch >= 'A' && ch <= 'Z') ? ch : (ch + 'A' - 'a');
			sprintf(buf,
				"0Announce/groups/GROUP_0/Personal_Corpus/%c/%.15s",
				ch, &(cmd[1]));
		} else {
			for (ch = 'A'; ch <= 'Z'; ch++) {
				sprintf(buf,
					"0Announce/groups/GROUP_0/Personal_Corpus/%c/%.15s",
					ch, &(cmd[1]));
				if (dashd(buf))
					break;
			}
		}
		if (!dashd(buf)) {
			a_prompt(-1,
				 "没有找到该个人文集，按<Enter>继续...",
				 genbuf, 2);
			clear();
			return;
		}
		break;
	default:
		ch = cmd[0];
		if (((ch >= 'A') && (ch <= 'Z'))
		    || ((ch >= 'a') && (ch <= 'z'))) {
			ch = (ch >= 'A' && ch <= 'Z') ? ch : (ch + 'A' - 'a');
			sprintf(buf,
				"0Announce/groups/GROUP_0/Personal_Corpus/%c",
				ch);
		} else {
			for (ch = 'A'; ch <= 'Z'; ch++) {
				sprintf(buf,
					"0Announce/groups/GROUP_0/Personal_Corpus/%c/%.15s",
					ch, &(cmd[0]));
				if (dashd(buf))
					break;
			}
		}
		if (!dashd(buf)) {
			clear();
			return;
		}
		sprintf(buf, "0Announce/groups/GROUP_0/Personal_Corpus/%c", ch);
		break;
	}
	a_menu(genbuf, buf, (USERPERM(currentuser, PERM_ANNOUNCE)
			     || USERPERM(currentuser,
					 PERM_SYSOP)) ? PERM_BOARDS : 0, 0);
}

// end 
/*chk_currBM_Personal用于对个人精华区的支持, by ylsdd*/
static int
chk_currBM_Personal(BMstr, bp)
char *BMstr;
struct boardmem *bp;
{
	char *ptr;
	char BMstrbuf[BM_LEN];
	int chk1 = 0, chk2 = 0;
	strncpy(BMstrbuf, BMstr, BM_LEN - 1);
	BMstrbuf[BM_LEN - 1] = 0;

	ptr = strtok(BMstrbuf, ",: ;|&()\n");
	while (1) {
		if (ptr == NULL)
			break;
		if (!strcmp(ptr, currentuser->userid))
			chk1 = 1;
		if (!strcmp(ptr, "_Personal"))
			chk2 = 1;
		if (!strcmp(ptr, "_AllBM") && bp
		    && chk_currBM(&(bp->header), 0))
			chk1 = 1;
		if (chk1 && chk2)
			break;
		ptr = strtok(NULL, ",: ;|&()\n");
	}
	if (chk2 == 0) {
		if (!USERPERM(currentuser, PERM_BOARDS))
			return 0;
		if (chk1 == 1 || USERPERM(currentuser, PERM_BLEVELS))
			return 2;
		return 0;
	}
	if (chk1 == 0 && USERPERM(currentuser, PERM_BLEVELS))
		return 3;
	if (chk1 == 1 && USERPERM(currentuser, PERM_SPECIAL8))
		return 1;
	return -1;
}

/* logvisit, 用来记录精华区的访问次数和时间长度 */
int
logvisit(time_t timein, const char *path)
{
	char fn[PATH_MAX + 1], *ptr;
	int fd, t, n[2], len;
	t = time(NULL) - timein;
	if (t < 20)
		return 0;
	if (!(ptr = strstr(path, "Personal_Corpus")))
		ptr = strstr(path, "groups/GROUP_");
	if (ptr && countstr(ptr, "/") > 3)
		return 0;
	len = snprintf(fn, PATH_MAX + 1, "%s/.logvisit", path);
	if (len > PATH_MAX)
		return -1;
	fd = open(fn, O_RDWR | O_CREAT, 0660);
	if (fd < 0)
		return -1;
	if (read(fd, n, sizeof (int) * 2) <= 0) {
		n[0] = 0;
		n[1] = 0;
	}
	n[0] += 1;
	n[1] += t;
	lseek(fd, 0, SEEK_SET);
	write(fd, n, sizeof (int) * 2);
	close(fd);
	return 0;
}

int
getvisit(int n[2], const char *path)
{
	char fn[PATH_MAX + 1];
	int fd;
	int len;
	n[0] = 0;
	n[1] = 0;
	len = snprintf(fn, PATH_MAX + 1, "%s/.logvisit", path);
	if (len > PATH_MAX)
		return -1;
	fd = open(fn, O_RDONLY | O_CREAT, 0660);
	if (fd < 0)
		return -1;
	if (read(fd, n, sizeof (int) * 2) <= 0) {
		n[0] = 0;
		n[1] = 0;
	}
	close(fd);
	return 0;
}

/* 下面的函数a_repair用来收集丢失的条目, 把它们放进精华区的目录中  by ylsdd*/

static int
a_repair(pm)
MENU *pm;
{
	DIR *dirp;
	struct dirent *direntp;
	int i, changed;
	changed = 0;

	dirp = opendir(pm->path);
	if (dirp == NULL)
		return -1;

	while ((direntp = readdir(dirp)) != NULL) {
		if (direntp->d_name[0] == '.')
			continue;
		for (i = 0; i < pm->num; i++) {
			if (strcmp(pm->item[i]->fname, direntp->d_name)
			    == 0) {
				i = -1;
				break;
			}
		}
		if (i != -1) {
			a_additem(pm, direntp->d_name, direntp->d_name,
				  NULL, 0);
			changed++;
		}
	}
	closedir(dirp);
	if (changed > 0)
		a_savenames(pm);
	return changed;
}

/* 下面的函数a_rjunk用来恢复从精华区中删除的目录或者文章,把它们放进精华区的目录中  by lepton*/

static int
a_rjunk(pm)
MENU *pm;
{
	DIR *dirp;
	struct dirent *direntp;
	int changed = 0, len, count;
	char buf[PATH_MAX + 1], rpath[PATH_MAX + 1], fpath[PATHLEN],
	    fname[STRLEN];
	time_t now;

	//Make the leading part of the junk filename
	if (realpath(MY_BBS_HOME "/0Announce", rpath) == NULL)
		return -1;
	len = strlen(rpath);
	if (rpath[len - 1] != '/')
		len++;
	if (realpath(pm->path, rpath) == NULL)
		return -1;
	strcpy(buf, rpath + len);
	normalize(buf);
	len = strlen(buf);

	//Look for the junk files, and restore them
	dirp = opendir(MY_BBS_HOME "/0Announce/.junk");
	if (dirp == NULL)
		return -2;
	time(&now);
	while ((direntp = readdir(dirp)) != NULL) {
		if (strncmp(buf, direntp->d_name, len)
		    || !strlen(direntp->d_name + len))
			continue;
		sprintf(rpath, MY_BBS_HOME "/0Announce/.junk/%s",
			direntp->d_name);
		count = 0;
		do {
			if (count++ > MAX_POSTRETRY)
				goto OUT;
			sprintf(fname, "M%d", (int) now++);
			if (snprintf
			    (fpath, PATHLEN, "%s/%s", pm->path,
			     fname) > PATHLEN - 1)
				goto OUT;
		} while (!access(fpath, F_OK));
		rename(rpath, fpath);
		a_additem(pm, fname, fname, NULL, 0);
		changed++;
	}
      OUT:
	closedir(dirp);

	//Save the new list
	if (changed)
		a_savenames(pm);
	return changed;
}

int
add_anpath(char *title, char *path)
{
	char titles[20][STRLEN], paths[20][PATHLEN], *ptr;
	int i;
	int index = 0, nindex = 0;
	read_anpath(titles, paths);
	move(t_lines - 22, 0);
	clrtobot();
	prints
	    ("设置为哪个丝路? (按A-T↑↓选择, ' '或回车确定', ←或'X'取消, Z删除)");
	for (i = 0; i < 20; i++) {
		move(t_lines - 22 + 1 + i, 0);
		prints(" %s(%c) %s\033[0m",
		       (i == index) ? ">\033[1;7m" : " ",
		       'A' + i,
		       (titles[i][0] !=
			0) ? titles[i] : "\033[32m尚未设定\033[0m");
	}
	while (1) {
		i = igetkey();
		i = toupper(i);
		if (i == KEY_LEFT || i == 'X')
			return -1;
		if (i == '\n' || i == '\r' || i == ' ')
			break;
		if(i == 'Z') {
			if(!titles[index][0])
				continue;
			titles[index][0] = 0;
			save_anpath(titles, paths);
		} else if (i == KEY_UP || i == KEY_DOWN) {
			if (i == KEY_UP) {
				nindex = index - 1;
				if (nindex < 0)
					nindex = 19;
			} else {
				nindex = index + 1;
				if (nindex >= 20)
					nindex = 0;
			}
		} else {
			i = i - 'A';
			if (i >= 0 && i < 20) {
				index = i;
				break;
			}
		}
		if (nindex != index || i == 'Z') {
			move(t_lines - 22 + 1 + index, 0);
			prints("  (%c) %s\033[0m",
			       'A' + index,
			       (titles[index][0] !=
				0) ? titles[index] : "\033[32m尚未设定\033[0m");
			clrtoeol();
			move(t_lines - 22 + 1 + nindex, 0);
			prints(" >\033[1;7m(%c) %s\033[0m",
			       'A' + nindex,
			       (titles[nindex][0] !=
				0) ? titles[nindex] :
			       "\033[32m尚未设定\033[0m");
			clrtoeol();
			index = nindex;
		}
	}
	if ((ptr = strchr(title, '\n')) != NULL)
		*ptr = 0;
	if ((ptr = strchr(path, '\n')) != NULL)
		*ptr = 0;
	strncpy(titles[index], title, sizeof (titles[index]));
	titles[index][sizeof (titles[index]) - 1] = 0;
	strncpy(paths[index], path, sizeof (paths[index]));
	paths[index][sizeof (paths[index]) - 1] = 0;
	move(t_lines - 22 + 1 + index, 0);
	clrtoeol();
	prints("  (%c) %s", 'A' + index,
	       (titles[index][0] !=
		0) ? titles[index] : "\033[32m尚未设定\033[0m");
	if (save_anpath(titles, paths) < 0) {
		prints("设置丝路错误! 按任意键继续");
		igetkey();
	} else
		pressreturn();
	return 0;
}

int
select_anpath()
{
	char titles[20][STRLEN], paths[20][PATHLEN];
	int i;
	static int index = 0, nindex;
	Importname[0] = 0;
	move(t_lines - 22, 0);
	clrtobot();
	if (read_anpath(titles, paths) <= 0) {
		prints("丝路未曾设置或过期, 请首先设置丝路");
		pressreturn();
		return -1;
	}
	prints
	    ("将文档保存到哪个丝路? (按A-T↑↓选择, ' '或回车确定', ←或'X'取消)");
	for (i = 0; i < 20; i++) {
		move(t_lines - 22 + 1 + i, 0);
		prints(" %s(%c) %s\033[0m",
		       (i == index) ? ">\033[1;7m" : " ",
		       'A' + i,
		       (titles[i][0] !=
			0) ? titles[i] : "\033[32m尚未设定\033[0m");
	}
	while (1) {
		i = igetkey();
		i = toupper(i);
		if (i == KEY_LEFT || i == 'X')
			return -1;
		if (i == '\n' || i == '\r' || i == ' ') {
			if (titles[index][0] != 0)
				break;
			continue;
		}
		if (i == KEY_UP || i == KEY_DOWN) {
			if (i == KEY_UP) {
				nindex = index - 1;
				if (nindex < 0)
					nindex = 19;
			} else {
				nindex = index + 1;
				if (nindex >= 20)
					nindex = 0;
			}
		} else {
			i = i - 'A';
			if (i >= 0 && i < 20) {
				if (titles[i][0] != 0) {
					index = i;
					break;
				}
				nindex = i;
			}
		}
		if (nindex != index) {
			move(t_lines - 22 + 1 + index, 0);
			prints("  (%c) %s\033[0m",
			       'A' + index,
			       (titles[index][0] !=
				0) ? titles[index] : "\033[32m尚未设定\033[0m");
			clrtoeol();
			move(t_lines - 22 + 1 + nindex, 0);
			prints(" >\033[1;7m(%c) %s\033[0m",
			       'A' + nindex,
			       (titles[nindex][0] !=
				0) ? titles[nindex] :
			       "\033[32m尚未设定\033[0m");
			clrtoeol();
			index = nindex;
		}
	}
	strcpy(Importname, paths[index]);
	return 0;
}

int
save_anpath(char titles[20][STRLEN], char paths[20][PATHLEN])
{
	int i;
	FILE *fp;
	char pathfile[PATHLEN];
	sethomefile(pathfile, currentuser->userid, "path8");
	if ((fp = fopen(pathfile, "w")) == NULL)
		return -1;
	for (i = 0; i < 20; i++)
		if (titles[i][0] != 0)
			fprintf(fp, "%s\n%s\n", titles[i], paths[i]);
		else
			fprintf(fp, "\n\n");
	fclose(fp);
	return 0;
}

int
read_anpath(char titles[20][STRLEN], char paths[20][PATHLEN])
{
	int i, j = 0;
	FILE *fp;
	char pathfile[PATHLEN], *ptr;
	sethomefile(pathfile, currentuser->userid, "path8");
	if ((fp = fopen(pathfile, "r")) == NULL) {
		for (i = 0; i < 20; i++)
			titles[i][0] = 0;
		return -1;
	}
	for (i = 0; i < 20; i++) {
		if (fgets(titles[i], STRLEN, fp) == NULL
		    || fgets(paths[i], PATHLEN, fp) == NULL) {
			while (i < 20)
				titles[i++][0] = 0;
			break;
		}
		if ((ptr = strchr(titles[i], '\n')) != NULL)
			*ptr = 0;
		if ((ptr = strchr(paths[i], '\n')) != NULL)
			*ptr = 0;
		if (strlen(paths[i]) < 10)
			titles[i][0] = 0;
		if (titles[i][0] != 0)
			j++;
	}
	fclose(fp);
	return j;
}


//cojie 2005.6 for 自动期刊
/*
All MARKS:

BBS.NameCn
BBS.NAMEEN
BBS.Domain

JOURNAL.BoardCn
JOURNALBoardEn
JOURNAL.Publisher
JOURNAL.Number
JOURNAL.Date

INDEX.No
INDEX.Title
INDEX.Begin[?-?]
INDEX.End

ARTICLE.Author
ARTICLE.Time
ARTICLE.Content
ARTICLE.Title
ARTICLE.[?-?]
ARTICLE.End
*/

struct MacroSet {
	char bbs_name_cn[STRLEN];
	char bbs_name_en[STRLEN];
	char bbs_domain[STRLEN];

	char journal_bname_cn[STRLEN];
	char journal_bname_en[STRLEN];
	char journal_name[STRLEN];
	char journal_publisher[20];
	char  journal_no[20];
	time_t journal_date;
	
	int  index_no;
	char index_title[STRLEN];
	void *index;

	char article_author[20];
	char article_title[STRLEN];
	int article_no;
	int article_format;
	FILE *article_fp;
	void *article;
};

void initMacroValue(struct MacroSet *ms, MENU *pm, int format) {
	struct boardmem *board;
	char buf[STRLEN], *ptr;
	
	
	strncpy(ms->bbs_name_cn, MY_BBS_NAME, STRLEN);
	strncpy(ms->bbs_domain, MY_BBS_DOMAIN, STRLEN);

	strncpy(ms->bbs_name_en, MY_BBS_DOMAIN, STRLEN);
	ptr = strchr(ms->bbs_name_en, '.');
	if(!ptr)
		*ptr = '\0';

	strncpy(ms->journal_bname_en, currboard, STRLEN);
	board = getbcache(currboard);
	if(board)
		strncpy(ms->journal_bname_cn, board->header.title, STRLEN);
	else
		strcpy(ms->journal_bname_cn, "--");
	
	// pm->mtitle 形为 [?]xxxxx, 读取?作为期刊号, xxxx为期刊名
	strncpy(buf, pm->mtitle, STRLEN);
	ptr = strchr(buf, ']');
	if(ptr) {
		*ptr = '\0';
		strncpy(ms->journal_no, buf + 1, STRLEN);
		strncpy(ms->journal_name, ptr + 1, STRLEN);
	} else {
		strcpy(ms->journal_no, "--");
		strncpy(ms->journal_name, buf, STRLEN);
	}
	
	strncpy(ms->journal_publisher, currentuser->userid, IDLEN + 1);
	ms->journal_date = time(NULL);

	ms->index_no = 0;
	ms->index = pm;
	
	ms->index_no = 0;
	ms->article_format = format;
	ms->article = pm;
}


void printArticleContent(FILE *fp_output, FILE *fp_article, struct MacroSet *ms) {
	char line[1024], buf[32768];
	char *attach;
	int len;
	long attachpos;
	MENU *pm;

	pm = ms->article;	
	
	while(fgets(line, 1024, fp_article)) {
		// 如果制作.mht格式的期刊
		if(ms->article_format == 2 && transferattach(line, sizeof (line), fp_article, fp_output))
			continue;
		// 如果制作.html引用链接的期刊
                if(ms->article_format == 1 &&NULL != (attach = checkbinaryattach(line, fp_article, &len))) {
			if (len <= 0)
                                continue;
			attachpos = ftell(fp_article) - sizeof(len);
			fseek(fp_article, len, SEEK_CUR);
			fprintf(fp_output, "<IMG SRC=\"http://%s/%s/attach/bbsanc/%s?path=%s&item=/%s&attachpos=%ld&attachname=/%s\">",
				MY_BBS_DOMAIN, SMAGIC, attach, pm->path + strlen("0Announce"), pm->item[ms->article_no]->fname, 
				attachpos, attach);
			continue;	
		}
		if(!strncmp(line, "--\n", 3) || !strncmp(line, "--\r\n", 4))
			break;			

		hsprintfLite(buf, line);
        	filteransi(buf);
	        fputs(buf, fp_output);
	}
	return ;
}


void replaceMacro(char *line, FILE *fp, struct MacroSet *ms, int flag) {
	char *ptr;

	while(line) {
		ptr = strchr(line, '$');
		if(ptr == NULL) {
			fputs(line, fp);
			break;
		} 
		*ptr = '\0';
		fputs(line, fp);

		line = ++ptr;
		if(!strncmp(ptr, "BBS.NameCn", 10)) {
			fprintf(fp, "%s", ms->bbs_name_cn);
			ptr += 10;
		} else if(!strncmp(ptr, "BBS.NameEn", 10)) {
			fprintf(fp, "%s", ms->bbs_name_en);
			ptr += 10;
		} else if(!strncmp(ptr, "BBS.Domain", 10)) {
			fprintf(fp, "%s", ms->bbs_domain);
			ptr += 10;
		} else if(!strncmp(ptr, "JOURNAL.Name", 12)) {
			fprintf(fp, "%s", ms->journal_name);
			ptr += 12;
		} else if(!strncmp(ptr, "JOURNAL.BoardEn", 15)) {
			fprintf(fp, "%s", ms->journal_bname_en);
			ptr += 15;
		} else if(!strncmp(ptr, "JOURNAL.BoardCn", 15)) {
			fprintf(fp, "%s", ms->journal_bname_cn);
			ptr += 15;
		} else if(!strncmp(ptr, "JOURNAL.Publisher", 17)) {
			fprintf(fp, "%s", ms->journal_publisher);
			ptr += 17;
		} else if(!strncmp(ptr, "JOURNAL.Number", 14)) {
                        fprintf(fp, "%s", ms->journal_no);
                        ptr += 14;
                } else if(!strncmp(ptr, "JOURNAL.Date", 12)) {
                        fprintf(fp, "%s", Ctime(ms->journal_date));
                        ptr += 12;
                } else if(flag == 2 && !strncmp(ptr, "INDEX.No", 8)) {
                        fprintf(fp, "%d", ms->index_no);
                        ptr += 8;
                } else if(flag == 2 && !strncmp(ptr, "INDEX.Title", 11)) {
                        fprintf(fp, "%s", ms->index_title);
                        ptr += 11;
		} else if(flag == 3 && !strncmp(ptr, "ARTICLE.Author", 14)) {
                        fprintf(fp, "%s", ms->article_author);
                        ptr += 14;
		} else if(flag == 3 && !strncmp(ptr, "ARTICLE.Title", 13)) {
                        fprintf(fp, "%s", ms->article_title);
                        ptr += 13;
		} else if(flag == 3 && !strncmp(ptr, "ARTICLE.Content", 15)) {
			printArticleContent(fp, ms->article_fp, ms);
                        ptr += 15;
		} else if(flag == 3 && !strncmp(ptr, "ARTICLE.No", 10)) {
			fprintf(fp, "%d", ms->article_no);
                        ptr += 10;
		} else
			fputs("$", fp);	// 补偿一个错误消耗的$

		line = ptr;
	}
}

// 读取XXXXX.Begin[from-to]的from和to
void getRange(char **line, int *from, int *to) {
	char *ptrDiv, *ptrEnd;

	*from = 0;
	*to = 9999;

	if(**line == '[') {
		ptrEnd = strchr(*line, ']');
		if(ptrEnd != NULL) {
			*ptrEnd = '\0';
			*from = atoi(*line + 1);
			ptrDiv = strchr(*line, '-');
			if(ptrDiv != NULL)
				*to = atoi(ptrDiv + 1);
			*line = ptrEnd + 1;
		}
	}
	if(*from < 0)
		*from = 0;
	if(*to < *from )
		*to = 9999;
	return ;
}


void replaceIndexMacro(char *line, FILE *fp_output, struct MacroSet *ms) {
	char *model;
	MENU *pm;
	int i, size, from, to;

	getRange(&line, &from, &to);
	size = strlen(line);
	model = malloc(size + 1);

	pm = ms->index;

	for(i = from; i < pm->num && i < to; i++) {
		strncpy(ms->index_title, pm->item[i]->title, STRLEN);
		ms->index_title[38] = 0;
		ms->index_no = i;
		memcpy(model, line, size);
		model[size] = '\0';
		replaceMacro(model, fp_output, ms, 2);	
	}
	free(model);
	return ;
}


void replaceArticleMacro(char *str, FILE *fp_output, struct MacroSet *ms) {
	char filepath[1024];
	char *model;
	FILE *fp_article;
	int i, from, to, size;
	MENU *pm;

	getRange(&str, &from, &to);

	size = strlen(str);
	model = malloc(size + 1);
	pm = ms->article;
	
	for(i = from; i < pm->num && i < to; i++) {
		snprintf(filepath, 1024, "%s/%s", pm->path, pm->item[i]->fname);
		if(dashd(filepath))	//跳过目录
			continue;
		getdocauthor(filepath, ms->article_author, IDLEN + 1);
		if(ms->article_author[0] == '\0')		
			strcpy(ms->article_author, "--");
		strncpy(ms->article_title, pm->item[i]->title, STRLEN);
		ms->article_title[38] = 0;
		ms->article_no = i;
		memcpy(model, str, size);
		model[size] = '\0';

		fp_article = fopen(filepath, "r");
		if (fp_article == NULL)
			continue;
		ms->article_fp = fp_article;
		replaceMacro(model, fp_output, ms, 3);
		fclose(fp_article);
	}
	free(model);
	return ;
}


static int a_journal(MENU *pm) {
	char *ptr, *ptrStart, *ptrIndex, *ptrArticle;
	char dir[1024], htmpath[512], modelfile[512];
	FILE  *fp_output;
	struct MacroSet ms;
	struct mmapfile pmf = {ptr:NULL};
	int mode, retv;

	move(t_lines - 1, 0);
	if(!strncmp(pm->path, AN_PES_PATH, sizeof(AN_PES_PATH) - 1)) // 个人文集不做期刊
		return -2;
	if(askyn("为本目录文章制作期刊吗?", NA, NA) == NA)
		return 0;
	do {
		clrtoeol();
		prints("选择期刊格式[1.htm  2. mht]:");
		mode = igetkey() - '0';
	} while(mode != 1 && mode != 2);

	initMacroValue(&ms, pm, mode);  // 初始化静态的宏标记

	sprintf(dir, "%s/%s", PATHUSERATTACH, currentuser->userid);
	mkdir(dir, 0760);

	sprintf(htmpath, "%s/journal.htm", dir);
	fp_output = fopen(htmpath, "w+");
	if (fp_output == NULL)
		return -1;

	// 选择模板
	sprintf(modelfile, "ftphome/root/boards/%s/journal/model.htm", currboard);
	if(!file_exist(modelfile))
		strcpy(modelfile, "etc/j_model.htm");		
	if (mmapfile(modelfile, &pmf) < 0) {
		fclose(fp_output);
		return -1;
	}

	ptr = malloc(pmf.size + 1);
	if(ptr == NULL) {
		mmapfile (NULL, &pmf);
		fclose(fp_output);
		return -1;
	}
	memcpy(ptr, pmf.ptr, pmf.size);
	ptr[pmf.size] = '\0';
	mmapfile (NULL, &pmf);

	// 目前只有两个重复替换标记,如果有多个的话,应该考虑标记数组, 用排序的方式来找出最近标记
	for(ptrStart = ptr; ptrStart; ) {
		ptrIndex = strstr(ptrStart, "$INDEX.Begin");
		ptrArticle = strstr(ptrStart, "$ARTICLE.Begin");

		if(ptrIndex == NULL && ptrArticle == NULL) {  //如果没有需要重复的标记
			replaceMacro(ptrStart, fp_output, &ms, 1);
			break;
		} 

		if(ptrIndex && ((ptrIndex < ptrArticle) || !ptrArticle)) { // 如果最近的是索引标记
			*ptrIndex = '\0';
			replaceMacro(ptrStart, fp_output, &ms, 1);
			
			ptrStart = strstr(ptrIndex + 1, "$INDEX.End");
			if(ptrStart) {
				*ptrStart = '\0';
				ptrStart += 10;
			}
			replaceIndexMacro(ptrIndex + 12, fp_output, &ms);
		} else if(ptrArticle) { // 如果最近的是文章标记
			*ptrArticle = '\0';
			replaceMacro(ptrStart, fp_output, &ms, 1);
			
			ptrStart = strstr(ptrArticle + 1, "$ARTICLE.End");
			if(ptrStart) {
				*ptrStart = '\0';
				ptrStart += 12;
			}
			replaceArticleMacro(ptrArticle + 14, fp_output, &ms);
		}
	}
	fclose(fp_output);
	free(ptr);

	move(t_lines - 2, 0);
	if(mode == 2) { // 如果制作mht格式的
		char mhtfile[512];

		sprintf(mhtfile, "%s/journal.mht", dir);
		encodeMIME(htmpath, mhtfile);
		unlink(htmpath);
		if(file_size(mhtfile) > 2000000) {			
			unlink(mhtfile);
			clrtoeol();
			prints("文件太大, 请制作htm格式期刊!");
			pressanykey();
			return -1;
		}
	}

	// 在当前目录新建一篇文章, 把期刊作为其附件
	sprintf(htmpath, "M.%ld.A", time(NULL));
	a_additem(pm, pm->mtitle, htmpath, NULL, 0);
	a_savenames(pm);
	sprintf(htmpath, "%s/%s", pm->path, pm->item[pm->num - 1]->fname);
	retv = appendbinaryattach(htmpath, currentuser->userid, NULL);

	clrtoeol();
	if(retv > 0)
		prints("完成! 期刊已经作为目录首篇文章的附件!");
	else
		prints("错误!");
	pressanykey();
	return !retv;
}

// 把文件src编码为MIME格式的dest
static int encodeMIME(char *src, char *dest) {
        FILE *fin, *fout;
        char *attach;
        char b64_in[512], tail_buf[3];
        int len, tail_len, outlen;
        char *buf, *mime_type;
                                                                                                                                            
        fout = fopen(dest, "w");
        fin = fopen(src, "r");
        if (fin == NULL || fout == NULL) {
                if (fout)
                        fclose(fout);
                if (fin)
                        fclose(fin);
                return -1;
        }
        fprintf(fout, "MIME-Version: 1.0\n");
        fprintf(fout, "Content-Type: multipart/mixed;\n");
        fprintf(fout, "              boundary=" MY_MIME_BOUNDARY "\n");
        fprintf(fout, "Content-Transfer-Encoding: 7bit\n\n");
        fprintf(fout, "This is a multi-part message in MIME format.\n");
                                                                                                                                            
        fprintf(fout, "--" MY_MIME_BOUNDARY "\n");
        fprintf(fout, "Content-Type: text/html; charset=gb2312\n");
        fprintf(fout, "Content-Transfer-Encoding: Base64\n\n");
                                                                                                                                            
        f_b64_ntop_init(tail_buf, &tail_len, &outlen);
        while (fgets(b64_in, sizeof (b64_in), fin) != NULL) {
                if (NULL != (attach = checkbinaryattach(b64_in, fin, &len))) {
                        if (len <= 0)
                                continue;
                        buf = malloc(len);
                        if (buf == NULL)
                                continue;
                        fread(buf, len, 1, fin);
                        f_b64_ntop_fini(fout, tail_buf, &tail_len);
                        fprintf(fout, "\n--" MY_MIME_BOUNDARY "\n");
                        mime_type = get_mime_type(attach);
                        fprintf(fout, "Content-Type: %s; name=%s\n", mime_type,
                                attach);
                        fprintf(fout, "Content-Transfer-Encoding: Base64\n\n");
                        f_b64_ntop_init(tail_buf, &tail_len, &outlen);
                        f_b64_ntop(fout, buf, len, tail_buf, &tail_len,
                                   &outlen);
                        f_b64_ntop_fini(fout, tail_buf, &tail_len);
                        fprintf(fout, "\n--" MY_MIME_BOUNDARY "\n");
                        fprintf(fout,
                                "Content-Type: text/html; charset=gb2312\n");
                        fprintf(fout, "Content-Transfer-Encoding: Base64\n\n");
                        f_b64_ntop_init(tail_buf, &tail_len, &outlen);
                        free(buf);
                        continue;
                }
                f_b64_ntop(fout, b64_in, strlen(b64_in), tail_buf, &tail_len,
                           &outlen);
        }
        f_b64_ntop_fini(fout, tail_buf, &tail_len);
        fprintf(fout, "\n--" MY_MIME_BOUNDARY "--\n");
        fprintf(fout, ".\n");
	
        fclose(fin);
        fclose(fout);
        return 0;
}

inline void
strnncpy2(char *s, int *l, const char *s2, int len)
{
        memcpy(s + (*l), s2, len);
        (*l) += len;
}

// 简易的ansi 2 html, 用不了nju09/BBSLIB.c 里的hsprintf(), 用javascript也不合适

void hsprintfLite(char *s, const char *s0)
{
	static const char specchar[256] = {['&'] = 1,['<'] = 1,['>'] = 1,[' '] = 1,
			['\n'] = 1, ['\r'] = 1};
	int c, i, len;
	static int dbchar = 0;
	int lastdb;

	if (s == NULL)
		return;
	len = 0;
	for (i = 0; (c = s0[i]); i++) {
		lastdb = dbchar;
		if (dbchar)
			dbchar = 0;
		else if (c & 0x80)
			dbchar = 1;

		if (!specchar[(unsigned char) c]) {
			s[len++] = c;
			continue;
		}
		if (lastdb && len) {
			len--;
		}
		switch (c) {
		case '&':
			strnncpy2(s, &len, "&amp;", 5);
			break;
		case '<':
			strnncpy2(s, &len, "&lt;", 4);
			break;
		case '>':
			strnncpy2(s, &len, "&gt;", 4);
			break;
		case ' ':
			if (!i || s0[i - 1] != ' ') {
				s[len++] = c;
			} else {
				strnncpy2(s, &len, "&nbsp;", 6);
			}
			break;
		case '\r':
			break;
		case '\n':
			strnncpy2(s, &len, "<br>\n", 5);
			break;
		default:
			s[len++] = c;
		}
	}
	s[len] = 0;
}

