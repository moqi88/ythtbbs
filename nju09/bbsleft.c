#include "bbslib.h"
#define ONLINELIMIT 7000
#define SEARCHID "002851670354406323673:j5jyxomeixu"

void
printdiv(int *n, char *str)
{
	printf("<div id=div%da class=r><A href='javascript:changemn(\"%d\");'>",
	       *n, *n);
	printf("<img border=0 id=img%d src=/folder.gif>%s</A></div>\n", *n,
	       str);
	printf("<div id=div%d class=s>\n", (*n)++);
}

void
printsectree(const struct sectree *sec)
{
	int i;
	for (i = 0; i < sec->nsubsec; i++) {
#if 0
		if (sec->subsec[i]->nsubsec)
			continue;
#endif
		printf("&nbsp; <a target=_top href=boa?secstr=%s>"
		       "%s%c</a><br>\n", sec->subsec[i]->basestr,
		       nohtml(sec->subsec[i]->title),
		       sec->subsec[i]->nsubsec ? '+' : ' ');
	}
}

int
bbsleft_main()
{
	int i;
	int div = 0;
//	changemode(MMENU);
	if (0)
		errlog("%s-%s-%s-%s", getsenv("HTTP_ACCEPT_LANGUAGE"),
		       getsenv("Accept"), getsenv("Accept-Charset"),
		       getsenv("Accept-Encoding"));
	html_header(2);
#if 0
	{
		char *ptr;
		char buf[256];
		ptr = getsenv("HTTP_USER_AGENT");
		sprintf(buf, "%-14.14s %.100s", currentuser->userid, ptr);
		addtofile(MY_BBS_HOME "/browser.log", buf);
	}
#endif
	printf("<script src=" BBSLEFTJS "></script>\n"
		"<script src=" BBSJS "></script>\n"
		"<script src=" BBSAJAXJS "></script>\n"
		"<script src=" BBSJSONJS "></script>\n"
		"<script src=" BBSBRDJS "></script>\n"
	       "<body onMouseOver='doMouseOver()' "
	       "onMouseEnter='doMouseOver()' "
	       "onMouseOut='doMouseOut()'>\n<nobr>");
	if (!loginok || isguest) {
		printf("<table width=100%%>\n");
/*由于去掉了框架，暂时去掉https的跳转支持
#ifdef HTTPS_DOMAIN
#ifdef USESESSIONCOOKIE
		printf("<form name=l action=https://%s/" SMAGIC "/bbslogin "
				"method=post target=_top>", 
				getsenv("HTTP_HOST"));
#else
		printf("<form name=l action=https://" HTTPS_DOMAIN
		       "/" SMAGIC "/bbslogin method=post target=_top>");
#endif
		printf("<input type=hidden name=usehost "
				"value='http%s://%s:%s'>",
		       strcasecmp(getsenv("HTTPS"), "ON")?"":"s",
		       getsenv("HTTP_HOST"), getsenv("SERVER_PORT"));
#else
		printf("<form name=l action=bbslogin method=post target=_top>");
#endif
*/
		printf("<form name=l action=bbslogin method=post target=_top>");
		printf("<tr><td>"
		       "<input type=hidden name=lastip1 value=''>"
		       "<input type=hidden name=lastip2 value=''>"
		       "帐号<input type=text name=id maxlength=%d size=11><br>"
		       "密码<input type=password name=pw maxlength=%d size=11><br>"
		       "<a href=/ipmask.html target=_blank>范围</a>"
			"<select name=ipmask style='width: 78px;'>\n"
		       "<option value=0 selected>单IP</option>\n"
		       "<option value=1>2 IP</option>\n"
		       "<option value=2>4 IP</option>\n"
		       "<option value=3>8 IP</option>\n"
		       "<option value=4>16 IP</option>\n"
		       "<option value=5>32 IP</option>\n"
		       "<option value=6>64 IP</option>\n"
		       "<option value=7>128 IP</option>\n"
		       "<option value=8>256 IP</option>\n"
		       "<option value=15>32K IP</option></select>"
		       "<input type=submit value=登录>&nbsp;&nbsp;"
		       "<input type=submit value=注册 onclick=\"{top.location.href='/"
		       SMAGIC "/bbsemailreg';return false}\">\n"
		       "</td></tr></form></table>\n", IDLEN, PASSLEN - 1);
	} else {
		char buf[256] = "未注册用户";
		printf
		    ("用户: <a href=bbsqry?userid=%s target=_top>%s</a><br>",
		     currentuser->userid, currentuser->userid);
		if (currentuser->userlevel & PERM_LOGINOK)
			strcpy(buf, cuserexp(currentuser->exp_group, countexp(currentuser, 2)));
		if (currentuser->userlevel & PERM_BOARDS)
			strcpy(buf, "版主");
		if (currentuser->userlevel & PERM_XEMPT)
			strcpy(buf, "永久帐号");
		if (currentuser->userlevel & PERM_SYSOP)
			strcpy(buf, "本站站长");
		printf("级别: %s<br>", buf);
		printf("<a href=bbslogout target=_top>注销本次登录</a><br>\n");
	}
	printf("<hr>");
	check_msg();  //如果有新消息，则print链接  
	printf("&nbsp;&nbsp;<a target=_top href=boa?secstr=?>一路bbs导读</a><br>\n");
	printf("&nbsp;&nbsp;<a href=\"/ku\" target=\"_blank\">一路精华区（新）</a><br>\n");
	printf("&nbsp;&nbsp;<a target=_top href=bbs0an>精华公布栏</a><br>\n");
	printf("&nbsp;&nbsp;<a target=_top href=bbstop10>十大热门话题</a><br>\n");
	printf("&nbsp;&nbsp;<a target=_top href=digest?C=0>近日精彩话题</a><br>\n");
#if ENABLE_BLOG
	printf("&nbsp; <a target=_blank href=blogpage>一路博客</a><br>\n");
#endif
//	printf("&nbsp; <a target==_BLANK href=http://yjrg.net/wiki><font color=brown>如故知识库</font></a><br>\n");
	if (loginok && !isguest) {
		char buf[10];
		unsigned int mybrdmode;
		readuservalue(currentuser->userid, "mybrd_mode", 
				buf, sizeof(buf));
		mybrdmode = atoi(buf);
		printdiv(&div, "订阅讨论区");
		bbsmybrd_show_left(mybrdmode);
		printf
		    ("&nbsp;&nbsp;<a target=_top href=bbsboa?secstr=*>预定区总览</a><br>\n");
		printf
		    ("&nbsp;&nbsp;<a target=_top href=bbsmybrd?mode=1>预定管理</a><br>\n");
		printf("</div>\n");
	}
	printdiv(&div, "分类讨论区");
	printsectree(&sectree);
	printf("</div>\n");
#if 0
	printf("<div class=r>");
	for (i = 0; i < sectree.nsubsec; i++) {
		const struct sectree *sec = sectree.subsec[i];
		if (!sec->nsubsec)
			continue;
		printf
		    ("--<a target=_top href=bbsboa?secstr=%s>%s</a><br>\n",
		     sec->basestr, sec->title);
	}
	printf("</div>\n");
#endif
	printdiv(&div, "谈天说地");
	if (loginok && !isguest) {
		printf
		    ("&nbsp;&nbsp;<a href=bbsfriend target=_top>在线好友</a><br>\n");
	}
//      printf
//          ("&nbsp;&nbsp;<a href=bbsufind?search=A&limit=20 target=_top>环顾四方</a><br>\n");
	printf("&nbsp;&nbsp;<a href=bbsqry target=_top>查询网友</a><br>\n");
	if (currentuser->userlevel & PERM_PAGE) {
		printf
		    ("&nbsp;&nbsp;<a href=bbssendmsg target=_top>发送短消息</a><br>\n");
		printf
		    ("&nbsp;&nbsp;<a href=bbsmsg target=_top>查看所有短消息</a><br>\n");
	}
	printf("</div>\n");
	if (loginok && !isguest) {
#ifdef HTTPS_DOMAIN
		char str[STRLEN + 10], *ptr;
		//char taskfile[256];
#endif
		printdiv(&div, "个人设置");
#ifdef HTTPS_DOMAIN
		strsncpy(str, getsenv("SCRIPT_URL"), STRLEN);
		ptr = strrchr(str, '/');
		if (ptr)
			strcpy(ptr, "/bbspwd");
		printf("&nbsp;&nbsp;<a target=_top href=https://" HTTPS_DOMAIN
		       "%s>修改密码</a><br>", str);
#else
		printf("&nbsp;&nbsp;<a target=_top href=bbspwd>修改密码</a><br>");
#endif
		printf("&nbsp;&nbsp;<a target=_top href=bbsinfo>个人资料和头像</a><br>"
		       "&nbsp;&nbsp;<a target=_top href=bbsplan>改说明档</a><br>"
		       "&nbsp;&nbsp;<a target=_top href=bbssig>改签名档</a><br>"
		       "&nbsp;&nbsp;<a target=_top href=bbsparm>修改个人参数</a><br>"
		       "&nbsp;&nbsp;<a target=_top href=bbsmywww>WWW个人定制</a><br>"
		       "&nbsp;&nbsp;<a target=_top href=bbsmyclass>底栏显示的版面</a><br>"
		       "&nbsp;&nbsp;<a target=_top href=bbsnick>临时改昵称</a><br>"
		       "&nbsp;&nbsp;<a target=_top href=bbsstat>排名统计</a><br>"
		       "&nbsp;&nbsp;<a target=_top href=bbsfall>设定好友</a><br>");

		if (currentuser->userlevel & PERM_CLOAK)
			printf("&nbsp;&nbsp;<a target=_top "
			       "onclick='return confirm(\"确实切换隐身状态吗?\")' "
			       "href=bbscloak>切换隐身</a><br>\n");
		printf("</div>");
		printdiv(&div, "处理信件");
		printf("&nbsp;&nbsp;<a target=_top href=bbsnewmail>未读邮件</a><br>"
		       "&nbsp;&nbsp;<a target=_top href=bbsmail>所有邮件</a><br>"
		       "&nbsp;&nbsp;<a target=_top href=bbspstmail>发送邮件</a><br>"
		       "</div>");
//		       "&nbsp;&nbsp;<a target=_top href=bbsspam>垃圾邮件</a><br>"

	}
	printdiv(&div, "特别服务");
	//printf("&nbsp;&nbsp;<a target=_top href=bbssechand>二手市场</a><br>\n");
	printf("&nbsp;&nbsp;<a target=_top href=/wnl.html>万年历</a><br>\n");
	//以下特别服务里注释的地方暂时坏了或者没有打开
	//printf("&nbsp;&nbsp;<a target=_top href=/cgi-bin/cgincce>科技词典</a><br>\n");
	printf
	    ("&nbsp;&nbsp;<a target=_top href=/scicalc.html>科学计算器</a><br>\n");
	//printf("&nbsp;&nbsp;<a target=_top href=/periodic/periodic.html>元素周期表</a><br>\n");
	//printf("&nbsp;&nbsp;<a target=_top href=/cgi-bin/cgiman>Linux手册查询</a><br>\n");
	printf("&nbsp;&nbsp;<a href=bbsfind target=_top>文章查询</a><br>\n");
	//printf("&nbsp;&nbsp;<a target=_top href=/cgi-bin/cgifreeip>IP地址查询</a><br>\n");
	//printf("&nbsp;&nbsp;<a target=_top href=bbs0an>精华公布栏</a><br>\n");
	//printf("&nbsp;&nbsp;<a target=_top href=bbsx?chm=1>下载精华区</a><br>\n");
	//printf("&nbsp;&nbsp;<a target=_top href=http://%s/tools/telnet_tools/index.html>" "Telnet工具下载</a><br>\n", getsenv("HTTP_HOST"));
	printf("</div>\n");
if (loginok && !isguest &&
                    (currentuser->userlevel & PERM_DEFAULT) == PERM_DEFAULT) {
	printdiv(&div, "我的地盘");
  if (*system_load() >= 1.7 || count_online() > ONLINELIMIT)
	     printf("&nbsp;&nbsp;我的帖子（系统负载过高，暂停使用）<br>\n");
  else
  	   printf("&nbsp;&nbsp;<a target=_top href=bbsfind?user=%s&amp;day=180>我的帖子</a><br>\n",currentuser->userid);
#ifdef ENABLE_BLOG  	   
  if (currentuser->hasblog)
	printf("&nbsp;&nbsp;<a target=_top href=blog?U=%s>我的Blog</a><br>\n",currentuser->userid);
#endif
		if (isalpha(currentuser->userid[0]) && (currentuser->userlevel & PERM_SPECIAL8)) {
		printf
		    ("&nbsp;&nbsp;<a target=_top href=bbs0an?path=/groups/GROUP_0/Personal_Corpus/%c/%s>我的文集</a>",
		     toupper(currentuser->userid[0]), currentuser->userid);
	}
}
	printf("</div>\n");
	printf("<div class=r>");
//	printf("&nbsp;&nbsp;<a target=_top href=bbs0an?path=/groups/GROUP_0/Personal_Corpus>个人文集区</a><br>\n");
	printf("&nbsp;&nbsp;<a target=_top href=bbsall>所有讨论区</a><br>\n");
	#ifdef ENABLE_INVITATION
                if (loginok && !isguest &&
                    (currentuser->userlevel & PERM_DEFAULT) == PERM_DEFAULT)
                        printf
                            ("&nbsp;&nbsp;<a target=_top href=bbsinvite>邀请朋友</a><br>");
	#endif

	bbsadv_show(2);
	printf("<hr>");
	printf("<table width=100%%><tr><form action=bbssearchboard method=post target=_top><td><div align=center>"
		"<input type=text style='width:100px' name=match maxlength=24 "
		"size=9 value=搜索讨论区 onclick=\"this.select()\" align=left></div></td></form></tr></table>\n");
	printf("<hr>");
	printf("&nbsp;&nbsp;<a href='telnet:%s'>Telnet登录</a><br>\n", BBSHOST);
	printf("&nbsp;&nbsp;<a target=_top href=home?B=BBSHelp>用户帮助</a>\n");
#if 0	/* 先去掉，有空改成再发送一次注册确认信 */
	if (loginok && !isguest && !(currentuser->userlevel & PERM_LOGINOK)
	    && !has_fill_form(currentuser->userid))
		printf
		    ("<br>&nbsp;&nbsp;<a target=_top href=bbsform><font color=red>填写注册单</font></a>\n");
#endif
	if (loginok && !isguest && USERPERM(currentuser, PERM_ACCOUNTS))
		printf
		    ("<br>&nbsp;&nbsp;<a href=bbsscanreg target=_top>SCANREG</a>");
	if (loginok && !isguest && USERPERM(currentuser, PERM_SYSOP))
		printf("<br>&nbsp;&nbsp;<a href=kick target=_top>踢www下站</a>");
	//if(loginok && !isguest) printf("<br>&nbsp;&nbsp;<a href='javascript:openchat()'>bbs茶馆</a>");
	printf
	    ("<br>&nbsp;&nbsp;<a href=bbsselstyle target=_top>换个界面看看</a>");
	printf("<br>&nbsp;&nbsp;当前在线[%d] ", count_online());
	printf("<hr />\n");
	printf("<!-- SiteSearch Google --><form method=\"get\" action=\"http://www.google.com/custom\" target=\"google_window\"><table border=\"0\"><tr><td nowrap=\"nowrap\" valign=\"top\" align=\"left\" height=\"32\"></td><td nowrap=\"nowrap\"><input type=\"hidden\" name=\"domains\" value=\"yilubbs.com\"></input><label for=\"sbi\" style=\"display: none\">输入您的搜索字词</label><input type=\"text\" name=\"q\" size=\"15\" maxlength=\"255\" value=\"\" id=\"sbi\"></input></td></tr><tr><td>&nbsp;</td><td nowrap=\"nowrap\"><table><tr><td><input type=\"radio\" name=\"sitesearch\" value=\"yilubbs.com\" id=\"ss1\" checked></input><label for=\"ss1\" title=\"搜索 yilubbs.com\"><font size=\"-1\" color=\"#000000\">yilubbs.com</font></label></td><td></td></tr></table><label for=\"sbb\" style=\"display: none\">提交搜索表单</label><input type=\"submit\" name=\"sa\" value=\"Google 搜索\" id=\"sbb\"></input><input type=\"hidden\" name=\"client\" value=\"pub-7608613947207155\"></input><input type=\"hidden\" name=\"forid\" value=\"1\"></input><input type=\"hidden\" name=\"ie\" value=\"GB2312\"></input><input type=\"hidden\" name=\"oe\" value=\"GB2312\"></input><input type=\"hidden\" name=\"cof\" value=\"GALT:#008000;GL:1;DIV:#FFFFFF;VLC:663399;AH:center;BGC:FFFFFF;LBGC:FFFFFF;ALC:0000FF;LC:0000FF;T:000000;GFNT:0000FF;GIMP:0000FF;LH:50;LW:110;L:http://www.yilubbs.com/ku/images/logo.gif;S:http://www.yilubbs.com;FORID:1\"></input><input type=\"hidden\" name=\"hl\" value=\"zh-CN\"></input></td></tr></table></form><!-- SiteSearch Google -->");
	/*
	printf("<hr />\n"
		"<!-- Google CSE Search Box Begins -->\n"
		"<div align=center>"
		"<form action=\"/fulltext.htm\" id =\"" SEARCHID "\" "
		"target=_blank>\n"
		"<input type=\"hidden\" name=\"cx\" value=\"" SEARCHID "\" />\n"
		"<input type=\"hidden\" name=\"cof\" value=\"FORID:11\" />\n"
		"<input type=\"text\" name=\"q\" size=\"14\" />\n<br>\n"
		"<input type=\"submit\" name=\"sa\" value=\"全文搜索\" />\n"
		"</form>"
		"<script type=\"text/javascript\" src=\"http://"
		"www.google.com/coop/cse/brand?form=searchbox_" SEARCHID 
		"\">\n</script>\n"
		"</div>"
		"<!-- Google CSE Search Box Ends -->");
	*/
	if (1 || strcmp(MY_BBS_ID, "YTHT"))
		printf("<br><br><center><img src=/coco.gif>");
	else {
		printf
		    ("<br><center><a href=http://www.cbe-amd.com target=_blank><img src=/cbe-amd.gif border=0></a>");
		printf
		    ("<br><center><a href=http://www.amdc.com.cn/products/cpg/amd64/ target=_blank><img src=/AMD64_logo.gif border=0></a>");
	}
	printf ("<br><a target=_top href=bug ><strong>报告Bug</strong></a>");
	printf("</div>");
	printf("<script>if(isNS4) arrange();if(isOP)alarrangeO();</script>");
	if (loginok && !isguest) {
		if (USERPERM(currentuser, PERM_LOGINOK)
		    && !USERPERM(currentuser, PERM_POST))
			printf
			    ("<script>alert('您被封禁了全站发表文章的权限, 请参看sysop版公告, 期满后在sysop版申请解封. 如有异议, 发信给arbitration帐号投诉)</script>\n");
		mails(currentuser->userid, &i);
		if (i > 0)
			printf("<script>alert('您有新信件!')</script>\n");
	}
	// if(loginok&&currentuser.userdefine&DEF_ACBOARD)
	//              printf("<script>window.open('bbsmovie','','left=200,top=200,width=600,height=240');</script>"); 
	//virusalert();
	if (isguest && 0)
		printf
		    ("<script>setTimeout('open(\"regreq\", \"winREGREQ\", \"width=600,height=460\")', 1800000);</script>");
	if (loginok && !isguest) {
		char filename[80];
		sethomepath(filename, currentuser->userid);
		mkdir(filename, 0755);
		sethomefile(filename, currentuser->userid, BADLOGINFILE);
		if (file_exist(filename)) {
			printf("<script>"
			       "window.open('bbsbadlogins', 'badlogins', 'toolbar=0, scrollbars=1, location=0, statusbar=1, menubar=0, resizable=1, width=450, height=300');"
			       "</script>");
		}
	}
	bbsadv_show(0);
	if (!via_proxy && wwwcache->text_accel_port
	    && wwwcache->text_accel_addr.s_addr)
		printf("<script src=http://proxy.%s:%d/testdoc.js></script>",
		       MY_BBS_DOMAIN,
		       wwwcache->text_accel_port);
	else if (via_proxy)
		w_info->doc_mode = 0;
	//printf("<script src=/testdoc.js></script>");
	if (!loginok || isguest)
		setlastip();
	printf("</body></html>");
	return 0;
}

/*
 * void
virusalert()
{
	if (file_has_word("virusalert.txt", realfromhost)) {
		printf
		    ("<script>window.open('/virusalert.html','','left=200,top=200,width=250,height=80');</script>");
	}
}
*/
