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
    Copyright (C) 1999, Zhou Lin, kcn@cic.tsinghua.edu.cn
    
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

extern time_t login_start_time;
extern char fromhost[60];

static void getfield(int line, char *info, char *desc, char *buf, int len,
		     int min);

void
permtostr(perm, str)
int perm;
char *str;
{
	int num;
	strcpy(str, "bTCPRp#@XWBA#VS-DOM-F012345678");
	for (num = 0; num < 30; num++)
		if (!(perm & (1 << num)))
			str[num] = '-';
	str[num] = '\0';
}

void
disply_userinfo(u, real)
struct userec *u;
int real;
{
	struct stat st;
	struct userdata data;
	int num, diff;
	int exp;
	struct in_addr in;

	move(real == 1 ? 2 : 3, 0);
	clrtobot();
	loaduserdata(u->userid, &data);
	prints("您的代号     : %s\n", u->userid);
	prints("您的昵称     : %s\n", u->username);
	prints("真实姓名     : %s\n", data.realname);
	prints("所在城市     : %s\n", data.address);
	prints("电子邮件信箱 : %s\n", data.email);
	if (real) {
		prints("真实 E-mail  : %s\n", data.realmail);
//        if USERPERM(currentuser, PERM_ADMINMENU)
//           prints("Ident 资料   : %s\n", u->ident );
	}
	in.s_addr = u->ip;
	prints("域名指向     : %s\n", (u->ip == 0) ? "" : inet_ntoa(in));
	prints("帐号建立日期 : %s", ctime(&u->firstlogin));
	prints("最近光临日期 : %s", ctime(&u->lastlogin));
	if (real) {
		in.s_addr = u->lasthost;
		prints("最近光临机器 : %s\n", inet_ntoa(in));
	}
	prints("上次离站时间 : %s",
	       u->lastlogout ? ctime(&u->lastlogout) : "不详\n");
	prints("上站次数     : %d 次\n", u->numlogins);
	if (real) {
		prints("文章数目     : %d\n", u->numposts);
	}
	exp = countexp(u, 2);
	prints("经验值       : %d(%s)\n", exp, cuserexp(u->exp_group,exp));
	exp = countexp(u, 1);
	prints("精华值       : %d(%s)\n", exp, cuserexp(u->exp_group,exp));
	exp = countperf(u);
	prints("表现值       : %d(%s)\n", exp, cperf(exp));
	prints("上站总时数   : %d 小时 %d 分钟\n", (int) (u->stay / 3600),
	       (int) ((u->stay / 60) % 60));
	sprintf(genbuf, "mail/%c/%s/%s", mytoupper(u->userid[0]), u->userid,
		DOT_DIR);
	if (stat(genbuf, &st) >= 0)
		num = st.st_size / (sizeof (struct fileheader));
	else
		num = 0;
	prints("私人信箱     : %d 封\n", num);

	if (real) {
		strcpy(genbuf, "bTCPRp#@XWBA#VS-DOM-F012345678");
		for (num = 0; num < 30; num++)
			if (!(u->userlevel & (1 << num)))
				genbuf[num] = '-';
		genbuf[num] = '\0';
		permtostr(u->userlevel, genbuf);
		prints("使用者权限   : %s\n", genbuf);
	} else {
		diff = (time(0) - login_start_time) / 60;
		prints("停留期间     : %d 小时 %02d 分\n", diff / 60,
		       diff % 60);
		prints("荧幕大小     : %dx%d\n", t_lines, t_columns);
	}
	prints("\n");
	if (u->userlevel & PERM_LOGINOK) {
		prints("  您的注册程序已经完成, 欢迎加入本站.\n");
	} else if (u->lastlogin - u->firstlogin < 3 * 86400) {
		prints("  新手上路, 请阅读 Announce 讨论区.\n");
	} else {
		prints("  注册尚未成功, 请参考本站进站画面说明.\n");
	}
}

int
uinfo_query(u, real)
struct userec *u;
int real;
{
	struct userec newurec;
	struct userdata data, newdata;
	char ans[3], buf[STRLEN], genbuf[128];
	char src[STRLEN], dst[STRLEN];
	char md5pass[16];
	int i, fail = 0, netty_check = 0;
	FILE *fin, *fout, *dp;
	time_t code;
	struct in_addr in;

	memcpy(&newurec, u, sizeof (struct userec));
	loaduserdata(u->userid, &data);
	memcpy(&newdata, &data, sizeof (struct userdata));
	getdata(t_lines - 1, 0,
		"请选择 (0)结束 (1)修改资料 (2)设定密码 (3)选经验值系统==> [0]", ans, 2,
		DOECHO, YEA);
	clear();

	i = 3;
	move(i++, 0);
	if (ans[0] != '3' || real)
		prints("使用者代号: %s\n", u->userid);

	switch (ans[0]) {
	case '1':
		move(1, 0);
		prints("请逐项修改,直接按 <ENTER> 代表使用 [] 内的资料。\n");

		sprintf(genbuf, "昵称 [%s]: ", u->username);
		getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, YEA);
		if (buf[0])
			strncpy(newurec.username, buf, NAMELEN);
		if (!real && buf[0])
			strncpy(uinfo.username, buf, 40);
		if (real) {
			sprintf(genbuf, "真实姓名 [%s]: ", data.realname);
			getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, YEA);
			if (buf[0])
				strsncpy(newdata.realname, buf,
					 sizeof (newdata.realname));
		}
		sprintf(genbuf, "所在城市 [%s]: ", data.address);
		getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, YEA);
		if (buf[0] && strlen(buf) >= 4)
			strsncpy(newdata.address, buf,
				 sizeof (newdata.address));
		else if(buf[0]){
			move(i++, 0);
			prints("所在城市太短，至少2个中文字。");
		}
		sprintf(genbuf, "电子信箱 [%s]: ", data.email);
		getdata(i++, 0, genbuf, buf, STRLEN - 10, DOECHO, YEA);
		if (buf[0]) {
			strsncpy(newdata.email, buf, sizeof (newdata.email));
		}
		in.s_addr = u->ip;
		sprintf(genbuf, "域名指向 [%s]: ",
			(u->ip == 0) ? "" : inet_ntoa(in));
		getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
		if (buf[0]) {
			if (inet_aton(buf, &in))
				newurec.ip = in.s_addr;
		}
		if (real) {
			sprintf(genbuf, "真实Email[%s]: ", data.realmail);
			getdata(i++, 0, genbuf, buf, 60, DOECHO, YEA);
			if (buf[0])
				strsncpy(newdata.realmail, buf,
					 sizeof (newdata.realmail));

			sprintf(genbuf, "上线次数 [%d]: ", u->numlogins);
			getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
			if (atoi(buf) > 0)
				newurec.numlogins = atoi(buf);

			sprintf(genbuf, "文章数目 [%d]: ", u->numposts);
			getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
			if (atoi(buf) > 0)
				newurec.numposts = atoi(buf);

			sprintf(genbuf, "上站小时数 [%ld 小时 %ld 分钟]: ",
				(long int) (u->stay / 3600),
				(long int) ((u->stay / 60) % 60));
			getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
			if (atoi(buf) > 0)
				newurec.stay = atoi(buf) * 3600;

			sprintf(genbuf, "精华值 [%d]: ", data.extraexp);
			getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
			if (atoi(buf) > 0)
				newdata.extraexp = atoi(buf);
		}

		break;
	case '2':
		if (!real) {
			getdata(i++, 0, "请输入原密码: ", buf, PASSLEN, NOECHO,
				YEA);
			if (*buf == '\0'
			    || !checkpasswd(u->passwd, u->salt, buf)) {
				prints("\n\n很抱歉, 您输入的密码不正确。\n");
				fail++;
				break;
			}
		}
		getdata(i++, 0, "请设定新密码: ", buf, PASSLEN, NOECHO, YEA);
		if (buf[0] == '\0') {
			prints("\n\n密码设定取消, 继续使用旧密码\n");
			fail++;
			break;
		}
		strsncpy(genbuf, buf, sizeof (genbuf));

		getdata(i++, 0, "请重新输入新密码: ", buf, PASSLEN, NOECHO,
			YEA);
		if (strncmp(buf, genbuf, PASSLEN)) {
			prints("\n\n新密码确认失败, 无法设定新密码。\n");
			fail++;
			break;
		}
		newurec.salt = getsalt_md5();
		genpasswd(md5pass, newurec.salt, buf);
		memcpy(newurec.passwd, md5pass, 16);
		// change password in discuzx
		if (changediscuzpasswd(newurec.userid, newurec.salt, buf, 1)<0)
			prints("\n\n同步web密码失败。本帐号访问web可能会产生问题，请向sysop报告。\n");
		break;
	case '3':
		move(4,0);
#if 1 
		prints("长系(100级) \033[1;32m[1]古代官职\033[m\n\n");
#endif
		prints("短系(11级)  \033[1;32m[A]如故 [B]赌系 [C]修为 [D]武功 [E]菜系 [F]歌系 [G]兵器 [H]水系\033[m\n"
		       "            \033[1;32m[I]经验 [J]生物 [K]西游 [L]爱情 [M]光光 [N]军衔 [O]轮回 [P]佛教\033[m\n"
		       "            \033[1;32m[Q]学历 [R]经典 [S]酒令 [T]女生 [U]隐藏 [V]麻将 [W]恐龙 [X]天使\033[m\n");
		getdata(t_lines - 1, 0,
		"请选择您喜欢的经验值体系，按[0]退出。", ans, 2, DOECHO, YEA);
		ans[0] = toupper(ans[0]);
		if(ans[0]>='A' && ans[0]<='X')
			newurec.exp_group = ans[0];
		else if(ans[0]=='1'){
			newurec.exp_group = ans[0];
		}
		else if(ans[0]=='0'){
                       fail++;
                       break;
		}
		else
			fail++;
		break;
	default:
		clear();
		return 0;
	}
	if (fail != 0) {
		pressreturn();
		clear();
		return 0;
	}
	if (askyn("确定要改变吗", NA, YEA) == YEA) {
		if (real) {
			char secu[STRLEN];
			sprintf(secu, "修改 %s 的基本资料或密码。", u->userid);
			securityreport(secu, secu);
		}
/* added by netty to automatically send a mail to new user. */

		if ((netty_check == 1)) {
			sprintf(genbuf, "%s", email_domain());
			if ((sysconf_str("EMAILFILE") != NULL) &&
			    (!strstr(newdata.email, genbuf)) &&
			    (!invalidaddr(newdata.email)) &&
			    (!invalid_email(newdata.email))) {
				randomize();
				code = (time(0) / 2) + (rand() / 10);
				sethomefile(genbuf, u->userid, "mailcheck");
				if ((dp = fopen(genbuf, "w")) == NULL) {
					fclose(dp);
					return -2;
				}
				fprintf(dp, "%9.9ld\n", (long int) code);
				fclose(dp);
				sprintf(genbuf,
					"/usr/lib/sendmail -f %s.bbs@%s %s ",
					u->userid, email_domain(),
					newdata.email);
				fout = popen(genbuf, "w");
				fin = fopen(sysconf_str("EMAILFILE"), "r");
				if (fin == NULL || fout == NULL)
					return -1;
				fprintf(fout, "Reply-To: SYSOP.bbs@%s\n",
					email_domain());
				fprintf(fout, "From: SYSOP.bbs@%s\n",
					email_domain());
				fprintf(fout, "To: %s\n", newdata.email);
				fprintf(fout,
					"Subject: @%s@[-%9.9ld-]%s mail check.\n",
					u->userid, (long int) code, MY_BBS_ID);
				fprintf(fout, "X-Forwarded-By: SYSOP \n");
				fprintf(fout,
					"X-Disclaimer: %s registration mail.\n",
					MY_BBS_ID);
				fprintf(fout, "\n");
				fprintf(fout, "BBS LOCATION     : %s (%s)\n",
					email_domain(), MY_BBS_IP);
				fprintf(fout, "YOUR BBS USER ID : %s\n",
					u->userid);
				fprintf(fout, "APPLICATION DATE : %s",
					ctime(&u->firstlogin));
				fprintf(fout, "LOGIN HOST       : %s\n",
					fromhost);
				fprintf(fout, "YOUR NICK NAME   : %s\n",
					u->username);
				fprintf(fout, "YOUR NAME        : %s\n",
					data.realname);
				while (fgets(genbuf, 255, fin) != NULL) {
					if (genbuf[0] == '.'
					    && genbuf[1] == '\n')
						fputs(". \n", fout);
					else
						fputs(genbuf, fout);
				}
				fprintf(fout, ".\n");
				fclose(fin);
				pclose(fout);
			} else {
				if (sysconf_str("EMAILFILE") != NULL) {
					move(t_lines - 5, 0);
					prints
					    ("\n您所填的电子邮件地址 【\033[1;33m%s\033[m】\n",
					     newdata.email);
					prints
					    ("并非合法之 UNIX 帐号，系统不会投递注册信，请把它修正好...\n");
					pressanykey();
				}
			}
		}
		if (netty_check == 1) {
			newurec.userlevel &= ~(PERM_LOGINOK | PERM_PAGE);
			sethomefile(src, newurec.userid, "register");
			sethomefile(dst, newurec.userid, "register.old");
			rename(src, dst);
		}
		updateuserec(&newurec, 0);
		saveuserdata(newurec.userid, &newdata);
	}
	clear();
	return 0;
}

void
x_info()
{
	modify_user_mode(GMENU);
	if (!strcmp("guest", currentuser->userid)) {
		disply_userinfo(currentuser, 0);
		pressreturn();
		return;
	}
	disply_userinfo(currentuser, 1);
	uinfo_query(currentuser, 0);
}

static void
getfield(line, info, desc, buf, len, min)
int line, len, min;
char *info, *desc, *buf;
{
	char prompt[STRLEN];

	sprintf(genbuf, "  原先设定: %-46.46s \033[1;32m(%s)\033[m",
		(buf[0] == '\0') ? "(未设定)" : buf, info);
	move(line, 0);
	prints("%s", genbuf);
	if (min)
		sprintf(prompt, "  %s(至少%d个汉字): ", desc, (min + 1) / 2);
	else
		sprintf(prompt, "  %s(可选): ", desc);
	do {
		getdata(line + 1, 0, prompt, genbuf, len, DOECHO, YEA);
		if (genbuf[0] == 0)
			strcpy(genbuf, buf);
	} while (strlen(genbuf) < min);
	strncpy(buf, genbuf, len);
	move(line, 0);
	clrtoeol();
	prints("  %s: %s\n", desc, buf);
	clrtoeol();
}

//#define AUTO_REG 1
void
x_fillform()
{
	char rname[NAMELEN], addr[STRLEN];
	char phone[STRLEN], dept[STRLEN], assoc[STRLEN];
	char ans[5], *mesg;
	FILE *fn;
	time_t now;
	int lockfd;
	struct userdata currentdata;

	modify_user_mode(NEW);
	move(3, 0);
	clrtobot();
	if (!strcmp("guest", currentuser->userid)) {
		prints("抱歉, 请用 new 申请一个新帐号后再填申请表.");
		pressreturn();
		return;
	}
	if (currentuser->userlevel & PERM_LOGINOK) {
		prints("您已经完成本站的使用者注册手续, 欢迎加入本站的行列.");
		pressreturn();
		return;
	}

	if (has_fill_form(currentuser->userid)) {
		prints("站长尚未处理您的注册申请单, 请耐心等候.");
		pressreturn();
		return;
	}
	move(3, 0);
	sprintf(genbuf, "您要填写注册单，加入%s大家庭吗？\n"
		"\033[1;32m(使用email注册的无需填写，选n略过。)\033[m", MY_BBS_NAME);
	if (askyn(genbuf, YEA, NA) == NA)
		return;
	loaduserdata(currentuser->userid, &currentdata);
	strncpy(rname, currentdata.realname, NAMELEN);
	strncpy(addr, currentdata.address, STRLEN);
	dept[0] = phone[0] = assoc[0] = '\0';
	while (1) {
		move(3, 0);
		clrtoeol();
		prints("%s 您好, 请据实填写以下的资料:\n", currentuser->userid);
		getfield(6, "请用中文", "真实姓名", rname, NAMELEN, 4);
		getfield(8, "学校系级或公司名称", "学校系级或工作单位", dept,
			 STRLEN, 14);
		getfield(10, "包括寝室或门牌号码", "目前详细住址", addr,
			 STRLEN, 16);
		getfield(12, "包括可联络时间", "联络电话", phone, STRLEN, 0);
		getfield(14, "校友会或毕业学校", "校 友 会", assoc, STRLEN, 0);
/* only for 9#        getfield( 14, "介绍人ID和真实姓名",    "介绍人(外来的ID须填:ID/姓名)",   assoc, STRLEN );
*/
		mesg = "以上资料是否正确, 按 Q 放弃注册 (Y/N/Quit)? [N]: ";
		getdata(t_lines - 1, 0, mesg, ans, 3, DOECHO, YEA);
		if (ans[0] == 'Q' || ans[0] == 'q')
			return;
		if (ans[0] == 'Y' || ans[0] == 'y')
			break;
	}
#ifndef AUTO_REG
	lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX);
	if ((fn = fopen("new_register", "a")) != NULL) {
		now = time(NULL);
		fprintf(fn, "usernum: %d, %s", usernum, ctime(&now));
		fprintf(fn, "userid: %s\n", currentuser->userid);
		fprintf(fn, "realname: %s\n", rname);
		fprintf(fn, "dept: %s\n", dept);
		fprintf(fn, "addr: %s\n", addr);
		fprintf(fn, "phone: %s\n", phone);
		fprintf(fn, "assoc: %s\n", assoc);
		fprintf(fn, "rereg: 1\n");
		fprintf(fn, "----\n");
		fclose(fn);
	}
	close(lockfd);
	setuserfile(genbuf, "mailcheck");
	if ((fn = fopen(genbuf, "w")) == NULL) {
		fclose(fn);
		return;
	}
	fprintf(fn, "usernum: %d\n", usernum);
	fclose(fn);
#else
	snprintf(currentdata.realmail, sizeof (currentdata.realmail),
		 "%s$%s@auto", dept, phone);
	strncpy(currentdata.address, addr, sizeof (currentdata.address));
	strncpy(currentdata.realname, rname, sizeof (currentdata.realname));
	saveuserdata(currentuser->userid, &currentdata);
	sethomefile(buf, currentuser->userid, "register");
	if (file_exist(buf)) {
		sethomefile(buf2, currentuser->userid, "register.old");
		rename(buf, buf2);
	}
	if ((fn = fopen(buf, "w")) != NULL) {
		fprintf(fn, "usernum: %d, %s", usernum, ctime(&now));
		fprintf(fn, "userid: %s\n", currentuser->userid);
		fprintf(fn, "realname: %s\n", rname);
		fprintf(fn, "dept: %s\n", dept);
		fprintf(fn, "addr: %s\n", addr);
		fprintf(fn, "phone: %s\n", phone);
		fprintf(fn, "assoc: %s\n", assoc);
		fprintf(fn, "rereg: 1\n");
		fprintf(fn, "----\n");
		fclose(fn);
	}
	memcpy(&tmpu, currentuser, sizeof (tmpu));
	tmpu.userlevel |= PERM_DEFAULT;
	updateuserec(&tmpu, usernum);
	mail_file("etc/s_fill", currentuser->userid,
		  "恭禧您通过身份验证", "SYSOP");
	mail_file("etc/s_fill2", currentuser->userid,
		  "欢迎加入" MY_BBS_NAME "大家庭", "SYSOP");
#endif
}
