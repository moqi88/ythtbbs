#include "bbslib.h"

int
bbsinfo_main()
{
	int type;
	struct userdata currentdata;

	html_header(1);
	//check_msg();
	printf("<body>");
	if (!loginok || isguest)
		http_fatal("您尚未登录");
	changemode(EDITUFILE);
	type = atoi(getparm("type"));
	printf("%s -- 用户个人资料<hr>\n", BBSNAME);
	if (type != 0) {
		check_info();
		http_quit();
	}
	loaduserdata(currentuser->userid, &currentdata);
	if (currentuser->mypic) {
		printf("<table align=right><tr><td><center>");
		printmypic_nocache(currentuser->userid);
		printf("<br>当前头像</center></td></tr></table>");
	}
	printf("<form action=bbsinfo?type=1 method=post enctype='multipart/form-data'>");
	printf("您的帐号: %s&nbsp;&nbsp;数字登录标识: %d<br>\n", currentuser->userid, getuser(currentuser->userid, NULL));
	//getuser(userid, &currentuser));
	printf
	    ("您的昵称: <input type=text name=nick value='%s' size=24 maxlength=%d>%d个汉字或%d个英文字母以内<br>",
	     void1(nohtml(currentuser->username)), NAMELEN, NAMELEN / 2, NAMELEN);
	printf("发表大作: %d 篇<br>\n", currentuser->numposts);
//      printf("信件数量: %d 封<br>\n", currentuser.nummails);
	printf("上站次数: %d 次<br>\n", currentuser->numlogins);
	printf("上站时间: %ld 分钟<br>\n", currentuser->stay / 60);
	printf
	    ("真实姓名: %s<br>\n", void1(nohtml(currentdata.realname)));
	printf
	    ("所在城市: <input type=text name=address value='%s' size=40 maxlength=%d>%d个汉字或%d个英文字母以内<br>\n",
	     void1(nohtml(currentdata.address)), STRLEN, STRLEN / 2, STRLEN);
	printf("帐号建立: %s<br>", Ctime(currentuser->firstlogin));
	printf("最近光临: %s<br>", Ctime(currentuser->lastlogin));
	printf("来源地址: %s<br>", inet_ntoa(from_addr));
	printf
	    ("电子邮件: <input type=text name=email value='%s' size=32 maxlength=%d>%d个汉字或%d个英文字母以内<br>\n",
	     void1(nohtml(currentdata.email)), STRLEN, STRLEN / 2, STRLEN);
	printf("上载头像: <input type=file name=mypic>4M 字节以内<br>");
#if 0
	printf
	    ("出生日期: <input type=text name=year value=%d size=4 maxlength=4>年",
	     currentuser.birthyear + 1900);
	printf("<input type=text name=month value=%d size=2 maxlength=2>月",
	       currentuser.birthmonth);
	printf("<input type=text name=day value=%d size=2 maxlength=2>日<br>\n",
	       currentuser.birthday);
	printf("用户性别: ");
	printf("男<input type=radio value=M name=gender %s>",
	       currentuser.gender == 'M' ? "checked" : "");
	printf("女<input type=radio value=F name=gender %s><br>",
	       currentuser.gender == 'F' ? "checked" : "");
#endif
	printf
	    ("<input type=submit value=确定> <input type=reset value=复原>\n");
	printf("</form>");
	printf("<hr>");
	showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
	printf("</body></html>");
	return 0;
}

int
check_info()
{
	int m, all_ok=1;
	char buf[256];
	struct userdata currentdata;
	struct userec tmp;
	struct parm_file *parmFile;

	loaduserdata(currentuser->userid, &currentdata);
	memcpy(&tmp, currentuser, sizeof (tmp));
	strsncpy(buf, getparm("nick"), 30);
	for (m = 0; m < strlen(buf); m++)
		if ((buf[m] < 32 && buf[m] > 0) || buf[m] == -1)
			buf[m] = ' ';
	if (strlen(buf) > 1) {
		strcpy(tmp.username, buf);
	} else {
		printf("警告: 昵称太短!<br>\n");
		all_ok=0;
	}
#if 0
	strsncpy(buf, getparm("realname"), 9);
	if (strlen(buf) > 1) {
		strcpy(currentdata.realname, buf);
	} else {
		printf("警告: 真实姓名太短!<br>\n");
		all_ok=0;
	}
#endif
	strsncpy(buf, getparm("address"), 40);
	if (strlen(buf) >= 4) {
		strcpy(currentdata.address, buf);
	} else {
		printf("警告: 所在城市太短!<br>\n");
		all_ok=0;
	}
	strsncpy(buf, getparm("email"), 32);
	if (strlen(buf) > 8 && strchr(buf, '@')) {
		strcpy(currentdata.email, buf);
	} else {
		printf("警告: email地址不合法!<br>\n");
		all_ok=0;
	}
	if ((parmFile = getparmfile("mypic"))) {
                if (parmFile->len > 4194304) {
                        printf("警告: 图片尺寸最大 4M 字节，请先压缩图片。");
			all_ok=0;
                } else if (parmFile->len > 0) {
                        char picfile[256], *out;
                        int fd, osize, ret;
			ret = szm_head_photo(tjpg_ctx, parmFile->content,
				     parmFile->len, &out, &osize);
			if(ret<0&&parmFile->len > 10240){
				printf("警告:头像文件格式错误: %d", ret);
				all_ok=0;
			} else {
                        	sethomefile(picfile, currentuser->userid, "mypic");
                        	fd = open(picfile, O_CREAT | O_WRONLY, 0660);
                        	if (fd > 0) {
					if(ret<0)
                                		write(fd, parmFile->content, parmFile->len);
					else{
						write(fd, out, osize);
						free(out);
					}
                                	close(fd);
                        	}
                        	tmp.mypic = 1;
			}
                } else
			printf("头像没有修改");
        } 
	updateuserec(&tmp, 0);
	saveuserdata(currentuser->userid, &currentdata);
	if(all_ok){
		printf("<br>个人资料修改成功, 1秒内将自动显示您的最新资料!");
		printf("<script>setTimeout('self.location.replace("
	       "\"bbsinfo\")',1000);</script>");
	}
	else
		printf("<br><font color=red>个人资料修改存在警告或错误,请重新检查.</font>");
	return 0;
}
