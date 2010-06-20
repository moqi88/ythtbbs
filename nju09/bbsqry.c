#include "bbslib.h"

int
bbsqry_main()
{
	FILE *fp;
	char userid[14], filename[80], buf[512];
	struct userec *x;
	int tmp2;
	struct in_addr in;
	html_header(1);
	//check_msg();
	changemode(QUERY);
	strsncpy(userid, getparm("U"), 13);
	if (!userid[0])
		strsncpy(userid, getparm("userid"), 13);
//	printf("<body><center>");
//修改框架，增加左侧
	printf("<body topmargin=0 leftMargin=1 MARGINWIDTH=1 MARGINHEIGHT=0>" WWWLEFT_DIV);
	printf("<center>");

	printf("%s -- 查询网友<hr>\n", BBSNAME);
	if (userid[0] == 0) {
		printf("<form name=qry action=bbsqry>\n");
		printf
		    ("请输入用户名: <input name=userid maxlength=12 size=12>\n");
		printf("<input type=submit value=查询用户>\n");
		printf("</form><hr>\n");
		printf("<script>document.qry.userid.focus();</script>");
		showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
		http_quit();
	}
	if (getuser(userid, &x) <= 0) {
		printf("不可能，肯定是你敲错了，根本没这人啊");
		printf("<p><a href=javascript:history.go(-1)>快速返回</a>");
		showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
		http_quit();
	}
	printf("</center><pre style='font-size:14px'>\n");
	if (x->mypic) {
		printf("<table align=left><tr><td><center>");
		printmypic(x->userid);
		printf("</center></td></tr></table>");
	}
	sprintf(buf,
		"%s (\033[32m%s\033[37m) 共上站 \033[32m%d\033[m 次，发表文章 \033[32m%d\033[m 篇",
		x->userid, x->username, x->numlogins, x->numposts);
	hprintf("%s", buf);
//      show_special(x->userid);

	printf("\n");
	if (!USERPERM(currentuser, PERM_SYSOP))
		in.s_addr = x->lasthost & IPMASK;
	else
		in.s_addr = x->lasthost;
	printf( "上次在 <font color=green>%s</font> 从 <font color=green>%s</font> 到本站一游。",
	     Ctime(x->lastlogin), inet_ntoa(in));
	mails(userid, &tmp2);
	printf("信箱：[<font color=green>%s</font>]<br>", tmp2 ? "⊙" : "  ");
	//if (!strcasecmp(x->userid, currentuser->userid)) {
		printf("经验值：[<font color=purple>%d+%d</font>](<font color=olive>%s</font>) ",
			(x->exp_group=='U')?0:countexp(x,0), (x->exp_group=='U')?0:countexp(x,1),cuserexp(x->exp_group, countexp(x,2)));
		printf("表现值：[<font color=green>%d</font>](<font color=olive>%s</font>) ",
			countperf(x), cperf(countperf(x)));
	//}
	if (x->dieday) {
		printf
		    ("<br>已经离开了人世,呜呜...<br>还有 [<b>%d</b>] 天就要转世投胎了<br>",
		     countlife(x));
	} else {
		printf("生命力：[<font color=red>%d</font>]。<br>", countlife(x));
		if( USERPERM(x, PERM_SYSOP))
			printf("职务：<font color=olive>站务总管  </font>");
		else if (USERPERM(x, PERM_ARBITRATE))
			printf("职务：<font color=olive>仲裁  </font>");
		else if (USERPERM(x, PERM_SPECIAL4))
			printf("职务：<font color=olive>分区站务 </font>");
		else if (USERPERM(x, PERM_EXT_IDLE))
                        printf("职务：<font color=olive>执法站务 </font>");
		else if (USERPERM(x, PERM_SPECIAL5))
			printf("职务：<font color=olive>美工站务  </font>");
		else if (USERPERM(x, PERM_SPECIAL6))
			printf("职务：<font color=olive>立法会  </font>");

		if (x->userlevel & PERM_BOARDS) {
			int i;
			printf("担任版务：");
			for (i = 0; i < MAXBOARD && i < shm_bcache->number; i++)
				bm_printboard(&shm_bcache->bcache[i],
					      x->userid);
			printf("<br>");
		}
		else if(USERPERM(x, PERM_SPECIAL4) || USERPERM(x, PERM_EXT_IDLE) || USERPERM(x, PERM_SPECIAL5) || USERPERM(x, PERM_SPECIAL6) || USERPERM(x, PERM_SYSOP))
		printf("<br>");
		
		if (!show_onlinestate(userid)) {
			printf("目前不在站上, 上次离站时间 [<font color=blue>%s</font>]\n\n",
			     (x->lastlogout >=
			      x->lastlogin) ? Ctime(x->lastlogout) :
			     "因在线上或不正常断线不详");
		}
	}
	printf("\n");
	printf("</pre><table width=100%%><tr><td class=f2>");
	sethomefile(filename, x->userid, "plans");
	fp = fopen(filename, "r");
	sprintf(filename, "00%s-plan", x->userid);
	fdisplay_attach(NULL, NULL, NULL, NULL);
	if (fp) {
		while (1) {
			if (fgets(buf, 256, fp) == 0)
				break;
			if (!strncmp(buf, "begin 644 ", 10)) {
				errlog("old attach %s", filename);
				fdisplay_attach(stdout, fp, buf, filename);
				continue;
			}
			fhhprintf(stdout, "%s", buf);
		}
		fclose(fp);
	} else {
		printf("<font color=teal>没有个人说明档</font><br>");
	}

	printf("</td></tr></table>");
	printf
	    ("<br><br><a href=bbspstmail?userid=%s&title=没主题>[给我发信]</a> ",
	     x->userid);
	printf("<a href=bbssendmsg?destid=%s>[发送短消息]</a> ", x->userid);
	printf("<a href=bbsfadd?userid=%s>[加入好友]</a> ", x->userid);
	printf("<a href=bbsfdel?userid=%s>[删除好友]</a> ", x->userid);
	if (isalpha(x->userid[0]) && (x->userlevel & PERM_SPECIAL8)) {
		printf
		    ("<a href=bbs0an?path=/groups/GROUP_0/Personal_Corpus/%c/%s>[个人文集]</a>",
		     toupper(x->userid[0]), x->userid);
	}
#ifdef ENABLE_BLOG
	if (x->hasblog) {
		printf(" <a href=blog?U=%s>[Blog]</a>", x->userid);
	}

#endif
	if(loginok && !isguest) {
		sprintf(buf, "1 %s %s", currentuser->userid, x->userid);
		friendslog(buf);
	}

	printf("<hr>");
	printf("<center><form name=qry action=bbsqry>\n");
	printf("请输入用户名: <input name=userid maxlength=12 size=12>\n");
	printf("<input type=submit value=查询用户>\n");
	printf("</form><hr>\n");
	showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
//	printf("</body>\n");
//修改框架，页面底部
	printf(WWWFOOT_DIV "</body></html>\n");
	http_quit();
	return 0;
}

void
show_special(char *id2)
{
	FILE *fp;
	char id1[80], name[80];
	fp = fopen("etc/sysops", "r");
	if (fp == 0)
		return;
	while (1) {
		id1[0] = 0;
		name[0] = 0;
		if (fscanf(FCGI_ToFILE(fp), "%s %s", id1, name) <= 0)
			break;
		if (!strcmp(id1, id2))
			printf(" <font color=red>★</font><font color=olive>%s</font><font color=red>★</font>",
				name);
	}
	fclose(fp);
}

int
bm_printboard(struct boardmem *bmem, char *who)
{
	if (chk_BM_id(who, &bmem->header) && has_view_perm_x(currentuser, bmem)) {
		printf("<a href=doc?B=%d>", getbnumx(bmem));
		printf("%s", bmem->header.filename);
		printf("</a> ");
	}
	return 0;
}

int
show_onlinestate(char *userid)
{
	int uid, i, uent, num = 0;
	struct user_info *uentp;
	uid = getuser(userid, NULL);
	if (uid <= 0 || uid > MAXUSERS)
		return 0;
	for (i = 0; i < 6; i++) {
		uent = uindexshm->user[uid - 1][i];
		if (uent <= 0)
			continue;
		uentp = &(shm_utmp->uinfo[uent - 1]);
		if (!uentp->active || !uentp->pid || uentp->uid != uid)
			continue;
		if (uentp->invisible && !USERPERM(currentuser, PERM_SEECLOAK))
			continue;
		num++;
		if (num == 1)
			printf("目前在站上, 状态如下:\n");
		if (uentp->invisible)
			printf("<font color=olive>C</font>");
		printf("<font color=%s>%s</font> ", uentp->pid == 1 ? "magenta" : "blue",
			ModeType(uentp->mode));
		if (num % 5 == 0)
			printf("<br>");
	}
	return num;
}
