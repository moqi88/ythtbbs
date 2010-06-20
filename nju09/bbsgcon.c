#include "bbslib.h"
int
bbsgcon_main()
{
	FILE *fp;
	char board[80], dir[80], file[80], filename[80], *ptr;
	struct fileheader x;
	int num, total = 0;
	struct fileheader *dirinfo = NULL;
	struct mmapfile mf = { ptr:NULL };
	struct boardmem *brd;
	changemode(READING);
	getparmboard(board, sizeof(board));
	strsncpy(file, getparm2("F", "file"), 32);
	num = atoi(getparm("num")) - 1;
	if ((brd=getboard(board)) == NULL)
		http_fatal("错误的讨论区");
	if (strncmp(file, "M.", 2) && strncmp(file, "G.", 2))
		http_fatal("错误的参数1");
	if (strstr(file, "..") || strstr(file, "/"))
		http_fatal("错误的参数2");
	sprintf(dir, "boards/%s/.DIGEST", board);
	sprintf(filename, "boards/%s/%s", board, file);
	MMAP_TRY {
		if (mmapfile(dir, &mf) == -1) {
			MMAP_UNTRY;
			http_fatal("此讨论区不存在或者为空");
		}
		total = mf.size / sizeof (struct fileheader);
		if (total == 0) {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_fatal("此讨论区不存在或者为空");
		}
		if (num < 0)
			num = total - 1;
		num++;
		dirinfo = findbarticle(&mf, file, &num, 0);
	}
	MMAP_CATCH {
		dirinfo = NULL;
	}
	MMAP_END mmapfile(NULL, &mf);
	if (NULL == dirinfo)
		http_fatal("文章不存在或已被删除");
	if (*getparm("attachname") == '/') {
		showbinaryattach(filename);
		return 0;
	}
	html_header(1);
	//check_msg();
	changemode(READING);
	printf("<body>\n");
	printf("%s -- 文章阅读 [讨论区: %s]<hr>", BBSNAME, board);
	showcon(filename);
	printf("[<a href=bbsboa>分类讨论区</a>]");
	printf("[<a href=bbsall>全部讨论区</a>]");
	fp = fopen(dir, "r");
	if (fp == 0)
		http_fatal("dir error2");
	if (num > 0) {
		fseek(fp, sizeof (x) * (num - 1), SEEK_SET);
		fread(&x, sizeof (x), 1, fp);
		printf("[<a href=bbsgcon?B=%d&file=%s&num=%d>上一篇</a>]",
		       getbnumx(brd), fh2fname(&x), num - 1);
	}
	printf("[<a href=doc?B=%d>本讨论区</a>]", getbnumx(brd));
	if (num < total - 1) {
		fseek(fp, sizeof (x) * (num + 1), SEEK_SET);
		fread(&x, sizeof (x), 1, fp);
		printf("[<a href=bbsgcon?B=%d&file=%s&num=%d>下一篇</a>]",
		       getbnumx(brd), fh2fname(&x), num + 1);
	}
	fclose(fp);
	ptr = dirinfo->title;
	if (!strncmp(ptr, "Re: ", 4))
		ptr += 4;
	printf("[<a href='bbstfind?B=%d&th=%d'>同主题阅读</a>]\n",
	       getbnumx(brd), dirinfo->thread);
	showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
	printf("</body>\n");
	http_quit();
	return 0;
}
