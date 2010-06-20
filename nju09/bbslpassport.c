#include "bbslib.h"
#include <openssl/des.h>

static des_cblock iv;

int key_fail;

static unsigned char magic[2][8];
static des_key_schedule ks[2][3];

static int
load_key(char *file, int key)
{
	FILE *fp;
	des_cblock k;
	int i;
	fp = fopen(file, "r");
	if (NULL == fp)
		return -1;
	fread(magic[key], 1, 8, fp);
	for (i = 0; i < 3; i++) {
		fread(&k, 1, sizeof (k), fp);
		des_set_key_checked(&k, ks[key][i]);
	}
	memcpy(&iv, &k, sizeof (k));
	fclose(fp);
	return 0;
}

int
load_all_key(void)
{
	if (load_key("etc/my_key", 0))
		return -1;
	if (load_key("etc/service_key", 1))
		return -1;
	return 0;
}

char *
des3_encode(char *id, int key)
{
	unsigned char buf[24], out[24];
	static char result[65];
	des_cblock this_iv;
	memcpy(&this_iv, &iv, sizeof (iv));
	memcpy(buf, &now_t, 4);
	memcpy(buf + 4, id, IDLEN);
	memcpy(buf + 16, magic[key], 8);
	des_ede3_cbc_encrypt(buf, out, 24, ks[key][0], ks[key][1],
			     ks[key][2], &iv, 1);
	hex_encode(this_iv, 8, result);
	hex_encode(out, 24, result + 16);
	return result;
}

char *
des3_decode(char *buf, int key)
{
	des_cblock this_iv;
	char tmp[24], out[16];
	static char id[IDLEN + 1];
	id[0] = 0;
	if (strlen(buf) != 64)
		return id;
	strsncpy(tmp, buf, 17);
	hex_decode(tmp, this_iv);
	hex_decode(buf + 16, tmp);
	des_ede3_cbc_encrypt(tmp, out, 24, ks[key][0], ks[key][1],
			     ks[key][2], &this_iv, 0);
	if (memcmp(out + 16, magic[key], 8))
		return id;
	if (now_t < *((time_t *) out) || now_t > *((time_t *) out) + 86400)
		return id;
	strsncpy(id, out + 4, IDLEN + 1);
	return id;
}

int
bbslpassport_main()
{
	int uid, infochanged = 0;
	char id[IDLEN + 1], pw[PASSLEN], site[256], md5pass[MD5LEN], buf[384];
	struct userec *x, tmpu;
	time_t t, dtime;
	html_header(3);
	strsncpy(id, strtrim(getparm("id")), IDLEN + 1);
	strsncpy(pw, getparm("pw"), PASSLEN);
	strsncpy(site, getparm("site"), 256);

	if (!id[0])
		http_fatal("请输入帐号");
	if (!site[0])
		http_fatal("no...");
	if (key_fail)
		http_fatal("内部错误, 联系维护!\n");
	if ((uid = getuser(id, &x)) <= 0) {
		printf("%s<br>", id);
		http_fatal("错误的使用者帐号");
	}
	strcpy(id, x->userid);
	if (!strcasecmp(id, "guest"))
		http_fatal("错误的使用者帐号");

	if (checkbansite(fromhost)) {
		http_fatal
		    ("对不起, 本站不欢迎来自 [%s] 的登录. <br>若有疑问, 请与SYSOP联系.",
		     fromhost);
	}
	if (userbansite(x->userid, fromhost))
		http_fatal("本ID已设置禁止从%s登录", fromhost);
	if (!checkpasswd(x->passwd, x->salt, pw)) {
		logattempt(x->userid, fromhost, "PASSPORT", now_t);
		http_fatal
		    ("密码错误，如有疑问请联系站务组，提供注册资料找回密码");
	}
#if 0
	if (!user_perm(x, PERM_BASIC))
		http_fatal
		    ("由于本帐号名称不符合帐号管理办法，已经被管理员禁止继续上站。<br>请用其他帐号登录在 <font color=red>"
		     DEFAULTBOARD "</font> 版询问.");
	if (file_has_word(MY_BBS_HOME "/etc/prisonor", x->userid)) {
		if (x->inprison == 0) {
			memcpy(&tmpu, x, sizeof (tmpu));
			tmpu.inprison = 1;
			tmpu.dieday = 2;
			updateuserec(&tmpu, 0);
		}
		http_fatal("安心改造，不要胡闹");
	}
	if (x->dieday)
		http_fatal("死了?还要做什么? :)");
#endif
	t = x->lastlogin;
	memcpy(&tmpu, x, sizeof (tmpu));
	if (tmpu.salt == 0) {
		tmpu.salt = getsalt_md5();
		genpasswd(md5pass, tmpu.salt, pw);
		memcpy(tmpu.passwd, md5pass, MD5LEN);
		infochanged = 1;
	}
#if 1
	if (count_uindex(uid) == 0) {
		if (now_t - t > 1800)
			tmpu.numlogins++;
		infochanged = 1;
		tmpu.lastlogin = now_t;
		dtime = t - 4 * 3600;
		t = localtime(&dtime)->tm_mday;
		dtime = now_t - 4 * 3600;
		if (t < localtime(&dtime)->tm_mday && x->numdays < 60000) {
			tmpu.numdays++;
		}
	}
#endif
	if (abs(t - now_t) < 20) {
		http_fatal("两次登录间隔过密!");
	}

	if (x->lasthost != from_addr.s_addr) {
		tmpu.lasthost = from_addr.s_addr;
		infochanged = 1;
	}
	if (infochanged)
		updateuserec(&tmpu, 0);
	tracelog("%s enter %s passport %d %s", x->userid, fromhost, infochanged,
		 getsenv("HTTP_X_FORWARDED_FOR"));
	printf
	    ("<script>exDate = new Date; exDate.setMonth(exDate.getMonth()+9);"
	     "document.cookie='pp=%s;path=/;expires=' + exDate.toGMTString();</script>",
	     des3_encode(id, 0));
	snprintf(buf, sizeof (buf), "http://%s?q=%s", site, des3_encode(id, 1));
	redirect(buf);
	http_quit();
	return 0;
}
