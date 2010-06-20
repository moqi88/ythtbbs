#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ght_hash_table.h>
#include <signal.h>
#include "bbs.h"

int
initBlogLogMSQ()
{
	int msqid;
	struct msqid_ds buf;
	msqid = msgget(getBBSKey(BLOGLOG_MSQ), IPC_CREAT | 0664);
	if (msqid < 0)
		return -1;
	msgctl(msqid, IPC_STAT, &buf);
	buf.msg_qbytes = 1024 * 512;
	msgctl(msqid, IPC_SET, &buf);
	return msqid;
}

char *
rcvlog(int msqid, int nowait)
{
	static char buf[1024];
	struct mymsgbuf *msgp = (struct mymsgbuf *) buf;
	int retv;
	retv =
	    msgrcv(msqid, msgp, sizeof (buf) - sizeof (msgp->mtype) - 2, 0,
		   (nowait ? IPC_NOWAIT : 0) | MSG_NOERROR);
	while (retv > 0 && msgp->mtext[retv - 1] == 0)
		retv--;
	if (retv <= 0)
		return NULL;
	msgp->mtext[retv] = 0;
	return msgp->mtext;
}

char *
getfilename(int *day)
{
	static char logf[256];
	time_t dtime;
	struct tm *n;
	time(&dtime);
	n = localtime(&dtime);
	sprintf(logf, MY_BBS_HOME "/newtrace/%d-%02d-%02d.blog",
		1900 + n->tm_year, 1 + n->tm_mon, n->tm_mday);
	*day = n->tm_mday;
	return logf;
}


struct BlogEvent {
	char userid[IDLEN+1];
	char nouse[3];
	ght_hash_table_t *access;
};


struct AccessEvent {
	char userid[30];
	short count;
};
	

ght_hash_table_t *betable = NULL;

int
dumptable()
{
	
	struct BlogEvent *be;
	struct AccessEvent *ae;
	ght_iterator_t it1, it2;
	void *key1, *key2;
	int fd, n, count;
	char buf[64];

	fd = open(BLOG_DIR_PATH "/access.log", O_CREAT | O_TRUNC | O_WRONLY, 0660);
	if(fd < 0)
		return -1;
	for (be=ght_first(betable, &it1, &key1); be; be = ght_next(betable, &it1, &key1)) {
		count = 0;
		for (ae=ght_first(be->access, &it2, &key2); ae; ae = ght_next(be->access, &it2, &key2)) {
			count++;
			ght_remove(be->access, strlen(ae->userid), ae->userid);
			free(ae);
		}
		if(count > 0) {
			n = sprintf(buf, "%s %d\n", be->userid, count);
			write(fd, buf, n);
		}
	}
	close(fd);
	return 0;
}


int
filterlog(char *eventstr)
{
	char blogid[IDLEN+1], eventid[30];
	char *ptr;
	struct BlogEvent *be;
	struct AccessEvent *ae;

	ptr = strchr(eventstr, ' '); //t
	if(ptr == NULL)
		return 1;
	
	eventstr = ptr+1;
	ptr = strchr(eventstr, ' ');	//action
	if(ptr == NULL)
		return 1;
	*ptr = 0;
	if(strcasecmp(eventstr, "READ"))
		return 1;
	
	eventstr = ptr+1;
	ptr = strchr(eventstr, ' ');	//blogid
	if(ptr == NULL)
		return 1;
	*ptr = '\0';
	strsncpy(blogid, eventstr, sizeof(blogid));
	
	eventstr = ptr+1;
	ptr = strchr(eventstr, ' ');	//item
	if(ptr == NULL)
		return 1;

	eventstr = ptr+1;
	ptr = strchr(eventstr, '\n');	//eventid(ip)
	if(ptr != NULL)
		*ptr = '\0';
	strsncpy(eventid, eventstr, sizeof(eventid));
	
	be = ght_get(betable, strlen(blogid), blogid);
	if(be == NULL) {
		be = calloc(1, sizeof(struct BlogEvent));
		if (be == NULL)
			return 1;
		strsncpy(be->userid, blogid, sizeof(be->userid));
		be->access = ght_create(251);
		if(be->access == NULL) {
			free(be);
			return 1;
		}
		if (ght_insert(betable, be, strlen(be->userid), be->userid) < 0) {
			ght_finalize(be->access);
			free(be);
			return 1;
		}
	}

	ae = ght_get(be->access, strlen(eventid), eventid);
	if(ae == NULL) {
		ae = malloc(sizeof(struct AccessEvent));
		if(ae == NULL)
			return 1;
		strsncpy(ae->userid, eventid, sizeof(ae->userid));
		ae->count = 1;
		if (ght_insert(be->access, ae, strlen(ae->userid), ae->userid) < 0) {
			free(ae);
			return 1;
		}
	} else {
		ae->count++;
	}
	return 0;
}

int sync_flag = 0;
int fd = -1;
char buf[100 * 1024];
int n = 0;

static void
set_sync_flag(int signno)
{
	sync_flag = 1;
}

static void
write_back_all(int signno)
{
	if (fd >= 0)
		write(fd, buf, n);
	dumptable();
	exit(0);
}

int
main()
{
	char *str;
	int len, msqid, i;
	int lastday, day;

	umask(027);

	msqid = initBlogLogMSQ();
	if (msqid < 0)
		return -1;

	if (fork())
		return 0;
	setsid();
	if (fork())
		return 0;
		
	close(0);
	close(1);
	close(2);

	betable = ght_create(4093);
	if (betable == NULL)
		return -1;

	fd = open(MY_BBS_HOME "/reclog/bloglogd.lock", O_CREAT | O_RDONLY,
		  0660);
	if (flock(fd, LOCK_EX | LOCK_NB) < 0)
		return -1;

	signal(SIGHUP, set_sync_flag);
	signal(SIGTERM, write_back_all);
	fd = open(getfilename(&lastday), O_WRONLY | O_CREAT | O_APPEND, 0660);
	i = 0;
	while (1) {
		while ((str = rcvlog(msqid, 1))) {
			getfilename(&day);
			if (day != lastday) {
				if (fd >= 0) {
					write(fd, buf, n);
					n = 0;
					i = 0;
					close(fd);
				}
				fd = open(getfilename(&lastday),
					  O_WRONLY | O_CREAT | O_APPEND, 0660);
				dumptable();
			}
			len = strlen(str);
			if (n + len > sizeof (buf)) { // 如果缓冲区不够了, 写回
				write(fd, buf, n);
				sync_flag = 0;
				n = 0;
				i = 0;
			}
			memcpy(buf + n, str, len); // 直接加到缓冲区
			n += len;
			
			filterlog(str); //str值被改变，放在最后用!!
		}
		sleep(5);
		i++;
		if (i < 30 && !sync_flag) // 至少每两分钟写回一次
			continue;
		write(fd, buf, n);
		n = 0;
		i = 0;
		sync_flag = 0;
	}
}
