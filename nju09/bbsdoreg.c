#include "bbslib.h"
#define AUTO_REG 1
#define	MAX_INVINTE 50
#define isletter(a)	(a>= 'A' && a <= 'Z')?1:((a>= 'a' && a <= 'z')?1:0)

#ifdef USESESSIONCOOKIE
extern char sessionCookie[];
#endif

int
verifyInvite(char *email)
{
	char inviter[30], code[30], t[30];
	char invitefn[256];
	char ivemail[80];
	strsncpy(inviter, getparm("inviter"), sizeof (inviter));
	strsncpy(code, getparm("code"), sizeof (code));
	strsncpy(t, getparm("t"), sizeof (t));
	sprintf(invitefn, INVITATIONDIR "/%s/%s.%s", inviter, t, code);
	close(open(invitefn, O_RDWR));	//touch new
	if (!file_exist(invitefn) ||
	    readstrvalue(invitefn, "toemail", ivemail, sizeof (ivemail)) < 0)
		return 0;
	if (strcasecmp(email, ivemail))
		return 0;
	return 1;
}

int
postInvite(char *userid, char *inviter)
{
	struct userec *x, tmpu;
	char buf[256];
	FILE *fp;
	if (getuser(inviter, &x) <= 0)
		return -1;
	loadfriend(userid);
	if (friendnum < 199) {
		strsncpy(fff[friendnum].id, x->userid, sizeof (fff[0].id));
		strsncpy(fff[friendnum].exp, inviter, sizeof (fff[0].exp));
		friendnum++;
		sethomefile(buf, userid, "friends");
		fp = fopen(buf, "w");
		fwrite(fff, sizeof (struct override), friendnum, fp);
		fclose(fp);
	}

	loadfriend(x->userid);
	if (friendnum < 199) {
		strsncpy(fff[friendnum].id, userid, sizeof (fff[0].id));
		strsncpy(fff[friendnum].exp, getparm("name"),
			 sizeof (fff[0].exp));
		friendnum++;
		sethomefile(buf, x->userid, "friends");
		fp = fopen(buf, "w");
		fwrite(fff, sizeof (struct override), friendnum, fp);
		fclose(fp);
	}
	tmpu = *x;
	if (tmpu.extraexp1 >= MAX_INVINTE ) {
		tmpu.extraexp1 = MAX_INVINTE;
		snprintf(buf, sizeof (buf),
			 "您邀请的 %s 已经注册了，用户名为 %s。"
			 "您邀请好友获得的附加经验值达到上限！", getparm("name"), userid);
	} else {
		tmpu.extraexp1 ++;
		snprintf(buf, sizeof (buf),
			 "您邀请的 %s 已经注册了，用户名为 %s，"
			 "您的经验值上涨 20 点！", getparm("name"), userid);
	}
	updateuserec(&tmpu, 0);
	system_mail_buf(buf, strlen(buf), x->userid, "您的朋友注册了", userid);

	return 0;
}

int
checkRegPass(char *iregpass, const char *userid)
{
	struct MD5Context mdc;
	unsigned int regmd5[4];
	char regpass[5];
	//记录新 ID 的信息...
	MD5Init(&mdc);
	MD5Update(&mdc, (void *) (&bbsinfo.ucachehashshm->regkey),
		  sizeof (int) * 4);
	MD5Update(&mdc, userid, strlen(userid));
	MD5Final((char *) regmd5, &mdc);
	sprintf(regpass, "%d%d%d%d", regmd5[0] % 10, regmd5[1] % 10,
		regmd5[2] % 10, regmd5[3] % 10);
	if (!strcmp(regpass, iregpass))
		return 0;
//              errlog("%s regpass:%s iregpass:%s", x.userid, regpass, iregpass);
	MD5Init(&mdc);
	MD5Update(&mdc, (void *) (&bbsinfo.ucachehashshm->oldregkey),
		  sizeof (int) * 4);
	MD5Update(&mdc, userid, strlen(userid));
	MD5Final((char *) regmd5, &mdc);
	sprintf(regpass, "%d%d%d%d", regmd5[0] % 10, regmd5[1] % 10,
		regmd5[2] % 10, regmd5[3] % 10);
	if (strcmp(regpass, iregpass))
//                     errlog("%s regpass:%s iregpass:%s", x.userid, regpass, iregpass);
		//验证码错误
		return -1;
	if (now_t - bbsinfo.ucachehashshm->keytime > 300) {
//              errlog("too long time %d",
//                     (int)(now_t - bbsinfo.ucachehashshm->keytime));
		//http_fatal("验证码过期，发呆太久了吧");
		return -2;
	}
	return 0;
}

int
bbsdoreg_main()
{
	FILE *fp;
	struct userec x;
	struct userdata xdata;
	char filename[80], pass1[80], pass2[80], dept[80], phone[80], assoc[80],
	    words[1024], buf[80], buf2[80], *ub = FIRST_PAGE, *ptr;
	char iregpass[5];
	char md5pass[MD5LEN];
	//int lockfd;
	//struct MD5Context mdc;
	//unsigned int regmd5[4];
	//char regpass[5];
	int r = 0, emailcheck = 0, invited = 0;
  char url[10];
	html_header(1);
	printf("<body>");
	bzero(&x, sizeof (x));
	bzero(&xdata, sizeof (xdata));
	strsncpy(x.userid, getparm("userid"), IDLEN + 1);
	strsncpy(pass1, getparm("pass1"), PASSLEN);
	strsncpy(pass2, getparm("pass2"), PASSLEN);
	strsncpy(x.username, getparm("username"), sizeof (x.username));
	strsncpy(xdata.realname, getparm("realname"), sizeof (xdata.realname));
	strsncpy(dept, getparm("dept"), sizeof (dept));
	strsncpy(xdata.address, getparm("address"), sizeof (xdata.address));
	strsncpy(xdata.email, strtrim(getparm("email")), sizeof (xdata.email));
	strsncpy(phone, getparm("phone"), sizeof (phone));
	strsncpy(assoc, getparm("assoc"), sizeof (assoc));
	strsncpy(words, getparm("words"), sizeof (words));
	strsncpy(iregpass, getparm("regpass"), 5);
#ifdef ENABLE_INVITATION
	if (!strcasecmp(getparm("useemail"), "on"))
		emailcheck = 1;
	else if (atoi(getparm("invited")) == 1)
		invited = 1;
#endif

	if (!goodgbid(x.userid))
		http_fatal("帐号只能由英文字母和标准汉字组成");
	if (strlen(x.userid) < 2 || (strlen(x.userid) == 2 && !(isletter(x.userid[0]) && isletter(x.userid[1])) ) )
		http_fatal("帐号长度太短(2-12个中英文字符)");
	if (strlen(pass1) < 4)
		http_fatal("密码太短(至少4字符)");
	if (strcmp(pass1, pass2))
		http_fatal("两次输入的密码不一致, 请确认密码");
	if (strlen(x.username) < 2)
		http_fatal("请输入昵称(昵称长度至少2个字符)");
	if (strlen(xdata.realname) < 4)
		http_fatal("请输入姓名(请用中文, 至少2个字)");
	if (strlen(xdata.address) < 4 && !invited)
		http_fatal("请输入您所在的城市(请用中文, 至少2个字)");
	if (strcasecmp(xdata.email, strtrim(getparm("email2")))) {
		printf("两次填写的 email 地址不一致。");
		http_fatal("请重新填写 email 地址。");
	}		
	if (!trustEmail(xdata.email)) {
		printf
		    ("所提供的 email 不在系统认定范围，或者被别的注册 ID 使用过。");
		http_fatal("请重新选择 email。\n");
	}
#if 0
	if (!emailcheck && !invited) {
		if (strlen(dept) < 14)
			http_fatal
			    ("学校系级或工作单位的名称长度至少要14个字符(或7个汉字)");
		if (strlen(xdata.address) < 16)
			http_fatal("通讯地址长度至少要16个字符(或8个汉字)");
	}
#endif
	if (badstr(x.passwd) || badstr(x.username)
	    || badstr(xdata.realname))
		http_fatal("您的注册单中含有非法字符 (密码、昵称、真实姓名)");
	if (badstr(xdata.address) || badstr(xdata.email))
		http_fatal("您的注册单中含有非法字符 (通讯地址、email)");
	if (is_bad_id(x.userid))
		http_fatal("不雅帐号或禁止注册的id, 请重新选择");
		
	if ((emailcheck || invited) && !trustEmail(xdata.email)) {
		//... is it in the range?
		printf
		    ("所提供的 email 不在系统认定范围，或者被别的注册 ID 使用过。");
		http_fatal("请重新选择 email，或者使用人工注册。\n");
	}
	 if (invited && !verifyInvite(xdata.email)) {
	        printf
	         ("没有找到相应的邀请纪录，请使用<a href=bbsemailreg>普通注册方式</a>");
	        return 0;
	 }

	//记录新 ID 的信息...
	switch (checkRegPass(iregpass, x.userid)) {
	case -1:
		http_fatal("验证码错误");
	case -2:
		http_fatal("验证码过期，发呆太久了吧");
	default:
		break;
	}

	x.salt = getsalt_md5();
	genpasswd(md5pass, x.salt, pass1);
	memcpy(x.passwd, md5pass, MD5LEN);
	x.lasthost = from_addr.s_addr;
#ifndef AUTO_REG
	x.userlevel = PERM_BASIC;
#else
	x.userlevel = PERM_DEFAULT;
#endif
	x.firstlogin = now_t;
	x.lastlogin = now_t;
	x.numlogins = 1;
	x.userdefine = 0xffffffff;
	x.flags[0] = CURSOR_FLAG | PAGER_FLAG;
	switch (insertuserec(&x)) {
	case -1:
		http_fatal("无法注册，可能原因：注册用户已达上限");
		break;
	case 0:
		r = user_registered(x.userid);
		if (r > 0)
			http_fatal("此帐号已经有人使用,请重新选择。");
		else if (r < 0)
			http_fatal
			    ("此帐号刚刚死亡待清理，请明天再注册，或重新选择id");
		else {
			errlog("www reg error: %s", x.userid);
			http_fatal("内部错误");
		}
		break;
	}
	sethomefile(filename, x.userid, "");
	mkdir(filename, 0755);
	saveuserdata(x.userid, &xdata);
	if (invited) {
		FILE *fp;
		char *ptr, *iv = getparm("inviter");
		if ((ptr = strchr(iv, '\n')))
			*ptr = 0;
		sethomefile(filename, x.userid, "mailcheck");
		fp = fopen(filename, "w");
		if (!fp) {
			errlog("can't open %s", filename);
			http_fatal("内部错误");
		}
		fprintf(fp, "firstlogin %ld\n", now_t);
		fprintf(fp, "realname %s\n", xdata.realname);
		fprintf(fp, "inviter %s\n", iv);
		fprintf(fp, "lasthost %s\n", realfromhost);
		fprintf(fp, "email %s\n", xdata.email);
		fclose(fp);
		doConfirm(&x, xdata.email, 1);
		postInvite(x.userid, iv);
	}

	if (emailcheck) {
		if (send_emailcheck(&x, &xdata) < 0)
			emailcheck = 0;
	}
	if ( !emailcheck && !invited){
#ifndef AUTO_REG
		//一般手工审批
		lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX);
		fp = fopen("new_register", "a");
		if (fp) {
			fprintf(fp, "usernum: %d, %s\n", getuser(x.userid, NULL),
			Ctime(now_t));
			fprintf(fp, "userid: %s\n", x.userid);
			fprintf(fp, "realname: %s\n", xdata.realname);
			fprintf(fp, "dept: %s\n", dept);
			fprintf(fp, "addr: %s\n", xdata.address);
			fprintf(fp, "phone: %s\n", phone);
			fprintf(fp, "assoc: %s\n", assoc);
			fprintf(fp, "rereg: 0\n");
			fprintf(fp, "----\n");
			fclose(fp);
		}
		close(lockfd);
#else
		//一般自动审批
		sethomefile(buf, x.userid, "register");
		if (file_exist(buf)) {
			sethomefile(buf2, x.userid, "register.old");
			rename(buf, buf2);
		}
		if ((fp = fopen(buf, "w")) != NULL) {
			fprintf(fp, "usernum: %d, %s", getuser(x.userid, NULL),
				Ctime(now_t));
			fprintf(fp, "realname: %s\n", xdata.realname);
			fprintf(fp, "dept: %s\n", dept);
			fprintf(fp, "addr: %s\n", xdata.address);
			fprintf(fp, "phone: %s\n", phone);
			fprintf(fp, "assoc: %s\n", assoc);
			fprintf(fp, "rereg: 0\n");
			fprintf(fp, "----\n");
			fclose(fp);
		}
		mail_file("etc/s_fill", x.userid, "恭禧您通过身份验证", "SYSOP");
		mail_file("etc/s_fill2", x.userid,
			  "欢迎加入" MY_BBS_NAME "大家庭", "SYSOP");
#endif
	}
	printf("<center><table><td><td><pre>\n");
	printf("亲爱的新使用者，您好！\n\n");
	printf("欢迎光临 本站, 您的新帐号已经成功被登记了。\n");
	printf("本页含有您的机密和隐私信息，请尽快点击下面的\"现在进入" MY_BBS_ID "\"或关闭此页面。\n");
	printf("您目前拥有本站基本的权限, 包括阅读文章、环顾四方、接收私人\n");
	printf("信件、接收他人的消息、进入聊天室等等。\n");
	printf("并且您可以发表文章参与讨论。\n");
#if 0
	if (invited) {
		printf("并且您可以发表文章参与讨论。\n");
	} else {
		printf
		    ("当您通过本站的身份确认手续之后，您还会获得更多的权限。\n");
	}
	if (!emailcheck && !invited) {
		printf
		    ("目前您的注册单已经被提交等待审阅。一般情况 24 小时以内就会\n");
		printf("有答复，请耐心等待。同时请留意您的站内信箱。\n");
	} else if (emailcheck) {
		printf("Email 确认信已经发往 %s，请注意查收并回复。\n"
		       "如果收不到确认信，请填写左边菜单中的注册单。\n",
		       xdata.email);
	}
#endif
	printf("如果您有任何疑问，可以去 BBSHelp (BBS求助) 版发文求助。\n\n"
	       "</pre></table>");
	printf("<hr><br>您的基本资料如下:<br>\n");
	printf("<table border=1 width=400>");
	printf("<tr><td>帐号位置: <td>%d\n", getuser(x.userid, NULL));
	printf("<tr><td>使用者代号: <td>%s (%s)\n", x.userid, x.username);
	printf("<tr><td>姓  名: <td>%s<br>\n", xdata.realname);
	printf("<tr><td>昵  称: <td>%s<br>\n", x.username);
	printf("<tr><td>所在城市: <td>%s<br>\n", xdata.address);
	printf("<tr><td>上站位置: <td>%s<br>\n", inet_ntoa(from_addr));
	printf("<tr><td>电子邮件: <td>%s<br></table><br>\n", xdata.email);
	newcomer(&x, words);
	tracelog("%s newaccount %d %s www", x.userid, getuser(x.userid, NULL),
		 realfromhost);
	wwwstylenum = 6;
	ub = wwwlogin(&x, 0, 0);
	ptr = getsenv("HTTP_X_FORWARDED_FOR");
	tracelog("%s enter %s www %s", x.userid, realfromhost, ptr);
#ifdef USESESSIONCOOKIE
	printf("<script>document.cookie='SESSION=%s; path=/; </script>", urlencode(sessionCookie));
#endif
	printf
            ("<center><form><input type=button onclick="
             "'top.location.href=\"/%s/bbslogin?id=%s&pw=%s&regjump=1\";' value=\"现在进入" MY_BBS_ID
             "\" ></form></center>\n", SMAGIC, x.userid, pass1);
/*
	printf
	    ("<center><form name=jump action=/" SMAGIC "/bbslogin method=post><input type=hidden name=regjump value=1><input type=hidden name=t value=''><input type=hidden name=lastip1 value=''>""<input type=hidden name=lastip2 value=''><input type=hidden name=id value=%s><input type=hidden name=pw value=%s><input type=hidden name=ipmask value=5><input type=submit value=现在进入" MY_BBS_ID"></form></center><script language=javascript>jump.submit();</script>\n",x.userid,pass1); */
/*	printf
	    ("<center><form><input type=button onclick="
	    "'top.location.href=\"http://%s/%s?t=%d\";'value=现在进入" MY_BBS_ID
	    "><form></center>\n",MY_BBS_DOMAIN,ub,(int)now_t);*/
	return 0;
}

int
badstr(unsigned char *s)
{
	int i;
	for (i = 0; s[i]; i++)
		if (s[i] != 9 && (s[i] < 32 || s[i] == 255))
			return 1;
	return 0;
}

void
newcomer(struct userec *x, char *words)
{
	FILE *fp;
	char filename[80];
	sprintf(filename, "bbstmpfs/tmp/%d.tmp", thispid);
	fp = fopen(filename, "w");
	fprintf(fp, "大家好, \n\n");
	fprintf(fp, "我是 %s(%s), 来自 %s\n", x->userid, x->username, fromhost);
	fprintf(fp, "今天初来此地报到, 请大家多多指教.\n\n");
	fprintf(fp, "自我介绍:\n\n");
	fprintf(fp, "%s", words);
	fclose(fp);
	post_article("newcomers", "WWW新手上路", filename, x->userid,
		     x->username, fromhost, -1, 0, 0, x->userid, -1);
	unlink(filename);
}
