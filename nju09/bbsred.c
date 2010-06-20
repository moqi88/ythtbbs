#include "bbslib.h"

static char *(replacestr[][2]) = { {
	"pku", "boa?secstr=DP"}, {
#ifdef ENABLE_BLOG
	"blog", "blogpage"}, {
	"b", "blogpage"}, {
#endif
	"p", "home?B=Personal_Corpus"}, {
	NULL, NULL}};
static char buf[256];

#ifdef ENABLE_BLOG
int 
bbsredblog(char *command) {
	struct userec *x;
	char userid[IDLEN + 1];
	if (isdigit(*command))
		strsncpy(userid, getuseridbynum(atoi(command)), IDLEN + 1);
	else
		strsncpy(userid, command, IDLEN + 1);
	if (getuser(userid, &x) < 1 || !x->hasblog)
		return 0;
	snprintf(buf, sizeof(buf), "blog?U=%s", command);
	return 1;
}
#endif

int
bbsredpc(char *command) {
	struct userec *x;
	char userid[IDLEN + 1];
	char path[256];
	if (isdigit(*command))
		strsncpy(userid, getuseridbynum(atoi(command)), IDLEN + 1);
	else
		strsncpy(userid, command, IDLEN + 1);
	if (getuser(userid, &x) < 1)
		return 0;
	snprintf(path, sizeof(path), 
			MY_BBS_HOME "/0Announce/groups/"
			"GROUP_0/Personal_Corpus/%c/%s", 
			mytoupper(x->userid[0]), x->userid);
	if (file_exist(path)) {
		//there is still something wrong with chinese id's
		//since we cannot get it's pinyin by userid directly
		//maybe some other tools could help us.
		snprintf(buf, sizeof(buf), "bbs0an?path=%s", 
				path + strlen(MY_BBS_HOME) + 10);
		return 1;
	}
	return 0;
}

char *
bbsred(char *command)
{
	char *b;
	int clen;
	int i;
	int type = 0 ; // 0 for normal; 1 for blog; 2 for personal corpus
	char *ptr;
	b = getparm("b");
	bzero(buf, sizeof(buf));
	if(b[0]) {
		strsncpy(buf, getsenv("QUERY_STRING"), sizeof(buf));
		ptr = strstr(buf, "b=");
		if (!ptr)
			return b;
		else
			return ptr + 2;
	}
#ifdef ENABLE_BLOG
	if ((ptr = strstr(command, ".blog"))) {
		*ptr = 0;
		type = 1;
	} else if ((ptr = strstr(command, ".p"))) {
#else
	if ((ptr = strstr(command, ".p"))) {
#endif
		*ptr = 0;
		type = 2;
	}
	clen = strlen(command);
	if (!clen || isaword(specname, command)) {
		/*struct brcinfo *brcinfo = brc_readinfo(currentuser->userid);
		   sprintf(buf, "bbsboa?secstr=%c", brcinfo->lastsec[0]);
		   return buf; */
		return "bbsboa?secstr=?";
	}
	for (i = 0; replacestr[i][0]; i++) {
		if (!strcasecmp(command, replacestr[i][0]))
			return replacestr[i][1];
	}
	if (clen <= IDLEN) {
#ifdef ENABLE_BLOG
		if (type == 1) {
			if (bbsredblog(command))
				return buf;
			return "blogpage";
		} else if (type == 2) {
#else
		if (type == 2) {
#endif
			if (bbsredpc(command))
				return buf;
			return "home?B=Personal_Corpus";
		}
	}
	if (getboard2(command)) {
		snprintf(buf, 256, "bbshome?B=%d", getbnum(command));
	} else {
#ifdef ENABLE_BLOG
		if (bbsredblog(command))
			return buf;
#endif
		if (bbsredpc(command))
			return buf;
		strcpy(buf, "bbsboa?secstr=");
	}
	return buf;
}
