#include "bbslib.h"

int
bbsemailreg_main()
{
#ifndef ENABLE_EMAILREG
	//if ENABLE_EMAILREG is not defined, bbsemailreg will just be an
	//alias of bbsreg. See bbsmain.c. The following line is put here
	//just for reference. It will never be excuted.
	return bbsreg_main();
#else
	html_header(1);
	printf("<script language=\"JavaScript\"><!--\n"
	       "function show(userid) {"
	       "document.images['myImage'].src = \"/cgi-bin/regimage?userid=\"+userid;\n"
	       "document.regform.g.value=\"看不到数字请点我\";"
	       "document.regform.g.disabled=false;}\n"
          "//--></script>\n");

	printf("<body>");
	printf("<center>%s -- 新用户注册<hr>\n", BBSNAME);
#if 0
	printf("欢迎加入本站，以下资料请如实填写。带 * 号的项目为必填内容。<br>"
	       "提交注册单之后，系统将会给您的 email 地址发送一封确认信。回复<br>"
	       "该确认信之后，您就可以在本站发表文章、参与讨论了。<br>");
#endif
	printf
	    ("<i>为免予纠纷起见，请不要注册不文明、不健康之代号、引起歧义的帐号；<br>"
	     "如无特别需要，也请避免使用知名人士的姓名。<br><br></i>");
	//printf("<b><font color=red>如果无法提供 email，请"
	//       "<a href=bbsreg>填写注册单</a>以手工注册</font></b><br><br>");
	printf("<form name=regform method=post action=bbsdoreg?useemail=off>\n");
	printf("<table>\n");
	printf("<tr><td align=right>*请输入代号:<td align=left>"
	       "<input name=userid size=%d maxlength=%d "
	       "onBlur=show(document.regform.userid.value);return false> "
	       "(2-12字符, 必须全为英文字母或标准汉字)\n", IDLEN, IDLEN);
	printf("<tr><td align=right>*请输入密码:<td align=left>"
	       "<input type=password name=pass1 size=%d maxlength=%d> (有效长度31位)\n",
	       PASSLEN - 1, PASSLEN - 1);
	printf("<tr><td align=right>*请确认密码:<td align=left>"
	       "<input type=password name=pass2 size=%d maxlength=%d> (有效长度31位)\n",
	       PASSLEN - 1, PASSLEN - 1);
	printf("<tr><td align=right>*请输入昵称:<td align=left>"
	       "<input name=username size=20 maxlength=%d> (2-30字符, 中英文不限)\n",
	       NAMELEN - 1);
	printf("<tr><td align=right>*请输入您的真实姓名:</td><td align=left>"
	       "<input name=realname size=20> (请用中文, 至少2个汉字)</td></tr>\n");
	printf("<tr><td align=right>*请输入您所在的城市:</td><td align=left>"
	       "<input name=address size=20> (请用中文, 至少2个汉字)</td></tr>\n");
	printf
	    ("<tr><td align=right>请输入您的联系电话(可选):<td align=left><input name=phone size=40>\n");
	printf("<tr><td align=right>*Email地址(被用于找回密码，别输错啊):</td><td align=left>"
	       "<input name=email size=40> </td></tr>\n");
	printf("<tr><td align=right>*请确认Email地址:</td><td align=left>"
	       "<input name=email2 size=40></td></tr>\n");
	printf("<tr><td align=right>跟大家打个招呼(可选):</td><td align=left>");
	printf
	    ("<textarea name=words rows=3 cols=40 wrap=virutal></textarea></td></tr>");
	printf("<tr><td align=right>"
	       "<input type=\"button\" name=g value=\"请先填写代号\" disabled=true "
	       "onClick=show(document.regform.userid.value);return false><br>");
	printf
	    ("<td align=left><img src=\"/cgi-bin/regimage\" name=\"myImage\">");
	printf("<tr><td align=right>*请输入上面的数字:<td align=left>"
	       "<input name=regpass size=10 maxlength=10>\n");
	printf("</table><br><hr>\n");
	printf
	    ("<input type=submit value=提交表格>"
	     " <input type=reset value=重新填写>"
	     " <input type=button value=查看帮助 onclick=\"javascript:{open('/reghelp.html','winreghelp','width=600,height=460,resizeable=yes,scrollbars=yes');return false;}\"\n");
	printf("</form>"
  "<tr><td align=right><input type=button value=检测帐号 onclick=\"javascript:{open('regcheck?userid='+document.regform.userid.value,'regcheck','width=600,height=200,resizeable=yes,scrollbars=yes');return false;}\"\n</tr>"
 // "<tr><td align=right><input type=button value=检测帐号 onclick=\"checkval();return false;}\"\n</tr>"
  "</center>\n");
	
/*	printf("<form name=prev action=regcheck method=post target=preview_page>"
          "<input type=hidden name=yhm value=\"\"><input type=hidden name=dzyj value=\"\">"
          "<input type=hidden name=dzyj2 value=\"\"></form>\n");
  printf("<script language=\"JavaScript\"><!--\n"
        "fucntion checkval() {"
         "document.prev.yhm.value=document.regform.userid;"
         "var popupWin = window.open('', 'preview_page', 'scrollbars=yes,width=500,height=300');document.prev.submit();}"
         "//--></script>\n");*/
 	http_quit();
	return 0;
#endif
}
