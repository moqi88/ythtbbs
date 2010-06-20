#include "bbslib.h"

int
bbsnot_main()
{
	FILE *fp;
	char buf[512], board[80], filename[80];
	struct boardmem *x;
	html_header(1);
	//check_msg();
	changemode(READING);
	getparmboard(board, sizeof(board));
	if (!(x = getboard(board)))
		http_fatal("错误的版面");
	printf("<center>\n");
	printf("%s -- 备忘录 [讨论区: %s]<hr>\n", BBSNAME, board);
	sprintf(filename, "vote/%s/notes", board);
	fp = fopen(filename, "r");
	if (fp == 0) {
		printf("<br>本讨论区尚无「备忘录」。\n");
		http_quit();
	}
	printf("<table border=1><tr><td class=f2><tt>\n");
	while (1) {
		char *s;
		bzero(buf, 512);
		if (fgets(buf, 512, fp) == 0)
			break;
		while (1) {
			int i;
			s = strstr(buf, "$userid");
			if (s == 0)
				break;
			for (i = 0; i < 7; i++)
				s[i] = 32;
			for (i = 0; i < strlen(currentuser->userid); i++)
				s[i] = currentuser->userid[i];
		}
		fhhprintf(stdout, "%s", buf);
	}
	fclose(fp);
	printf("</tt></td></tr></table><hr>\n");
	printf("[<a href=doc?B=%d>本讨论区</a>] ", getbnumx(x));
	if (has_BM_perm(currentuser, x))
		printf("[<a href=bbsmnote?B=%d>编辑备忘录</a>]", getbnumx(x));
	printf("</center>\n");
	http_quit();
	return 0;
}
