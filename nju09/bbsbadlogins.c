#include "bbslib.h"

int
bbsbadlogins_main()
{
	char file[STRLEN];
	html_header(1);
	printf("<body>");
	if (!loginok || isguest) {
		printf("哦，你并没有登陆，不用检查密码记录了");
		showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
		printf("</body></html>");
		return 0;
	}

	sethomefile(file, currentuser->userid, BADLOGINFILE);
	if (!file_exist(file)) {
		printf("没有任何密码输入错误记录");
		showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
		printf("</body></html>");
		return 0;
	}
	if (*getparm("del") == '1') {
		unlink(file);
		printf("密码输入错误记录已被删除<br>");
		printf
		    ("<a href='#' onClick='javascript:window.close()'>关闭窗口</a>");
	} else {
		printf("发现以下密码输入错误记录<br><pre>");
		showfile(file);
		printf("</pre>");
		printf("<a href=bbsbadlogins?del=1>删除密码输入错误记录</a>");
	}
	printf("<br><br><b>为保证帐户的安全，可以用 telnet 方式登陆" MY_BBS_NAME
	       "，并在主菜单--&gt;“工具箱”--&gt;“修编个人档案”功能中设定禁止登陆的 IP。</b>");
	showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
	printf("</body></html>");
	return 0;
}
