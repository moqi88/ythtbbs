#include "bbslib.h"

int
bbspassport_main()
{
	char id[IDLEN + 1], buf[384], site[256];
	struct userec *x;

	html_header(1);
	if (key_fail)
		http_fatal("内部错误 联系维护!");
	strsncpy(buf, getparm("pp"), sizeof (buf));
	strsncpy(site, getparm("site"), sizeof (site));
	strsncpy(id, des3_decode(buf, 0), IDLEN + 1);
	if (id[0] && getuser(id, &x) > 0) {
		snprintf(buf, sizeof (buf), "http://%s?q=%s", site,
			 des3_encode(id, 1));
		redirect(buf);
		http_quit();
		return 0;
	}
	printf("<form name=lpassport method=post action=lpassport>\n");
	printf("<table width=100%%>\n");
	printf
	    ("<tr><td align=right>*请输入代号:<td align=left><input name=id size=12 maxlength=%d >\n",
	     IDLEN);
	printf
	    ("<tr><td align=right>*请输入密码:<td align=left><input type=password name=pw size=12 maxlength=%d>\n",
	     PASSLEN - 1);
	printf("<tr><td><td><input type=hidden name=site value=\"%s\">", site);
	printf("<tr><td align=right><input type=submit value=登录>"
	       " <td align=left><input type=reset value=重新填写></table>");
	printf("</form>");
	http_quit();
	return 0;
}
