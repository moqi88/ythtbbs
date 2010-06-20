#include "bbslib.h"
#ifdef ENABLE_MYSQL
int do_eva(char *board, char *file, int star);
int
bbseva_main()
{
	char board[80], file[80];
	int star;
	html_header(1);
	//check_msg();
	getparmboard(board, sizeof(board));
	strsncpy(file, getparm("F"), 20);
	if (!file[0])
		strsncpy(file, getparm("file"), 32);
	star = atoi(getparm("star"));
	if (!loginok)
		http_fatal("匆匆过客不能进行本项操作");
	changemode(READING);
	if (!getboard(board))
		http_fatal("错误的讨论区");
	if (hideboard(board))
		http_fatal("隐藏版面就不要评价文章啦!");
	if (star < 1 || star > 5)
		http_fatal("错误的评价分数");
//汗，这句是谁加的？这么恶劣
/*	if (star == 1)
		star++;
*/
	printf("<center>%s -- 评价文章 [使用者: %s]<hr>\n", BBSNAME,
	       currentuser->userid);
	printf("<table><td>");
	do_eva(board, file, star);
	printf("</td></table>");
	printf("页面将在三秒后自动返回上一页，如果您的浏览器没有自动返回，请点击[<a href='javascript:history.go(-1)'>这里</a>]。");
	printf("<script>setTimeout(\"history.go(-1);\", 3000);</script>");
	http_quit();
	return 0;
}

char *des[6] = { "没评价", "一般", "不错", "很好", "很好", "我顶!" };

int
set_eva(char *board, char *file, int star, int result[2], char *buf)
{
	int count, starchanged, oldstar;
	float avg;
	starchanged = 0;
//small site 小站点都先留15天评价期，因为这是大部分BBS用户的上站区间，其实永久能评价也行
/*
	if (now_t - atoi(file + 2) > 15 * 86400) {
		sprintf(buf, "这么老的文章就别评价了吧,去评点新文章吧!");
		return 0;
	}
*/
	//oldstar = 0 被认为是未评价状态
	if (bbseva_qset
	    (utmpent, board, file, currentuser->userid, star, &oldstar, &count,
	     &avg) < 0) {
		sprintf(buf, "对文章的评价未能改变");
		return 0;
	}
	if (star == oldstar) {
		sprintf(buf, "您没有改变您对这篇文章的评价");
	} else {
//		if (oldstar != 1 && oldstar != 0)
		if (oldstar != 0)
			sprintf(buf, "您把您对这篇文章的评价从 %s 改到 %s",
				des[oldstar], des[star]);
		else
			sprintf(buf, "这篇文章被您评价为 %s", des[star]);
	}
	if (oldstar != star) {
		starchanged = 1;
		result[0] = (int) (50 * avg);
		result[1] = count > 255 ? 255 : count;
	}
	return starchanged;
}

int
do_eva(char *board, char *file, int star)
{
	FILE *fp;
	char dir[256];
	struct fileheader f;
	struct mmapfile mf = { ptr:NULL };
	int result[2], filetime, total;
	int num = -1;
	filetime = atoi(file + 2);
	sprintf(dir, "boards/%s/.DIR", board);
	MMAP_TRY {
		if (mmapfile(dir, &mf) < 0) {
			MMAP_UNTRY;
			http_fatal("版面错误");
		}
		total = mf.size / sizeof (struct fileheader);
		num = Search_Bin((struct fileheader*)mf.ptr, filetime, 0, total - 1);
		if (num >= 0)
			memcpy(&f, mf.ptr + num * sizeof (struct fileheader),
			       sizeof (struct fileheader));
	}
	MMAP_CATCH {
		num = -1;
	}
	MMAP_END mmapfile(NULL, &mf);
	if (num < 0) {
		printf("<td><td><td>%s<td>文件不存在.\n", file);
		return -1;
	}
	else {	
		fp = fopen(dir, "r+");
		if (fp == 0)
			http_fatal("错误的参数");
		flock(fileno(fp), LOCK_EX);
		fseek(fp, num * sizeof (struct fileheader), SEEK_SET);
		if (set_eva(board, file, star, result, dir)) {
			f.staravg50 = result[0];
			f.hasvoted = result[1];
			fwrite(&f, sizeof (struct fileheader), 1, fp);
		}
		fclose(fp);
		printf("%s", dir);
	}
	return 0;
}
#else
int
bbseva_main()
{
	html_header(1);
	http_fatal("请安装MySQL支持环境!");
	return 0;
}
#endif
