/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    Firebird Bulletin Board System
    Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
                        Peng Piaw Foong, ppfoong@csie.ncu.edu.tw
    Copyright (C) 1999, KCN,Zhou Lin, kcn@cic.tsinghua.edu.cn
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include "bbs.h"
#include "bbstelnet.h"
/*
#define  EMAIL          0x0001 
#define  NICK           0x0002 
#define  REALNAME       0x0004 
#define  ADDR           0x0008
#define  REALEMAIL      0x0010
#define  BADEMAIL       0x0020
#define  NEWREG         0x0040
*/

#define	ENABLE_EMAILREG	1	/* 邮件注册，改天把跟www的define弄到一起去*/
#define isletter(a)     (a>= 'A' && a <= 'Z')?1:((a>= 'a' && a <= 'z')?1:0)

char *sysconf_str();

extern time_t login_start_time;
extern int convcode;

static int valid_ident(char *ident);

static int
getnewuserid(struct userec *newuser)
{
	int i;
	if ((i = insertuserec(newuser)) < 0) {
		if (file_isfile("etc/user_full")) {
			ansimore("etc/user_full", NA);
		} else {
			prints
			    ("抱歉, 使用者帐号已经满了, 无法注册新的帐号.\n\r");
		}
		prints("请您明天再试, 祝你好运.\n\r");
		refresh();
		sleep(3);
		exit(1);
	}
	if (i == 0) {
		prints
		    ("抱歉，就在刚才这个 id 被注册了，请另想一个新名字吧.\n\n");
		refresh();
		sleep(3);
		exit(1);
	}
	return i;
}

void
new_register()
{
	struct userec newuser;
	char passbuf[STRLEN];
	char passbuf1[STRLEN];
	int allocid, try;
	char md5pass[16];
	int r = 0;
	char useridutf8[22];  //21 is the longest utf8 length

	if (0) {
		now_t = time(0);
		sprintf(genbuf, "etc/no_register_%3.3s", ctime(&now_t));
		if (file_isfile(genbuf)) {
			ansimore(genbuf, NA);
			pressreturn();
			exit(1);
		}
	}
	if (fileIP(MY_BBS_HOME "/etc/untrust", realfromhost)) {
		clear();
		move(4, 4);
		prints("对不起，您的IP暂时不能注册ID，"
				"请尝试换个IP再来过。\n");
		pressreturn();
		exit(1);
	}
	memset(&newuser, 0, sizeof (newuser));
	// getdata(0, 0, "使用GB编码阅读?(\xa8\xcf\xa5\xce BIG5\xbd\x58\xbe\x5c\xc5\xaa\xbd\xd0\xbf\xefN)(Y/N)? [Y]: ", passbuf, 4, DOECHO, YEA);
	// if (*passbuf == 'n' || *passbuf == 'N')
	//  if (!convcode)
	//          switch_code();

	clear();
	if (show_cake()) {
		prints("你别是个机器人吧...\n");
		refresh();
		longjmp(byebye, -1);
	}
	clear();
	ansimore("etc/register", NA);
	try = 0;
	while (1) {
		if (++try >= 9) {
			prints("\n掰掰，按太多下  <Enter> 了...\n");
			refresh();
			longjmp(byebye, -1);
		}
		getdata(t_lines - 5, 0,
			"请输入帐号名称 (Enter User ID, \"0\" to abort): ",
			newuser.userid, IDLEN + 1, DOECHO, YEA);
		if (newuser.userid[0] == '0' && newuser.userid[1] == '\0') {
			longjmp(byebye, -1);
		}
		clrtoeol();
		// transfer gbk username to utf8
		if(code_convert("gbk","utf8",newuser.userid, strlen(newuser.userid),useridutf8, 22 )==-1)
		{
			prints("不能识别的非简体中文或英文帐号\n");
			exit(0);
		}

		if (!goodgbid(newuser.userid)) {
			prints("不正确的中英文帐号\n");
		} else if (strlen(newuser.userid) < 3){
			prints("帐号至少需有三个字符!\n");
		} else if ((*newuser.userid == '\0')
			   || is_bad_id(newuser.userid)) {
			prints
			    ("抱歉, 您不能使用这个字作为帐号。 请想另外一个。\n");
		} else if ((r = user_registered(newuser.userid))) {
			if (r > 0)
				prints("此帐号已经有人使用\n");
			else
				prints
				    ("此帐号刚刚死亡待清理，请明天再来注册\n");
		} else if ((r = checkdiscuzuser(newuser.userid))) {
			if (r > 0)
				prints("此帐号已被人在Web下注册，帐号所有者可直接用原帐号登录\n");
			else
				prints("数据库内部错误，请报告给SYSOP\n");
		} else if (strlen(useridutf8)>DISCUZ_USERNAME_LENGTH)	{
			prints("此帐号超过了五个中文字符长度的限制\n");
		}
		else
			break;
	}
	while (1) {
		getdata(t_lines - 4, 0, "请设定您的密码 (Setup Password): ",
			passbuf, PASSLEN, NOECHO, YEA);
		if (strlen(passbuf) < 4 || !strcmp(passbuf, newuser.userid)) {
			prints("密码太短或与使用者代号相同, 请重新输入\n");
			continue;
		}
		strsncpy(passbuf1, passbuf, sizeof (passbuf1));
		getdata(t_lines - 3, 0,
			"请再输入一次你的密码 (Reconfirm Password): ", passbuf,
			PASSLEN, NOECHO, YEA);
		if (strncmp(passbuf, passbuf1, PASSLEN) != 0) {
			prints("密码输入错误, 请重新输入密码.\n");
			continue;
		}
		newuser.salt = getsalt_md5();
		genpasswd(md5pass, newuser.salt, passbuf);
		memcpy(newuser.passwd, md5pass, 16);
		break;
	}
	newuser.ip = 0;
	newuser.userdefine = 0xffffffff;
	if (!strcmp(newuser.userid, "guest")) {
		newuser.userlevel = 0;
		newuser.userdefine &=
		    ~(DEF_FRIENDCALL | DEF_ALLMSG | DEF_FRIENDMSG);
	} else {
		newuser.userlevel = PERM_BASIC;
		newuser.flags[0] = PAGER_FLAG | BRDSORT_FLAG2;
	}
	newuser.userdefine &= ~(DEF_NOLOGINSEND);
	newuser.userdefine &= ~(DEF_INTERNETMAIL);
	if (convcode)
		newuser.userdefine &= ~DEF_USEGB;

	newuser.flags[1] = 0;
	newuser.firstlogin = newuser.lastlogin = time(NULL);
	newuser.lastlogout = 0;
	allocid = getnewuserid(&newuser);
	if (allocid > MAXUSERS || allocid <= 0) {
		prints("No space for new users on the system!\n\r");
		refresh();
		exit(1);
	}
	if (!(usernum = getuser(newuser.userid, &currentuser))) {
		errlog("User failed to create, %s\n", newuser.userid);
		prints("User failed to create\n");
		refresh();
		exit(1);
	}
	sethomepath(genbuf, newuser.userid);
	mkdir(genbuf, 0775);
	tracelog("%s newaccount %d %s", newuser.userid, allocid, realfromhost);
	//		add new user to discuz database
	if(discuzreg(useridutf8, passbuf, newuser.salt)==-1)
	{
		char log[256];
		sprintf(log, "register id %s written to discuz database failed!\n", useridutf8);
		prints("register id %s written to discuz database failed!\n", useridutf8);
		writesyslog(ERRORLOG, log);
	}
	//		end of adding new user to discuz database

}

int
invalid_email(addr)
char *addr;
{
	FILE *fp;
	char temp[STRLEN];

	if ((fp = fopen(".bad_email", "r")) != NULL) {
		while (fgets(temp, STRLEN, fp) != NULL) {
			strtok(temp, "\n");
			if (strstr(addr, temp) != NULL) {
				fclose(fp);
				return 1;
			}
		}
		fclose(fp);
	}
	return 0;
}

static int
invalid_realmail(userid, email, msize)
char *userid, *email;
int msize;
{
	FILE *fn;
	char fname[STRLEN];
	struct stat st;

	//判断是否使用email注册... 不过是不是真的用这个判断的? --ylsdd
	if (sysconf_str("EMAILFILE") == NULL)
		return 0;

	if (strchr(email, '@') && valid_ident(email)
	    && USERPERM(currentuser, PERM_LOGINOK))
		return 0;

	sethomefile(fname, userid, "register");
	if (stat(fname, &st) == 0) {
#ifdef REG_EXPIRED
		now_t = time(0);
		if (now_t - st.st_mtime >= REG_EXPIRED * 86400) {
			sethomefile(fname, userid, "register.old");
			if (stat(fname, &st) == -1
			    || now_t - st.st_mtime >= REG_EXPIRED * 86400)
				return 1;
		}
#endif
	}
	sethomefile(fname, userid, "register");
	if ((fn = fopen(fname, "r")) != NULL) {
		fgets(genbuf, STRLEN, fn);
		fclose(fn);
		strtok(genbuf, "\n");
		if (valid_ident(genbuf) && ((strchr(genbuf, '@') != NULL)
					    || strstr(genbuf, "usernum"))) {
			if (strchr(genbuf, '@') != NULL)
				strncpy(email, genbuf, msize);
			move(21, 0);
			prints("恭贺您!! 您已顺利完成本站的使用者注册手续,\n");
			prints("从现在起您将拥有一般使用者的权利与义务...\n");
			pressanykey();
			return 0;
		}
	}
	return 1;
}

void
check_register_info()
{
	struct userdata udata;
	struct userec tmpu;
	char *newregfile;
	int perm, i;
	FILE *fout;
	char buf[192], buf2[STRLEN], buf3[STRLEN * 3];
	int info_changed = 0;
	//char occu_des[12][8]={"学生", "教育业", "计算机", "工程师",  "金融业",  "服务业",
	//			"商业", "公务员", "制造业", "互联网",  "传媒", "其他"};
	// for discuz database

	clear();
	memcpy(&tmpu, currentuser, sizeof (tmpu));
	sprintf(buf, "%s", email_domain());
	if (!(tmpu.userlevel & PERM_BASIC)) {
		tmpu.userlevel = 0;
		updateuserec(&tmpu, usernum);
		return;
	}
	loaduserdata(tmpu.userid, &udata);
	perm = PERM_DEFAULT & sysconf_eval("AUTOSET_PERM");
	while ((strlen(tmpu.username) < 2)) {
		getdata(2, 0, "请输入您的昵称 : ",
			tmpu.username, NAMELEN, DOECHO, YEA);
		info_changed = 1;
		strcpy(uinfo.username, tmpu.username);
		update_utmp();
	}
// 各位维护，下面这个判断要注意了，这是Firebird远古时代，给没有填写过注册单，PERM_LOGINOK为假的用户使用的
// 现在，只有新注册的账号需要执行这个if语句里头的内容
// 而整个函数是每次登录时user_login()都需要调用的。之所以放在register.c里面是因为当初架构没有做好。
// written by bridged
	if (!USERPERM(currentuser, PERM_LOGINOK)) {
// 由于政治环境的原因，我们不再强求用户必须输入姓名和城市等可能暴露隐私的选项
/*		while ((strlen(udata.realname) < 4)
		       || (strstr(udata.realname, "  "))
		       || (strstr(udata.realname, "　"))) { */
			move(3, 0);
			prints("请输入您的真实姓名（此项信息可不填） :\n");
			getdata(4, 0, "> ", udata.realname, NAMELEN, DOECHO,
				YEA);
//		}

/*		while ((strlen(udata.address) < 4)
		       || (strstr(udata.address, "   "))) {  */
			move(5, 0);
			prints("请输入您所在的城市 （此项信息可不填）：\n");
			getdata(6, 0, "> ", udata.address, NAMELEN, DOECHO,
				YEA);
//		}

#if 0	//为什么加上这一段就不行了啊？？！！
		while ( udata.occupation < 'a' || udata.occupation > 'l') {
			move(7, 0);
			prints( "a. 学生 b. 教育业 c. 计算机 d. 工程师  e. 金融业  f. 服务业\n");
			prints( "g. 商业 h. 公务员 i. 制造业 j.互联网   k.传媒     l. 其他");
			prints( "请选择您的职业(输入序号) ：\n");
			udata.occupation = getchar();
		}
#endif
		move(10, 0);
		prints("请输入您的联系电话（此项信息可不填） ：\n");
		getdata(11, 0, "> ", udata.phone, STRLEN, DOECHO, YEA);
		if (strchr(udata.email, '@') == NULL) {
			char buf[sizeof (udata.email)];
			move(12, 0);
			prints
			    ("\033[1;31m请输入您的电子邮件（用于找回密码，请务必认真填写）\n");
			move(13, 0);
		      EMAILAGAIN:
			getdata(14, 0, "> ", buf, sizeof (buf), DOECHO, YEA);
			strsncpy(udata.email, strtrim(buf), sizeof (udata.email));
			if (!strchr(udata.email, '@')){
				move(13, 0);
				prints("\033[1;32memail格式不正确，请输入正确的email地址。\033[m");
				goto EMAILAGAIN;
			}
			//		write new user information to discuz database
			if(discuzupdateemail(currentuser->userid, udata.email, currentuser->firstlogin)==-1)
			{
				char log[256];
				sprintf(log, "register id %s updating its email in discuz database failed!\n", currentuser->userid);
				writesyslog(ERRORLOG, log);
			}
			//		end of write new information to discuz database
		}
		saveuserdata(tmpu.userid, &udata);
	}
	if (!strcmp(tmpu.userid, "SYSOP")) {
		tmpu.userlevel = ~0;
		info_changed = 1;
	}
	else if(!(tmpu.userlevel & PERM_LOGINOK)){
		tmpu.userlevel |= PERM_DEFAULT;
		info_changed = 1;
		mail_file("etc/s_fill", tmpu.userid,
		  "恭禧您通过身份验证", "SYSOP");
		mail_file("etc/s_fill2", tmpu.userid,
		  "欢迎加入" MY_BBS_NAME "大家庭", "SYSOP");
	}
	if (!(currentuser->userlevel & PERM_LOGINOK)) {
		if (!invalid_realmail(tmpu.userid, udata.realmail, 60)) {
			sethomefile(buf, tmpu.userid, "sucessreg");
			if (((file_isfile(buf)) && !sysconf_str("EMAILFILE"))
			    || (sysconf_str("EMAILFILE"))) {
				tmpu.userlevel |= PERM_DEFAULT;
				info_changed = 1;
			}
		} else {
#if 0
			//sethomefile(buf, tmpu.userid, "mailcheck");
			if (!strstr(udata.email, buf) &&
			    !invalidaddr(udata.email) &&
			    !invalid_email(udata.email) &&
			    //!file_exist(buf) &&
			    trustEmail(udata.email)) {
				move(13, 0);
				prints("您的电子信箱 尚须通过回信验证...  \n"
				       "    本站将马上寄一封验证信给您,\n"
				       "    您只要从 %s 回信, 就可以成为本站正式公民.\n"
				       "    成为本站正式公民, 就能享有更多的权益喔!\n",
				       udata.email);
				move(20, 0);
				if (askyn("您要我们现在就寄这一封信吗", YEA, NA)
				    == YEA) {
					send_emailcheck(&tmpu, &udata);
					move(21, 0);
					prints
					    ("确认信已寄出, 等您回信哦!! 请按 <Enter> : ");
					pressreturn();
				}
			} else {
				showansi = 1;
				if (sysconf_str("EMAILFILE") != NULL) {
					prints
					    ("\n您所填写的电子邮件地址 【\033[1;33m%s\033[m】\n",
					     udata.email);
					prints
					    ("并非系统所信任的 email 帐号（不在列表中，或者使用过），\n"
					     "系统不会投递注册信，请到\033[1;32mInfoEdit->Info\033[m中修改，或者填写注册单...\n");
					pressanykey();
				}
			}
#endif
		}
	}
	if (tmpu.lastlogin - tmpu.firstlogin < 3 * 86400) {
		if (tmpu.numlogins <= 1) {
			clear();
			move(5, 0);
			prints
			    ("请作个简短的个人简介, 向本站其他使用者打个招呼\n");
			prints("(最多三行, 写完可直接按 <Enter> 跳离)....");
			buf3[0] = 0;
			for (i = 0; i < 3; i++) {
				getdata(7 + i, 0, ":", buf2, 75, DOECHO, YEA);
				if (!buf2[0])
					break;
				strcat(buf3, buf2);
				strcat(buf3, "\n");
			}
			pressanykey();
			sprintf(buf, "bbstmpfs/tmp/newcomer.%s",
				currentuser->userid);
			if ((fout = fopen(buf, "w")) != NULL) {
				fprintf(fout, "大家好,\n\n");
				fprintf(fout, "我是 %s (%s), 来自 %s\n",
					currentuser->userid, tmpu.username,
					fromhost);
				fprintf(fout,
					"今天我初来此站报到, 请大家多多指教。\n");
				if (buf3[0]) {
					fprintf(fout, "\n\n自我介绍:\n\n");
					fprintf(fout, "%s", buf3);
				}
				fclose(fout);
				postfile(buf, "newcomers", "新手上路....", 2);
				unlink(buf);
			}
		}
		newregfile = sysconf_str("NEWREGFILE");
		if (!USERPERM(currentuser, PERM_SYSOP) && newregfile != NULL) {
			tmpu.userlevel &= ~(perm);
			info_changed = 1;
			saveuserdata(tmpu.userid, &udata);
			ansimore(newregfile, YEA);
		}
	}
	if (info_changed)
		updateuserec(&tmpu, usernum);
}

static int
valid_ident(ident)
char *ident;
{
	static char *const invalid[] = {
		"unknown@", "root@", "gopher@", "bbs@",
		"guest@", "nobody@", "www@", NULL
	};
	int i;
	if (ident[0] == '@')
		return 0;
	for (i = 0; invalid[i] != NULL; i++)
		if (strstr(ident, invalid[i]) != NULL)
			return 0;
	return 1;
}
