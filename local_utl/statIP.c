#include "bbs.h"
#include <arpa/inet.h>

struct ipitem {
	unsigned long ip;
	struct ipitem *next;
};

struct ipitemhead {
	int total;
	int edu;
	struct ipitem *start;
} currip;

struct iprange {
	unsigned long start;
	unsigned long end;
	struct iprange *next;
};

struct iprangehead {
	struct iprange *start;
} charip;

struct bbsinfo info;

static void invertl(char *lp) {
	char ch;
	ch = lp[0];
	lp[0] = lp[3];
	lp[3] = ch;
	ch = lp[1];
	lp[1] = lp[2];
	lp[2] = ch;
	return;
}

int validip(unsigned long ip, struct iprangehead *charip) {
	struct iprange *piprange = charip->start;

	while (piprange) {
		if (ip < piprange->start)
			return 0;
		if (ip > piprange->end) {
			piprange = piprange->next;
			continue;
		}
		return 1;
	}
	return 0;
}

int insert_ipitem(struct ipitemhead *currip, unsigned long fromIP, 
		struct iprangehead *charip) {
	struct ipitem *tmpitem = currip->start, *pipitem;
	unsigned long tmpIP = fromIP;

	invertl((char *) &tmpIP);
	pipitem = (struct ipitem *)malloc(sizeof(struct ipitem));
	if (!pipitem) {
		printf("Memory errors.\n");
		return -1;
	}
	pipitem->ip = tmpIP;

	if (NULL == tmpitem) {
		currip->start = pipitem;
		pipitem->next = NULL;
		if (validip(tmpIP, charip))
			currip->edu++;
		currip->total++;
		return 1;
	}

	if (tmpitem->ip > tmpIP) {
		currip->start = pipitem;
		pipitem->next = tmpitem;
		if (validip(tmpIP, charip))
			currip->edu++;
		currip->total++;
		return 1;
	} else if (NULL == tmpitem->next) {
		tmpitem->next = pipitem;
		pipitem->next = NULL;
		if (validip(tmpIP, charip))
			currip->edu++;
		currip->total++;
		return 1;
	}

	while (tmpitem->next != NULL && tmpitem->next->ip < tmpIP)
		tmpitem = tmpitem->next;
	if (tmpitem->next == NULL) {
		tmpitem->next = pipitem;
		pipitem->next = NULL;
		if (validip(tmpIP, charip))
			currip->edu++;
		currip->total++;
		//printf("%lu\t%d\n", tmpIP, validip(tmpIP, charip));
		return 1;
	}
	if (tmpitem->next->ip == tmpIP) {
		free(pipitem);
		return 0;
	}
	pipitem->next = tmpitem->next;
	tmpitem->next = pipitem;
	if (validip(tmpIP, charip))
		currip->edu++;
	currip->total++;
	//printf("%lu\t%d\n", tmpIP, validip(tmpIP, charip));
	return 1;
}

int insert_iprange(struct iprange *piprange, struct iprangehead *charip) {
	struct iprange *curr;
	
	curr = charip->start;

	if (NULL == curr) {
		charip->start = piprange;
		piprange->next = NULL;
		return 1;
	}

	if (curr->start > piprange->start) {
		piprange->next = charip->start;
		charip->start = piprange;
		return 1;
	} else if (curr->next == NULL) {
		curr->next = piprange;
		piprange->next = NULL;
		return 1;
	}

	while (curr->next != NULL && curr->next->start < piprange->start)
		curr = curr->next;
	piprange->next = curr->next;
	curr->next = piprange;
	return 1;
}

int free_iprange(struct iprangehead *charip) {
	struct iprange *tmp, *tmp2;
	
	tmp = charip->start;
	while (tmp != NULL) {
		tmp2 = tmp->next;
		free(tmp);
		tmp = tmp2;
	}
	return 1;
}

int free_ipitem(struct ipitemhead *currip) {
	struct ipitem *tmp, *tmp2;
	tmp = currip->start;
	while (tmp != NULL) {
		tmp2 = tmp->next;
		free(tmp);
		tmp = tmp2;
	}
	return 1;
}

int initiprange(struct iprangehead *charip) {
	FILE *fp;
	char buf[256], *ptr;
	struct iprange *piprange;

	if (NULL == (fp = fopen(MY_BBS_HOME "/wwwtmp/iplist.txt", "r"))) {
		printf("Cannot open " MY_BBS_HOME "/wwwtmp/iplist.txt!\n");
		return -1;
	}

	while (fgets(buf, sizeof(buf), fp)) {
		if (NULL == (ptr = strchr(buf, '\t')))
			continue;
		piprange = (struct iprange *)malloc(sizeof(struct iprange));
		if (!piprange) {
			free_iprange(charip);
			printf("Memory errors.\n");
			return -1;
		}
		*ptr = 0;
		piprange->start = inet_addr(buf);
		piprange->end = inet_addr(ptr + 1);
		invertl((char *) &(piprange->end));
		invertl((char *) &(piprange->start));
		insert_iprange(piprange, charip);
	}
#ifdef DEBUG
	piprange = charip->start;
	while (piprange) {
		printf("%lu\t%lu\n", piprange->start, piprange->end);
		piprange = piprange->next;
	}
#endif
	return 1;
}

int log_ip(struct ipitemhead *currip, int mode) {
	FILE *fp;
	char buf[256], *ptr;
	time_t now = time(NULL) - 24 * 3600;

	strncpy(buf, 
		MY_BBS_HOME "/ftphome/root/boards/bbslists/statip.txt", 
		sizeof(buf));

	if (NULL == (fp = fopen(buf, "a+"))) {
		printf("Cannot open log file.\n");
		return -1;
	}
	fprintf(fp, "%d\t%d\t", currip->edu, currip->total);
	if (mode) {
		strncpy(buf, ctime(&now), sizeof(buf));
		buf[3] = 0;
		if ((ptr = strchr(buf + 4, ' '))) {
			*ptr++ = 0;
			while (*ptr == ' ')
				ptr++;
			if (ptr)
				fprintf(fp, "%s %02d %s", 
					buf + 4, atoi(ptr), buf + 20);
		}
	}
	fclose(fp);
	return 1;
}

int stat_current() {
	struct user_info *p;
	int i;
	
	bzero(&currip, sizeof(currip));
	bzero(&charip, sizeof(charip));
	if (initbbsinfo(&info) < 0) {
		printf("Cannot init bbsinfo.\n");
		return -1;
	}

	if (initiprange(&charip) < 0) {
		return -1;
	}
	for (i = 0; i < USHM_SIZE; i++) {
		p = &info.utmpshm->uinfo[i];
		if (!p->active)
			continue;
		insert_ipitem(&currip, p->fromIP, &charip);
	}
#ifdef DEBUG
	while (1) {
		struct ipitem *pipitem = currip.start;
		struct in_addr in;
		while (pipitem) {
			in.s_addr = pipitem->ip;
			invertl((char *) &(in.s_addr));
			printf("[%lu][%s][%d]\n", 
				pipitem->ip, inet_ntoa(in), 
				validip(pipitem->ip, &charip));
			pipitem = pipitem->next;
		}
		return 0;
	}
#endif
	log_ip(&currip, 0);
	free_iprange(&charip);
	free_ipitem(&currip);
	return 1;
}

int stat_yesterday() {
	time_t now = time(NULL) - 3600 * 24;
	struct tm *yesterday = localtime(&now);
	char buf[256], *ptr;
	FILE *fp;
	unsigned long ip;

	bzero(&charip, sizeof(charip));
	bzero(&currip, sizeof(currip));
	
	if (initiprange(&charip) < 0) {
		return -1;
	}
	
	sprintf(buf, MY_BBS_HOME "/newtrace/%d-%02d-%02d.log", 
			yesterday->tm_year + 1900, 
			yesterday->tm_mon + 1, yesterday->tm_mday);
	if (NULL == (fp = fopen(buf, "r"))) {
		printf("Cannot open tracelog file (%s).\n", buf);
		return -1;
	}

	while(fgets(buf, sizeof(buf), fp)) {
		if (!(ptr = strchr(buf + 9, ' ')))
			continue;
		if (strncmp(ptr + 1, "enter ", 6))
			continue;
		ptr += 7;
		if (ptr) {
			if (strchr(ptr, ' '))
				*strchr(ptr, ' ') = 0;
			ip = inet_addr(ptr);
			if (ip > 0)
				insert_ipitem(&currip, ip, &charip);
		}
	}
#ifdef DEBUG
	fclose(fp);
	while (1){
		struct ipitem *pipitem = currip.start;
		struct in_addr in;
		printf("stat_yesterday:\n");
		while (pipitem) {
			in.s_addr = pipitem->ip;
			invertl((char *) (&in.s_addr));
			printf("[%lu][%s][%d]\n", pipitem->ip, 
				inet_ntoa(in), validip(pipitem->ip, &charip));
			pipitem = pipitem->next;
		}
		free_ipitem(&currip);
		free_iprange(&charip);
		return 1;
	}
#endif
	log_ip(&currip, 1);
	free_ipitem(&currip);
	free_iprange(&charip);
	fclose(fp);

	return 1;
}

void showhelp() {
	printf("statIP 0 : Analysis current online users.\n"
		"statIP 1 : Analysis yesterday users.\n");
}


int main(int argc, char *argv[]) {
	int mode;
	if (argc < 2) {
		showhelp();
		return -1;
	}
	mode = atoi(argv[1]);
	switch(mode) {
		case 0:
			stat_current();
			break;
		case 1:
			stat_yesterday();
			break;
		default:
			showhelp();
			return -2;
	}
	return 0;
}
