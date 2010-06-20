#include "bbslib.h"

char *defines[] = {
	"呼叫器关闭时可让好友呼叫",	/* DEF_FRIENDCALL */
	"接受所有人的短消息",	/* DEF_ALLMSG */
	"接受好友的短消息",	/* DEF_FRIENDMSG */
	"收到短消息发出声音",	/* DEF_SOUNDMSG */
	"使用彩色",		/* DEF_COLOR */
	"显示活动看版",		/* DEF_ACBOARD */
	"显示选单的讯息栏",	/* DEF_ENDLINE */
	"编辑时显示状态栏",	/* DEF_EDITMSG */
	"讯息栏采用一般/精简模式",	/* DEF_NOTMSGFRIEND */
	"选单采用一般/精简模式",	/* DEF_NORMALSCR */
	"分类讨论区以 New 显示",	/* DEF_NEWPOST */
	"阅读文章是否使用绕卷选择",	/* DEF_CIRCLE */
	"阅读文章游标停于第一篇未读",	/* DEF_FIRSTNEW */
	"进站时显示好友名单",	/* DEF_LOGFRIEND */
	"进站时显示备忘录",	/* DEF_INNOTE */
	"离站时显示备忘录",	/* DEF_OUTNOTE */
	"离站时询问寄回所有短消息",	/* DEF_MAILMSG */
	"使用自己的离站画面",	/* DEF_LOGOUT */
	"开启/关闭信箱",	/* DEF_SEEWELC1 */
	"好友上站通知",		/* DEF_LOGINFROM */
	"观看留言板",		/* DEF_NOTEPAD */
	"不要送出上站通知给好友",	/* DEF_NOLOGINSEND */
	"主题式看版",		/* DEF_THESIS */
	"收到短消息等候回应或清除",	/* DEF_MSGGETKEY */
	"汉字整字删除",		/* DEF_DELDBLCHAR */
	"使用GB码阅读",		/* DEF_USEGB KCN 99.09.03 */
	"使用动态底线",		/* DEF_ANIENDLINE */
	"初次访问版面提示进入精华区",	/* DEF_INTOANN */
	"发表文章时暂时屏蔽MSG",	/* DEF_POSTNOMSG */
	"进站时观看统计信息",	/* DEF_SEESTATINLOG */
	"过滤可能令人反感信息",	/* DEF_FILTERXXX */
	"收取站外信件",		/* DEF_INTERNETMAIL */
	NULL
};

char *exp_group[] =  {
	"[A]如故",
	"[B]赌系",
	"[C]修为",
	"[D]武功",
	"[E]菜系",
	"[F]歌系",
	"[G]兵器",
	"[H]水系",
	"[I]经验",
	"[J]生物",
	"[K]西游",
	"[L]爱情",
	"[M]光光",
	"[N]军衔",
	"[O]轮回",
	"[P]佛教",
	"[Q]学历",
	"[R]经典",
	"[S]酒令",
	"[T]女生",
	"[U]隐藏",
	"[V]麻将",
	"[W]恐龙",
	"[X]天使",
	NULL
};

char *exp_long[] = {
	"[1]古代官职(长系)",
	NULL
};

int
bbsparm_main()
{
	int i, perm = 1, type;
	struct userec *x;
	html_header(1);
	//check_msg();
	type = atoi(getparm("type"));
	printf("<body><center>%s -- 修改个人参数 [使用者: %s]<hr>\n", BBSNAME,
	       currentuser->userid);
	if (!loginok || isguest)
		http_fatal("匆匆过客不能设定参数");
	if (getuser(currentuser->userid, &x) <= 0) {
		printf("不可能，肯定是你敲错了，根本没这人啊");
		printf("<p><a href=javascript:history.go(-1)>快速返回</a>");
		http_quit();
	}

	changemode(USERDEF);
	if (type)
		return read_form();
	printf("<form action=bbsparm?type=1 method=post>\n");
	printf("<table>\n");
	for (i = 0; defines[i]; i++) {
		char *ptr = "";
		if (i % 2 == 0)
			printf("<tr>\n");
		if (currentuser->userdefine & perm)
			ptr = " checked";
		printf
		    ("<td><input type=checkbox name=perm%d%s></td><td>%s</td>",
		     i, ptr, defines[i]);
		perm = perm * 2;
	}
	printf("<tr><td>修改经验值体系</td></tr>");
	for (i = 0; exp_group[i]; i++) {
		char *ptr = "";
		if (i % 2 == 0)
			printf("<tr>\n");
		if (x->exp_group == (65 + i))
			ptr = " checked";
		printf("<td><input type=radio name=expgroup value=%d %s></td><td>%s</td>\n",
				65 + i, ptr, exp_group[i]);
	}
	printf("<tr>\n");
	printf("<td><input type=radio name=expgroup value=%d %s></td><td>%s</td>\n",
			'1', x->exp_group == '1' ?" checked":"", exp_long[0]);
	printf("</table>");
	printf
	    ("<input type=submit value=确定修改></form><br>以上参数大多仅在telnet方式下才有作用\n");
	showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
	printf("</body>");
	http_quit();
	return 0;
}

int
read_form()
{
	int i, perm = 1, def = 0;
	char var[100];
	struct userec tmp;
	for (i = 0; i < 32; i++) {
		sprintf(var, "perm%d", i);
		if (strlen(getparm(var)) == 2)
			def += perm;
		perm = perm * 2;
	}
	memcpy(&tmp, currentuser, sizeof (tmp));
	tmp.userdefine = def;
	tmp.exp_group = atoi(getparm("expgroup"));
	updateuserec(&tmp, 0);
	printf("个人参数设置成功.<br><a href=bbsparm>返回个人参数设置选单</a>");
	return 0;
}
