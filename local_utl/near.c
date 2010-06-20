#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "bbs.h"
#define MAXBOARD 1024
#define NAVFILE "nav.txt"

struct fileeva
{
	int cl;
	float avg;
	int count;
	char bn[30];
	int filenum;
	float neweva;  // new evaluation depends on file time and cl
};

static int
cmp_neweva(const void *a, const void *b)
{
	return ((struct fileeva *) b)->neweva -
	    ((struct fileeva *) a)->neweva;
}

int
boardcount(char *bn)
{
	int i;
	static char bname[MAXBOARD][20];
	static int count[MAXBOARD];
	static int nboard = 0;
	
	for (i = 0; i < nboard; i++) {
		if (!strcmp(bname[i], bn)) {
			return count[i]++;
		}
	}
	if (nboard >= MAXBOARD)
		return 0;
	strncpy(bname[nboard], bn, 19);
	bname[nboard][19] = 0;
	count[nboard] = 1;
	nboard++;
	return 0;
}

int
main(int argn, char **argv)
{
	FILE *fp;
	char buf[1024], *ptr, bn[30], s[30];
	int i, n = time(NULL), maxntr = 4, count = 0, maxcount = 100, maxoneboard = 5;
	float d = 2;
	int lineoffile = 0; 
	struct fileeva *evafile, *filept;
	char s2[30], s3[30];
	int line;

	if (argn >= 2)
		d = atof(argv[1]);
	if (d <= 0)
		d = 1;

	if (argn >= 3)
		maxcount = atoi(argv[2]);

	if (argn >= 4)
		maxntr = atoi(argv[3]);

	if (argn >= 5)
		maxoneboard = atoi(argv[4]);

    //首先读取文件的行数

	fp = fopen(NAVFILE, "r");
	if(!fp) {
		printf("can not find NAVFILE\n");
		return -1;
	}
	fgets(buf, 1024, fp);
	fputs(buf, stdout);

	while (fgets(buf, 1024, fp) != NULL) {
		lineoffile++;
	}
	fclose(fp);
	
	fp = fopen(NAVFILE, "r");
	if(!fp) {
		printf("can not find NAVFILE\n");
		return -1;
	}

	if (lineoffile<1)
	{
		printf("Error file line=0\n");
		return 0;
	}
	//然后对文件按cl和时间排序，排序字段new_eva定为 eva*168/(17+距发表的hours)
	evafile=(struct fileeva *)calloc(lineoffile, sizeof(struct fileeva));
	filept=&evafile[0];
	fgets(buf,1024,fp);

	while (fgets(buf, 1024, fp) != NULL)
	{
		ptr = strrchr(buf, 'M');
		if (ptr == NULL)
			ptr = strrchr(buf, 'G');
		if (ptr == NULL) {
			//printf("NULL! %s\n", buf);
			continue;
		}

		ptr += 2;
//		printf("ptr=%s\n",ptr);
		i = atoi(ptr);
		if (n - i > d * 24 * 3600)
			continue;
//		printf("i=%d\n",i);
		filept->filenum = i;
		sscanf(buf, "%s%s%s%s", s, s2, s3, bn);
		filept->cl = atoi(s);
		filept->avg = atof(s2);
		filept->count = atoi(s3);
		strcpy(filept->bn , bn);
		filept->neweva = filept->cl * 72 / ( 7 + ( n - i ) / 3600.0 );

//		printf("we got it\n");
//		printf("%d\t%f\t%d\t%s\tM.%d.A\n",filept->cl, filept->avg, filept->count, filept->bn, filept->filenum);
		filept++;

	}
	fclose(fp);
	//对evafile按neweva排序

	qsort(evafile, lineoffile, sizeof( struct fileeva), cmp_neweva);

	filept=&evafile[0];
    //最后输出文件
	for (line = 0; line < lineoffile; line++ )
	{
//	printf("outputfile\n");
//	printf("%d\t%f\t%d\t%s\tM.%d.A\n",filept->cl, filept->avg, filept->count, filept->bn, filept->filenum);
		if (boardcount(filept->bn) >= maxoneboard) {
			filept++;
			continue;
		}
		if (filept->cl == 0)
			break;
		printf("%d\t%f\t%d\t%s\tM.%d.A\n",filept->cl, filept->avg, filept->count, filept->bn, filept->filenum);
		filept++;
		count++;
		if (count > maxcount)
			break;
	}

	free(evafile);

	return 0;


/*
	while (fgets(buf, 1024, stdin) != NULL) {
		ptr = strrchr(buf, 'M');
		if (ptr == NULL)
			ptr = strrchr(buf, 'G');
		if (ptr == NULL) {
			//printf("NULL! %s\n", buf);
			continue;
		}
		ptr += 2;
		i = atoi(ptr);
		if (n - i > d * 24 * 3600)
			continue;
		sscanf(buf, "%s%s%s%s", s, s, s, bn);
#if 0
		if (!strcmp(bn, "triangle") || !strcmp(bn, "TaiWan")
		    || !strcmp(bn, "civic_life"))
			continue;
#endif
// triangle will not be special abroad
#if 0
		if (!strcmp(bn, "triangle")) {
			if (n - i > 2 * 24 * 3600)
				continue;
			ntr++;
			if (ntr > maxntr)
				continue;
		} else //else if (boardcount(bn) >=....)
#endif 
		if (boardcount(bn) >= maxoneboard) {
			continue;
		}
		fputs(buf, stdout);
		count++;
		if (count > maxcount)
			break;
	}
	*/
}
