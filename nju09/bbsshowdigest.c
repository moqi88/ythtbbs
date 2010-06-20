#include "bbslib.h"
#define DAODUMORE MY_BBS_HOME "/wwwtmp/daodumore.htm"
#define JINGHUAMORE MY_BBS_HOME "/wwwtmp/jinghuamore.htm"
#define DIGESTJS MY_BBS_HOME "/wwwtmp/digest.js"

int
bbsshowdigest_main(){
	char *colum, columchar, columrightchar;

	colum = getparm("C");
	if (!colum[0])
		columchar = '#';
	else
		columchar = colum[0];

	if(columchar=='0')
		columrightchar='1';
	else
		columrightchar='0';

	html_header(1);
	printf("<script src=\"" BBSBOAJS "\"></script>\n");
	printf("<title>一路BBS导读精彩话题列表</title>\n");
/*	if (!showfile(DIGESTJS))
		http_fatal("未能找到文摘数据文件，请联系系统维护。");
*/
//	printf("</head>\n<body>\n");
//修改框架，增加左侧和底部
	printf("</head>\n");
	printf("<body topmargin=0 leftMargin=1 MARGINWIDTH=1 MARGINHEIGHT=0>" WWWLEFT_DIV);
/*
	printf("\n<script language=\"javascript\">\n"
			"\tDL.showdefault('%c', '" MY_BBS_NAME "',"
			" '" SMAGIC "');\n"
			"</script>\n", columchar);*/
	printf("<div id=\"digest\">\n<div id=\"digest_left\">\n<div id=\"digest_%c\" class=\"digest_box\">\n", columchar);
	printf("<h2 id=\"digest_%c_title\">", columchar);
	if(columchar=='0')
		printf("近日精彩话题");
	else
		printf("本站精华文摘");
	if(columchar=='1')
		printf("<a target=\"_blank\" href=\"/HT/bbsrss?rssid=130&sec=0\"><img border=\"0\" src=\"/rss.gif\"/></a>");
	printf("</h2><div id=\"digest_%c_content\">\n", columchar);
	if(columchar=='0')
		showfile(DAODUMORE);
	else
		showfile(JINGHUAMORE);
	printf("</div></div>\n</div>\n");
	printf("<div id=\"digest_right\"><div id=\"digest_%c\" class=\"digest_box\">\n", columrightchar);
	printf("<h2 id=\"digest_%c_title\">", columrightchar);
	if(columrightchar=='0')
		printf("近日精彩话题");
	else
		printf("本站精华文摘");
	if(columrightchar=='1')
		printf("<a target=\"_blank\" href=\"/HT/bbsrss?rssid=digest&sec=0\"><img border=\"0\" src=\"/rss.gif\"/></a>");
	printf("</h2><div id=\"digest_%c_content\">\n", columrightchar);
	if(columrightchar=='0')
		showfile(DAODUMORE);
	else
		showfile(JINGHUAMORE);
	printf("</div></div></div>\n");
	showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
//	printf("</body></html>");
	printf(WWWFOOT_DIV "</body></html>\n");

	return 1;
}

void
digestadjust(char *idname) {
//	printf("<script>DL.autofit('%s', 0, 0);</script>\n", idname);
//  changed by dl  暂时去掉，回头补上相应C代码
	return;
}

int
digestinit(int *init_digest) {
	if (!*init_digest)
		return 1;
	showfile(DIGESTJS);
	*init_digest = 0;
	return 1;
}

int
showsecdigest(const struct sectree *sec, char *s, int *init_digest) {
	char colum;

	colum = *s;
	
	if (!digestinit(init_digest))
		return 0;
	printf("<script>DL.show('%c', 1, 1, 1, 0, '" SMAGIC 
			"');</script>\n", colum);
	return 0;
}


