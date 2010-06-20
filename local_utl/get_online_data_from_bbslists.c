#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include "bbs.h"

static void
get_info(char *filename)
{
	FILE *fp;
	char buf[512], date[20], *ptr;
	int i, max = 0, ave = 0;
	fp = fopen(filename, "r");
	if (NULL == fp)
		return;
	fgets(buf, sizeof (buf), fp);
	fgets(buf, sizeof (buf), fp);
	if (!strstr(buf, "本站的平均上站人数图")) {
		fclose(fp);
		return;
	}
	while (fgets(buf, sizeof (buf), fp)) {
		if ((ptr = strstr(buf, "平均上站人数：\033[37m"))) {
			ave = atoi(ptr + 19);
			break;
		}
		if ((ptr = strstr(buf, "平均负载人数统计"))) {
			memcpy(date, ptr + 41, 6);
			continue;
		}
		ptr = buf;
		while ((ptr = strstr(ptr, "\033[35m"))) {
			ptr += 5;
			i = atoi(ptr);
			if (i > max)
				max = i;
		}
	}
	fclose(fp);
	if (date[4] == ' ')
		date[4] = '0';
	date[6] = 0;
	printf("%d\t\t%d\t\t%s 2005\n", max, ave, date);
}

int
main(int argc, char *argv[])
{
	struct fileheader h;
	FILE *fp;
	char buf[256];
	fp = fopen("boards/bbslists/.DIR", "r");
	if (NULL == fp)
		return -1;
	while (fread(&h, sizeof (h), 1, fp)) {
		snprintf(buf, sizeof (buf), "boards/bbslists/%s", fh2fname(&h));
		get_info(buf);
	}
	fclose(fp);
	return 0;
}
