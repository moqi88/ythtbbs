#include "bbs.h"

int cmpboard(struct boardheader *a,struct boardheader *b) {
	if (b->sec1[0] != a->sec1[0])
		return (a->sec1[0] - b->sec1[0]);
	return (b->board_ctime - a->board_ctime);
}

int printdetail(struct boardheader *bh) {
	char buf[] = "-------";
	if (!*(bh->filename))
		return 0;
	printf("%-28s", bh->filename);
	if (bh->flag & VOTE_FLAG)
		buf[0] = 'V';
	if (bh->flag & NOZAP_FLAG)
		buf[1] = 'Z';
	if (bh->flag & CLUB_FLAG) {
		if (bh->flag & CLOSECLUB_FLAG && bh->flag & CLUBLEVEL_FLAG)
			buf[2] = 'C';
		else if (bh->flag & CLOSECLUB_FLAG)
			buf[2] = 'c';
		else
			buf[2] = 'o';
	}
	if (bh->level) {
		if (bh->level & PERM_POSTMASK)
			buf[2] = 'P';
		else
			buf[2] = 'R';
	}
	if (bh->flag & ANONY_FLAG)
		buf[2] = 'A';
	if (bh->flag & INNBBSD_FLAG)
		buf[3] = 'I';
	else if (bh->flag2 & NJUINN_FLAG)
		buf[3] = 'N';
	if (bh->flag & IS1984_FLAG)
		buf[4] = '1';
	if (bh->flag & POLITICAL_FLAG)
		buf[5] = 'P';
	if (bh->flag2 & WATCH_FLAG)
		buf[6] = 'W';
	printf("%s    ", buf);
	if (bh->limitchar)
		printf("%d", bh->limitchar * 100);
	printf("\n");
	return 1;
}

int main() {
	struct mmapfile mf = { ptr:NULL };
	struct boardheader *ptr, *bh;
	int size, i;
	char sec;

	chdir(MY_BBS_HOME);
	if (mmapfile(".BOARDS", &mf) < 0) {
		printf("Cannot find .BOARDS.\n");
		return -1;
	}
	size = mf.size;
	ptr = malloc(size);
	memcpy(ptr, mf.ptr, size);
	mmapfile(NULL, &mf);
	qsort(ptr, size / sizeof(struct boardheader), 
			sizeof(struct boardheader), (void *)cmpboard);
	size /= sizeof(struct boardheader);
	sec = 0;
	bh = ptr;
	printf("说明：版面特殊属性一共七位，减号表示不具有该属性。\n"
			"第一位：\tV\t投票中。\n"
			"第二位：\tZ\t不能ZAP。\n"
			"第三位：\tC\t隐藏俱乐部。\n"
			"\t\tc\t封闭俱乐部。\n"
			"\t\to\t开放俱乐部。\n"
			"\t\tA\t匿名版面。\n"
			"\t\tP\t写限制版面。\n"
			"\t\tR\t读限制版面。\n"
			"第四位：\tI\tcn.bbs转信。\n"
			"\t\tN\t点对点转信。\n"
			"第五位：\t1\t内容审查。\n"
			"第六位：\tP\t政治相关版面。\n"
			"第七位：\tW\t定时锁版。\n");
	for (i = 0; i < size && i < MAXBOARD; i++) {
		if (bh->sec1[0] != sec) {
			sec = bh->sec1[0];
			if (!sec)
				return 1;
			printf("\n%c 区特殊版面列表\n"
				"================\n"
				"%-28s%-11s版面上限\n", 
				sec, "版面名称", "版面属性");
		}
		if (bh->flag || bh->flag2 || bh->level || bh->limitchar) {
			printdetail(bh);
		}
		bh++;
	}

	return 1;
}
