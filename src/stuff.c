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
    Copyright (C) 1999	KCN,Zhou lin,kcn@cic.tsinghua.edu.cn
    
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

extern char fromhost[];
extern int convcode;

#ifdef CAN_EXEC
char tempfile[MAXPATHLEN];
#endif

extern void output(char *s, int len);

int
dashf(fname)
const char *fname;
{
	struct stat st;

	return (stat(fname, &st) == 0 && S_ISREG(st.st_mode));
}

int
dashd(fname)
char *fname;
{
	struct stat st;

	return (stat(fname, &st) == 0 && S_ISDIR(st.st_mode));
}

int
dashl(fname)
char *fname;
{
	struct stat st;

	return (lstat(fname, &st) == 0 && S_ISLNK(st.st_mode));
}

int
pressanykey()
{
	extern int showansi;

	showansi = 1;
	move(t_lines - 1, 0);
	clrtoeol();
	prints
	    ("\033[m                                \033[5;1;33m按任何键继续...\033[m");
	egetch();
	move(t_lines - 1, 0);
	clrtoeol();
	return 0;
}

int
pressreturn()
{
	extern int showansi;
	char buf[3];

	showansi = 1;
	move(t_lines - 1, 0);
	clrtoeol();
	getdata(t_lines - 1, 0,
		"                              \033[1;33m请按 ◆\033[5;36mEnter\033[m\033[1;33m◆ 继续\033[m",
		buf, 2, NOECHO, YEA);
	move(t_lines - 1, 0);
	clrtoeol();
	return 0;
}

int
askyn(str, defa, gobottom)
char str[STRLEN];
int defa, gobottom;
{
	int x, y;
	char realstr[280];
	char ans;

	snprintf(realstr, sizeof (realstr), "%s (Y/N)? [%c]: ", str,
		 (defa) ? 'Y' : 'N');
	if (gobottom)
		move(t_lines - 1, 0);
	getyx(&x, &y);
	clrtoeol();
	ans = askone(x, y, realstr, "YN", (defa) ? 'Y' : 'N');
	outc('\n');
	if (ans == 'Y')
		return 1;
	else
		return 0;
}

void
printdash(mesg)
char *mesg;
{
	char buf[80], *ptr;
	int len;

	memset(buf, '=', 79);
	buf[79] = '\0';
	if (mesg != NULL) {
		len = strlen(mesg);
		if (len > 76)
			len = 76;
		ptr = &buf[40 - len / 2];
		ptr[-1] = ' ';
		ptr[len] = ' ';
		strncpy(ptr, mesg, len);
	}
	prints("%s\n", buf);
}

void
bell()
{
	char sound;
	if (USERDEFINE(currentuser, DEF_SOUNDMSG)) {
		sound = Ctrl('G');
		output(&sound, 1);
	}
}

/* rrr - Snagged from pbbs 1.8 */

#define LOOKFIRST  (0)
#define LOOKLAST   (1)
#define QUOTEMODE  (2)
#define MAXCOMSZ (1024)
#define MAXARGS (40)
#define MAXENVS (20)
#define BINDIR "/bin/"

char *bbsenv[MAXENVS];
int numbbsenvs = 0;

void
strtolower(dst, src)
char *dst, *src;
{
	for (; *src; src++)
		*dst++ = tolower(*src);
	*dst = '\0';
}

