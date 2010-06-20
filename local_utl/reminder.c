//by ecnegrevid 2001.9.29
//提醒信息后面可以附加内容, 附加的内容写到etc/reminder.extra中
#include "../include/bbs.h"
#include "ythtlib.h"
#include "ythtbbs.h"

int
sendreminder(struct userec *urec)
{
	char cmd[1024], *ptr;
	time_t t;
	FILE *fp;
	struct userdata udata;
	static struct mmapfile mf = { ptr:NULL };
	if (!mf.ptr)
		mmapfile(MY_BBS_HOME "/etc/reminder.extra", &mf);
	loaduserdata(urec->userid, &udata);
	if (strchr(udata.email, '@') == NULL)
		return 0;
	ptr = udata.email;
	while (*ptr) {
		if (!isalnum(*ptr) && !strchr(".@", *ptr))
			return 0;
		ptr++;
	}
	if (strcasestr(udata.email, ".bbs@ytht.") != NULL)
		return 0;
	sprintf(cmd, "/usr/lib/sendmail -f SYSOP.bbs@%s %s", MY_BBS_DOMAIN,
		udata.email);
	fp = popen(cmd, "w");
	if (fp == NULL)
		return 0;
	fprintf(fp, "Return-Path: SYSOP.bbs@%s\n", MY_BBS_DOMAIN);
	fprintf(fp, "Reply-To: SYSOP.bbs@%s\n", MY_BBS_DOMAIN);
	fprintf(fp, "From: SYSOP.bbs@%s\n", MY_BBS_DOMAIN);
	fprintf(fp, "To: %s\n", udata.email);
	fprintf(fp, "Subject: 系统提醒（" MY_BBS_NAME "）\n\n");
	fprintf(fp,
		"    " MY_BBS_NAME "(" MY_BBS_DOMAIN
		")的用户您好，您在本站注册的\n"
		"帐号 %s 现在生命力已经降低到 10，需要您用\n"
		"该帐号登陆一次才能使生命力恢复。如果该帐户并不是\n"
		"您注册的，忽略这封信就可以了。\n\n" "关于生命力的说明:\n"
		"    在BBS系统上，每个帐号都有一个生命力，在用户不\n"
		"登录的情况下，生命力每天减少1，等生命力减少到0的时\n"
		"候，帐号就会自动消失。帐号每次登录后生命力就恢复到\n"
		"一个固定值，对于通过注册而且已经登录4次的用户，这个\n"
		"固定值至少是120；对于通过注册但登录少于4次的用户，\n"
		"这个固定值是30；对于未通过注册的用户，这个固定值是15。\n\n",
		urec->userid);
	if (mf.ptr)
		fwrite(mf.ptr, 1, mf.size, fp);
	fputs("\n.\n", fp);
	pclose(fp);
	if ((fp = fopen(MY_BBS_HOME "/reminder.log", "a")) != NULL) {
		t = time(NULL);
		ptr = ctime(&t);
		ptr[strlen(ptr) - 1] = 0;
		fprintf(fp, "%s %s %s\n", ptr, urec->userid, udata.email);
		fclose(fp);
	}
	sleep(2);
	return 0;
}

int
main(int argc, char *argv[])
{
	int fd1;
	struct userec rec;
	int size1 = sizeof (rec);

	if ((fd1 = open(PASSFILE, O_RDONLY, 0660)) == -1) {
		perror("open PASSWDFILE");
		return -1;
	}

	while (read(fd1, &rec, size1) == size1) {
		if (!rec.userid[0])
			continue;
		if (countlife(&rec) == 10||!strcmp(rec.userid,"lepton")) {
			sendreminder(&rec);
		}
	}
	close(fd1);
	return 0;
}
