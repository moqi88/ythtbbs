#include "bbslib.h"

#define ONEFACEPATH "/face"

#define NFACE 4
struct wwwface {
	char *bgcolor;
	char *color;
	char *figure;
	char *stamp;
	char *logo;
	char *board;
};
static struct wwwface *pface;
static struct wwwface oneface = {
	NULL, NULL, NULL, NULL, NULL
};
static struct wwwface bbsface[NFACE] = {
	{"#000000", "#99ccff", "/ytht2men.jpg", NULL, NULL},
	{"white", "#99ccff", "/ythtBlkRedGry.gif", NULL, NULL},
	{"white", "black", "/cai.jpg", "/stamp.gif", "/logo.gif"},
	{"white", "black", "/cai.gif", "", "/yjrg.gif"}
};

int
checkfile(char *fn, int maxsz)
{
	char path[456];
	int sz;
	sprintf(path, HTMPATH "%s", fn);
	sz = file_size(path);
	if (sz < 100 || sz > maxsz)
		return -1;
	return 0;
}

int
loadoneface()
{
	FILE *fp;
	static char buf[256], figure[256 + 100], stamp[356], logo[356];
	char *ptr;

	fp = fopen(HTMPATH ONEFACEPATH "/config", "r");
	if (!fp)
		return -1;
	if (fgets(buf, sizeof (buf), fp) == NULL) {
		fclose(fp);
		return -1;
	}
	fclose(fp);
	ptr = buf;
	oneface.bgcolor = strsep(&ptr, " \t\r\n");
	oneface.color = strsep(&ptr, " \t\r\n");
	oneface.figure = strsep(&ptr, " \t\r\n");
	oneface.stamp = strsep(&ptr, " \t\r\n");
	oneface.logo = strsep(&ptr, " \t\r\n");
	oneface.board = strsep(&ptr, " \t\r\n");
	if (!oneface.logo)
		return -2;
	if (strstr(oneface.figure, "..") ||
	    strstr(oneface.stamp, "..") || strstr(oneface.logo, ".."))
		return -3;
	sprintf(figure, ONEFACEPATH "/%s", oneface.figure);
	oneface.figure = figure;
	if (checkfile(figure, 51200))	//回头减小到15K，不要把这个数字改大！
		return -4;
	if (!strcasecmp(oneface.stamp, "NULL"))
		oneface.stamp = NULL;
	else {
		sprintf(stamp, ONEFACEPATH "/%s", oneface.stamp);
		oneface.stamp = stamp;
		if (checkfile(stamp, 4000))
			return -5;
	}
	if (!strcasecmp(oneface.logo, "NULL"))
		oneface.logo = NULL;
	else {
		sprintf(logo, ONEFACEPATH "/%s", oneface.logo);
		oneface.logo = logo;
		if (checkfile(logo, 6500))
			return -6;
	}
	if (oneface.board && (!strcasecmp(oneface.board, "NULL")
			      || !strcasecmp(oneface.board, "Painter")))
		oneface.board = NULL;
	return 0;
}

int
showannounce()
{
      static struct mmapfile mf = { ptr:NULL };
	if (mmapfile("0Announce/announce", &mf) < 0 || mf.size <= 10)
		return -1;
	printf("<br><br><fieldset style=\"width: 85%%\">"
			"<legend align=\"center\" class=\"textstyle\">:: " MY_BBS_NAME "公告 ::</legend>"
			"<div style=\"padding: 10px;\">");
	fwrite(mf.ptr, mf.size, 1, stdout);
	printf("</div></fieldset>");
	if (mmapfile("0Announce/ICP", &mf) <0 || mf.size <=10 )
		return -1;
	fwrite(mf.ptr, mf.size, 1, stdout);
	return 0;
}

void
loginwindow()
{

//int n=random()%NFACE;
	char *scripturl;
	int n = 2, err;
	
	if (!(err = loadoneface()))
		pface = &oneface;
	else {
		if(!strcmp(MY_BBS_ID, "YJRG"))
			n = 3;
		pface = &(bbsface[n]);
	}
	html_header(4);
	printf("<title>%s %s</title>\n", MY_BBS_NAME, BBS_WWW_INTRO);
	
	scripturl = getsenv("SCRIPT_URL");
	/*if(!strcmp(MY_BBS_ID, "YJRG") &&  atoi(getparm("jump")) == 0) {
		printf("<body>\n<script>i=1\n"
		       "var autourl=new Array()\n"
		       "autourl[1]=\"http://yjrg.net%s?jump=1\"\n"
		       "autourl[2]=\"http://proxy.yjrg.net%s?jump=1\"\n"
		       "function choose(url) { if(i) { top.location=url; i=0;}}\n"
		       "function run() {\n for(var i=1;i<autourl.length;i++)\n "
		       "document.write(\"<img src=\"+autourl[i]+\" width=1 height=1 "
		       "onerror=choose('\"+autourl[i]+\"')>\");} run(); \n</script>\n"
		       "</body>\n</html>", scripturl, scripturl);
		return ;
	}*/
	printf("<style type=\"text/css\">\n"
	       ".textstyle{font-family:tahoma;font-size:12px;color:%s;}\n"
	       ".textstylebold{font-family:tahoma;font-size:12px;color:%s;font-weight:bold;}\n"
	       ".inputbox{border-style:solid;border-width:1px 1px 1px 1px;color:#336699;font-family:tahoma;font-size:12px;}"
	       ".button{border-width:0px 0px 0px 0px;background-color: #ffffff;color:#000000;font-family:tahoma;font-size:12px;position:relative;top:1px;}"
	       "a{text-decoration:none;color:%s;}\n"
	       "a:hover{color:red;}\n"
//	       "a:visited{color:%s;}\n"
	       "body{background-color:%s;}\n"
	       "</style>\n", pface->color, pface->color, 
	       pface->color, pface->bgcolor);
	printf("\n<!-- ERR: %d -->\n", err);
	printf(
		      //"<STYLE type=text/css>*{ font-family:arial,sans-serif;}\n"
		      //     "A{COLOR: %s; text-decoration: none;}"
		      //     "</STYLE>\n"
		      "<script>function sf(){document.l.id.focus();}\n"
		      "function st(){document.l.t.value=(new Date()).valueOf();}\n"
		      "function lg(){self.location.href='/" SMAGIC
		      "/bbslogin?id=guest&ipmask=8&t='+(new Date()).valueOf();}\n"
		      "</script>\n"
		      "</head>\n<BODY text=%s leftmargin=1 MARGINWIDTH=1>\n<br><br>"
		      "<center>", pface->color);
	if (pface->board != NULL) {
		if (strchr(pface->board, '.')) {
			printf("<a href=%s target=_blank>", pface->board);
			printf
			    ("<IMG src=%s border=0 alt='进入 %s'"
			     "onload=\"this.width = (this.width > document.width - 20)?"
			     "(document.width - 20):this.width;\" id=\"index_pic\">",
			     pface->figure, pface->board);
		} else {
			printf("<a href=http://%s." MY_BBS_DOMAIN ">",
			       pface->board);
			printf
			    ("<IMG src=%s border=0 alt='进入 %s 讨论区' "
			     "onload=\"this.width = (this.width > document.width - 20)?"
			     "(document.width - 20):this.width;\" id=\"index_pic\">",
			     pface->figure, pface->board);
		}
		printf("</a>");
	}

	else
		printf("<IMG src=%s border=0 alt='进站画面' "
				"onload=\"this.width = (this.width > document.width -20)?"
				"(document.width - 20):this.width;\" id=\"index_pic\">",
		       pface->figure);
	//if (pface->stamp)
	//      printf
	//          ("<table width=75%%><tr><td align=right><IMG src=%s border=0 alt=''></td></tr></table>",
	//           pface->stamp);
	printf
	    ("<table align=\"center\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"textstyle\"><tr><td>");
	printf("<form name=l action=/" SMAGIC
	       "/bbslogin method=post>");
	printf("<input type=hidden name=usehost value='http%s://%s:%s'>",
	       strcasecmp(getsenv("HTTPS"), "ON") ? "" : "s",
	       getsenv("HTTP_HOST"), getsenv("SERVER_PORT"));
	printf
	    ("<table class=\"textstyle\" align=\"center\" cellpadding=\"0\" cellspacing=\"0\"><tr><td>");
	if (pface->logo)
		printf("<IMG src=%s border=0 alt=''></td><td>", pface->logo);
	printf("<td>用户 <input class=\"inputbox\" maxLength=%d size=8 name=id>"
	       " 密码 <input class=\"inputbox\" type=password maxLength=%d size=8 name=pw>"
#ifdef HTTPS_DOMAIN
	       " <img name=ssl src=small.gif>"
#endif	       
	       "<input type=submit value=登录>"
	       "<input type=hidden name=t value=''>"
	       "<input type=hidden name=lastip1 value=''>"
	       "<input type=hidden name=lastip2 value=''></td><td>&nbsp;"
#ifndef USEBIG5
	       "<a href=/ipmask.html target=_blank>验证范围</a></td><td><select name=ipmask>"
#else
	       "<a href=/big5ipmask.html target=_blank>验证范围</a></td><td><select name=ipmask class=\"textstyle\">"
#endif
	       "<option value=0 selected>单IP</option>"
	       "<option value=3>8 IP</option>"
	       "<option value=4>16 IP</option>"
	       "<option value=5>32 IP</option>"
	       "<option value=6>64 IP</option>"
	       "<option value=7>128 IP</option>"
	       "<option value=8>256 IP</option>"
	       "<option value=15>32K IP</option>"
	       "</select></td><td>", IDLEN, PASSLEN - 1);
	if (strcmp(MY_BBS_ID, "YTHT"))
		printf("&nbsp;&nbsp;<a href=/" SMAGIC "/bbsemailreg class=red>注册</a>&nbsp;"
		       "<a href='javascript:lg();'>进入本站</a>"
		       "</td></tr></table></form>");
	printf("</td></tr></table>\n" "<script>sf();st();</script>");
	printf("<br><div class=\"textstyle\"><center>");
	if (0 && !strcmp(MY_BBS_ID, "YTHT"))
		printf
		    ("<a href=http://www.cbe-amd.com target=_blank><u>中基超威</u></a>提供 <a href=http://www.amdc.com.cn/products/cpg/amd64/ target=_blank><u>AMD64</u></a> 运算支持<br>");
	printf("<script>function langpage(lan) { "
	       "var p=lan;if(lan=='GB')p='';"
	       "exDate = new Date;exDate.setMonth(exDate.getMonth()+9);"
	       "document.cookie='uselang='+lan+';path=/;expires=' + exDate.toGMTString();"
	       "self.location.replace('/" BASESMAGIC
	       "'+p+'/?jump=1');}</script>");
	printf("<a href=\"/distribute.htm\">自动选择最快连接</a>　");
	printf("<a href=\"javascript:langpage('GB');\">简体版[GB]</a>　"
	       "<a href=\"javascript:langpage('BIG5');\">繁体版[BIG5]</a>　");
#ifdef HTTPS_DOMAIN
	printf("<a href=\"https://" HTTPS_DOMAIN "/\">安全传输[HTTPS]</a>　");
#endif
	printf("<!--a href='telnet://" MY_BBS_DOMAIN "'>Telnet登录" MY_BBS_ID
	       "</a>　"
	       "<a href=\"javascript:window.external.AddFavorite('http://"
	       MY_BBS_DOMAIN "/','◆" MY_BBS_LOC MY_BBS_NAME "◆')\">"
	       "将本站加入收藏夹</a>　<a href=\"mailto:" ADMIN_EMAIL
	       "\">联系站务组</a-->");
#ifdef HTTPS_DOMAIN
	//printf("<script src=https://" HTTPS_DOMAIN "/" SMAGIC "/tssl/%d_%d.js></script>",
	//		random(), now_t);
#endif
	printf("<a href=/yjrg_ad.xls >广告合作</a>");
	//if (!strcmp(MY_BBS_ID, "YTHT"))
	//      printf("　<a href=/ythtsearch.htm target=_blank>搜索本站</a>");
	showannounce();
	printf("</center></div>");
	setlastip();
	printf("</CENTER>");
	showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
	printf("</BODY></HTML>");
}

int
setlastip()
{
	printf("<script>document.write('<script src=/" SMAGIC
	       "/bbslastip?n=1&t='+(new Date()).valueOf()+'><\\/script>');\n"
	       "document.write('<script src=/" SMAGIC
	       "/bbslastip?n=2&t='+(new Date()).valueOf()+'><\\/script>');</script>");
	return 0;
}

void
shownologin()
{
	int n = 0;
      static struct mmapfile mf = { ptr:NULL };
	html_header(4);
	printf
	    ("<STYLE type=text/css>A{COLOR: #99ccff; text-decoration: none;}</STYLE>"
	     "</head><BODY text=#99ccff bgColor=%s leftmargin=1 MARGINWIDTH=1><br>"
	     "<CENTER>", bbsface[n].bgcolor);
	printf("<IMG src=%s border=0 alt='' width=70%%><BR>",
	       bbsface[n].figure);
	printf("<b>停站通知</b><br>");
	if (!mmapfile("NOLOGIN", &mf))
		fwrite(mf.ptr, mf.size, 1, stdout);
	printf("</CENTER></BODY></HTML>");
	return;
}

void
checklanguage()
{
	char *scripturl = getsenv("SCRIPT_URL");
	int tobig = 0;
	while (!strcmp("/", scripturl)) {
		char lang[6];
		char *langcookie = getparm("uselang");
		if (!strcmp(langcookie, "GB"))
			return;
		if (!strcmp(langcookie, "BIG5")) {
			tobig = 1;
			break;
		}
		strsncpy(lang, getsenv("HTTP_ACCEPT_LANGUAGE"), 6);
		if (!strncasecmp(lang, "zh-tw", 5)
		    || !strncasecmp(lang, "zh-hk", 5)
		    || !strncasecmp(lang, "zh-mo", 5)) {
			tobig = 1;
		}
		break;
	}
	if (!tobig)
		return;
	errlog("哦，有繁体使用繁体的？%s", realfromhost);
	html_header(3);
	redirect(BIG5FIRSTPAGE);
	http_quit();

}

int
bbsindex_main()
{
	char str[20], redbuf[50];
	char *t, *b;
	t = getparm("t");
	b = getparm("b");
	if (nologin) {
		shownologin();
		http_quit();
		return 0;
	}

#ifdef USESESSIONCOOKIE
	if (loginok && !isguest && (rframe[0] == 0) && !t[0] && !b[0]) {
		html_header(1);
		printf("<script>window.location.href="
				"'/" SMAGIC "/bbsindex?t=1&b=boa?secstr=';</script>");
		printf("</head><body>如果看到这条信息，"
				"说明您没有开启javascript，请开启javascript后"
				"重新访问本站。");
		return 0;
	}
#endif
#if 0
	if ((!loginok || isguest) && (rframe[0] == 0) && !t[0] && !b[0]) {
		checklanguage();
#if 0
		if (strcasecmp(FIRST_PAGE, getsenv("SCRIPT_URL"))) {
			html_header(3);
			redirect(FIRST_PAGE);
			http_quit();
		}
#endif

		wwwcache->home_visit++;
		loginwindow();
		http_quit();
	}
#endif	
	if (!loginok && !t[0] && !b[0]) {
		sprintf(redbuf, "/" SMAGIC "/bbslogin?id=guest&ipmask=8&t=%d",
			(int) now_t);
		html_header(3);
		//printf("<!-- %s -- %s -->", getsenv("SCRIPT_URL"), rframe);
		redirect(redbuf);
		http_quit();
	}

	if (cache_header(1000000000, 86400)) 
		return 0;
	html_header(1);
	if (!isguest
	    &&
	    (readuservalue(currentuser->userid, "wwwstyle", str, sizeof (str))
	     || atoi(str) != wwwstylenum)) {
		sprintf(str, "%d", wwwstylenum);
		saveuservalue(currentuser->userid, "wwwstyle", str);
	}
	printf("<meta http-equiv=\"refresh\" content=\"0; url=/index.html\">");
/*	printf("<title>%s %s</title>\n"
	       "<frameset id=fs0 frameborder=0 "
	       "frameSpacing=0 border=0 cols=\"140,11,*\">\n"
	       "<frame name=f2 src=bbsleft?t=%ld MARGINWIDTH=1 "
	       "MARGINHEIGHT=1>\n"
	       "<frame name=toogle scrolling=no noresize=true "
	       "marginwidth=0 marginheight=0 src=\"/wtoogle.htm\">"
	       "<frameset id=fs1 rows=\"18, *, 15\" "
	       "frameSpacing=0 frameborder=no border=0>\n"
	       "<frame scrolling=no name=fmsg src=\""
	       "bbsgetmsg\">\n"
	       "<frame name=f3 src=%s>"
	       "<frame scrolling=no marginwidth=4 marginheight=1 name=f4 src=\""
	       "bbsfoot\">\n"
	       "</frameset>\n"
	       "</frameset>\n", 
	       MY_BBS_NAME, BBS_WWW_INTRO, now_t, bbsred(rframe));*/
	//printf("<meta http-equiv=\"refresh\" content=\"0; url=%s\">",now_t);
	http_quit();
	return 0;
}
