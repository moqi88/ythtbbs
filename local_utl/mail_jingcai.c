#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bbs.h"

#define MAXSENT 800

time_t d;
struct tm *t;
int yday;
int sentlength;
char sent[MAXSENT][16];
char lines[120][100];
char buf[256];

int
issent(int thread)
{
	char buf[16];
	int i = 0;
	for(i=0; i<sentlength; i++) {
                strcpy(buf, sent[i]);
                int thisthread = atoi(buf);
                char *p;
                int thisyday;
                p = strchr(buf, ' ');
                if(p == NULL) 
                        continue;
                *(p++) = 0;
                thisyday = atoi(p);
                if(thisthread == thread)
                        return 1;
	}
	sprintf(sent[sentlength], "%d %d\n", thread, yday);
	sentlength++;
	return 0;
}

int 
main(int argc, char **argv)
{
	if(argc < 2) {
		printf("no input Emailaddress!\n");
		exit(1);
	}
	int count;
	FILE *fnav, *fsent;
	fnav = fopen(MY_BBS_HOME "/wwwtmp/navpart.txt", "r");
	if(fnav == NULL)
		return -1;
	count = 0;
	while (count < 120 && fgets(buf, sizeof (buf), fnav) != NULL) {
		buf[sizeof (lines[0]) - 1] = 0;
		memcpy(lines[count], buf, sizeof (lines[0]));
		count++;
	}
	fclose(fnav);

	fsent = fopen(MY_BBS_HOME "/wwwtmp/mail_jingcai.txt", "r");
	if(fsent) {
		sentlength = 0;
		while(count < MAXSENT && fgets(buf, sizeof(buf), fsent) != NULL) {
			buf[sizeof (sent[0]) - 1] = 0;
			memcpy(sent[sentlength], buf, sizeof (sent[0]));
			sentlength++;
		}
		fclose(fsent);
	}

	char target[80], path[80];
	
	strcpy(target, argv[1]);
	d = time(NULL);
	t = localtime(&d);
	yday = t->tm_yday;
	
	int i;
	for(i=0;i<count;i++) {
		memcpy(buf, lines[i], sizeof (lines[0]));
		char *numstr, *board, *author, *title, *ptr;
		int star,thread;
		star = atof(buf) + 0.5;
        	numstr = strchr(buf, ' ');
        	if (numstr == NULL)
                	continue;
        	*(numstr++) = 0;
        	board = strchr(numstr, ' ');
        	if (board == NULL)
                	continue;
       		*(board++) = 0;
        	author = strchr(board, ' ');
        	if (author == NULL)
                	continue;
        	*(author++) = 0;
        	ptr = strchr(author, ' ');
        	if (ptr == NULL)
                	continue;
        	*(ptr++) = 0;
        	thread = atoi(ptr);
        	title = strchr(ptr, ' ');
        	if (title == NULL)
                	continue;
        	*(title++) = 0;
		
		int hassent;
		hassent = issent(thread);
		if(hassent)
			continue;
		sprintf(path, MY_BBS_HOME "/boards/%s/M.%d.A", board, thread);
		int mail;
		mail = bbs_sendmail(path, title, target, "SYSOP", 1);
		if(mail) {
			printf("error %d\n", mail);
		}
	}
	fsent = fopen(MY_BBS_HOME "/wwwtmp/mail_jingcai.txt", "w");
	for(i=0; i<sentlength; i++) {
		char *p;
		int thisyday;
		memcpy(buf, sent[i], sizeof (sent[0]));
		p = strchr(buf, ' ');
		if(p == NULL)
			continue;
		*(p++) = 0;
		thisyday = atoi(p);
		if(fabs(yday - thisyday) < 7)
			fputs(sent[i], fsent);
	}
	fclose(fsent);
	return 0;
}
