#include "bbs.h"
#include <stdio.h>

int
main(int argc, char *argv[])
{
	FILE *fp;
	struct fileheader a;
	if (argc != 2) {
		printf("enter .DIR name!\n");
		return -1;
	}
	fp = fopen(argv[1], "r");
	if (NULL == fp) {
		printf("can't open .DIR file\n");
		return -2;
	}
	while (fread(&a, sizeof (a), 1, fp)) {
		printf("%s %s %s\n", fh2fname(&a), fh2owner(&a), a.title);
	}
	fclose(fp);
	return 0;
}
