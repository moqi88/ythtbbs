#include "bbs.h"
#define BLOCKFILE ".blockmail"
#define BUFLEN 256
//#define SKIPATTACH

int has_a = 0;
struct bbsinfo bbsinfo;

int str_decode(register unsigned char *dst, register unsigned char *src);

struct userec *checkuser;

void
chop(char *s)
{
	int i;
	i = strlen(s);
	if (s[i - 1] == '\n')
		s[i - 1] = 0;
	return;
}

int
dosearchuser(userid)
char *userid;
{
	if (getuser(userid, &checkuser) > 0)
		return 1;
	else
		return 0;
}

void
decode_mail(FILE * fin, FILE * fout)
{
	char filename[BUFLEN];
	char encoding[BUFLEN];
	char buf[BUFLEN];
	char sbuf[BUFLEN + 20];
#ifdef SKIPATTACH
#else
	char dbuf[BUFLEN + 20];
	int wc;
#endif
	char ch;
	int sizep;
	while (fgets(filename, sizeof (filename), fin)) {
		if (!fgets(encoding, sizeof (encoding), fin))
			return;
		chop(filename);
		chop(encoding);
		ch = 0;
		sizep = 0;
		if (filename[0]) {
			has_a = 1;
			str_decode(buf, filename);
			strsncpy(filename, buf, sizeof (filename));
			printf("f:%s\n", filename);
#ifdef SKIPATTACH
			fprintf(fout, "\n%s, ¸½¼þ±»ºöÂÔ\n", filename);
#else
			fprintf(fout, "\nbeginbinaryattach %s\n", filename);
			fwrite(&ch, 1, 1, fout);
			sizep = ftell(fout);
			fwrite(&sizep, sizeof (int), 1, fout);
#endif
		}
		if (!strcmp(encoding, "quoted-printable")
		    || !strcmp(encoding, "base64")) {
			ch = encoding[0];
			sprintf(sbuf, "=??%c?", ch);
			while (fgets(buf, sizeof (buf), fin)) {
#ifdef SKIPATTACH
#else
				int hasr = 0;
#endif
				if (!buf[0] && buf[1] == '\n')
					break;
#ifdef SKIPATTACH
#else
				strsncpy(sbuf + 5, buf, sizeof (buf));	//sbuf must be larger than buf
				chop(sbuf);
				if (ch == 'q') {
					if (sbuf[strlen(sbuf) - 1] == '=')
						sbuf[strlen(sbuf) - 1] = 0;
					else
						hasr = 1;
				}
				strcat(sbuf, "?=");
				wc = str_decode(dbuf, sbuf);
				if (wc >= 0) {
					fwrite(dbuf, wc, 1, fout);
					if (hasr)
						fputc('\n', fout);
				} else
					fputs(buf, fout);
#endif
			}
		} else {
			while (fgets(buf, sizeof (buf), fin)) {
				if (!buf[0] && buf[1] == '\n')
					break;
				fputs(buf, fout);
			}
		}
		if (sizep) {
			sizep = ftell(fout) - sizep - sizeof (int);
			fseek(fout, -sizep - 4, SEEK_CUR);
			sizep = htonl(sizep);
			fwrite(&sizep, sizeof (int), 1, fout);
			sizep = ntohl(sizep);
			fseek(fout, sizep, SEEK_CUR);
		}
	}
	if (sizep)
		fputs("\n\n--\n", fout);
}

int
append_mail(fin, sender1, id, sender, userid, title, received, spam)
FILE *fin;
char *id, *userid, *sender1, *sender, *title, *received, *spam;
{
	char buf[BUFLEN], genbuf[BUFLEN];
	char maildir[BUFLEN], path[BUFLEN];
	char filename[256];
	struct stat st;
	FILE *fout, *dp, *rmail;
	int passcheck = 0;
	char conv_buf[BUFLEN], *p1, *p2;
	time_t tmp_time;
	int ret = 0; 
	struct yspam_ctx *yctx;
	unsigned long long mailid;
	int isspam;

	isspam = atoi(spam);
/* check if the userid is in our bbs now */
	if (!dosearchuser(userid))
		return -4;
	if (!(checkuser->userdefine & DEF_INTERNETMAIL))
		return -5;
	if (!strcmp(sender, "MAILER-DAEMON@" MY_BBS_DOMAIN)) {
		fout = fopen("mailer-err.log", "a");
		if (!fout)
			return -6;
		while (fgets(genbuf, sizeof (genbuf), fin) != NULL)
			fputs(genbuf, fout);
		fclose(fout);
		return 0;
	}
	get_mailsize(checkuser);
	if (!strcasecmp(userid, "guest"))
		return -7;

/* check for the mail dir for the userid */
	snprintf(genbuf, sizeof (genbuf), "mail/%c/%s", mytoupper(userid[0]),
		 userid);

	if (stat(genbuf, &st) == -1) {
		if (mkdir(genbuf, 0755) == -1)
			return -8;
	} else {
		if (!(st.st_mode & S_IFDIR))
			return -9;
	}

	printf("Ok, dir is %s\n", genbuf);

	str_decode(conv_buf, sender);
	strsncpy(sender, conv_buf, BUFLEN);
	str_decode(conv_buf, title);

	while (!passcheck && !strcmp(userid, "SYSOP")
	       && strstr(conv_buf, " mail check.")) {
		passcheck = 1;
		if ((!strstr(sender, "bbs")) && (strchr(conv_buf, '@'))) {

			p1 = strchr(conv_buf, '@');
			if (!p1)
				break;
			p2 = strchr(p1 + 1, '@');
			if (!p2)
				break;
			*p2 = 0;
			strsncpy(sender1, p1 + 1, IDLEN + 1);
			strcpy(userid, sender1);
			snprintf(genbuf, sizeof (genbuf),
				 "home/%c/%s/mailcheck", mytoupper(sender1[0]),
				 sender1);
			if ((dp = fopen(genbuf, "r")) != NULL) {
				printf("open mailcheck\n");
				fgets(buf, sizeof (buf), dp);
				fclose(dp);
				sprintf(buf, "%9.9s", buf);
				if (dosearchuser(sender1)
				    && strstr(conv_buf, buf)
				    /*&&strstr(sender,checkuser.email) */
				    ) {
					printf("pass1\n");

					unlink(genbuf);
					passcheck = 5;
					/*Modify for SmallPig */
					snprintf(genbuf, sizeof (genbuf),
						 "home/%c/%s/register",
						 mytoupper(sender1[0]),
						 sender1);
					if (file_isfile(genbuf)) {
						snprintf(buf, sizeof (buf),
							 "home/%c/%s/register.old",
							 mytoupper(sender1[0]),
							 sender1);
						rename(genbuf, buf);
					}
					if ((fout = fopen(genbuf, "w")) != NULL) {
						fprintf(fout, "%s\n", sender);
						fclose(fout);
					}
				}
			}
		}
	}

/* allocate a record for the new mail */
	printf("Ok, the file is %s\n", maildir);
	sprintf(path, "bbstmpfs/tmp/mail2bbs.%d", getpid());
/* copy the stdin to the specified file */
	if ((fout = fopen(path, "w")) == NULL) {
		printf("Cannot open %s \n", maildir);
		return -1;
	}
	time(&tmp_time);
	if (passcheck >= 1) {
		fprintf(fout, "Ç×°®µÄ %s:\n", sender1);
		sprintf(maildir, "etc/%s",
			(passcheck == 5) ? "smail" : "fmail");
		if ((rmail = fopen(maildir, "r")) != NULL) {
			while (fgets(genbuf, sizeof (genbuf), rmail) != NULL)
				fputs(genbuf, fout);
			fclose(rmail);
		}
	} else
		decode_mail(fin, fout);
	fclose(fout);
	mailid = strtoull(id, NULL, 10);
	if (!isspam) {
		ret = mail_file(path, userid, conv_buf, sender);
		if (ret > 0) {
			sprintf(filename, "M.%d.A", ret);
			yctx = yspam_init("127.0.0.1");
			yspam_update_filename(yctx, mailid, filename); 
			yspam_fini(yctx);
		}
	}
	sprintf(filename, MY_BBS_HOME"/maillog/%llu.bbs", mailid);
	copyfile(path, filename);
	unlink(path);
	return ret;
/* append the record to the MAIL control file */
}

int
block_mail(addr)
char *addr;
{
	FILE *fp;
	char temp[STRLEN];

	if ((fp = fopen(BLOCKFILE, "r")) != NULL) {
		while (fgets(temp, STRLEN, fp) != NULL) {
			strtok(temp, "\n");
			if (strstr(addr, temp)) {
				fclose(fp);
				return 1;
			}
		}
		fclose(fp);
	}
	return 0;
}

int
main(argc, argv)
int argc;
char *argv[];
{

	char myarg[6][BUFLEN];
	char nettyp[BUFLEN];
	char *p;
	int i;

	if (initbbsinfo(&bbsinfo) < 0) {
		return -1;
	}
	if (uhash_uptime() == 0) {
		return -2;
	}
	for (i = 0; i < 6; i++) {
		if (!fgets(myarg[i], sizeof (myarg[i]), stdin))
			return -(4 + i);
	}
	for (i = 0; i < 6; i++)
		chop(myarg[i]);

	chdir(MY_BBS_HOME);
	setreuid(BBSUID, BBSUID);
	setregid(BBSGID, BBSGID);
	strsncpy(nettyp, myarg[1], sizeof (nettyp));
	p = strchr(nettyp, '@');
	if (NULL != p)
		*p = 0;

	if (block_mail(myarg[1]) == YEA)
		return -3;
	return append_mail(stdin, nettyp, myarg[0], myarg[1], myarg[2],
			   myarg[3], myarg[4], myarg[5]);
}
