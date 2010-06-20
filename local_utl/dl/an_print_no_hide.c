#include "bbs.h"
#include "ythtbbs.h"
#include "ythtlib.h"

void do_index(char *);

int
main(int n, char *argv[])
{
	if (n < 2)
		exit(0);
	do_index(argv[1]);
	return 0;
}

void
do_index(char *path)
{
	FILE *fp;
	int m;
	unsigned char names[512], title[256];
	unsigned char genbuf[512], path00[512];
	if (!lfile_isdir(path))
		return;
	sprintf(names, "%s/.Names", path);
	fp = fopen(names, "r");
	if (fp == NULL)
		return;
	printf("%s\n", names);
	while (fgets(genbuf, 80, fp) > 0) {
		if (strncmp(genbuf, "Name=", 5))
			continue;
		sprintf(title, "%s", genbuf + 5);
		fgets(genbuf, 256, fp);
		if (strncmp("Path=~/", genbuf, 6))
			continue;
		for (m = 0; m < strlen(genbuf); m++)
			if (genbuf[m] <= 27)
				genbuf[m] = 0;
		if (!strcmp("Path=~/", genbuf))
			continue;
		sprintf(path00, "%s/%s", path, genbuf + 7);
		for (m = 0; m < strlen(path00); m++)
			if (path00[m] <= 27)
				path00[m] = 0;
		if (!file_exist(path00))
			continue;
		if (strstr(title, "BMS")
		    || strstr(title, "SYSOPS")
		    || strstr(title, "<HIDE>"))
			continue;
		if (lfile_islnk(path00))
			continue;
		if (lfile_isdir(path00)) {
			do_index(path00);
			continue;
		}
		printf("%s\n", path00);
	}
	fclose(fp);
}
