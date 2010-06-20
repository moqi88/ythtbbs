#include <sys/ipc.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "ythtbbs.h"
#include <time.h>
#include <stdlib.h>

void
getrandomint(unsigned int *s)
{
#ifdef LINUX
	int fd;
	fd = open("/dev/urandom", O_RDONLY);
	read(fd, s, 4);
	close(fd);
#else
	srandom(getpid() - 19751016);
	*s = random();
#endif
}

void
getrandomstr(unsigned char *s)
{
	int i;
#ifdef LINUX
	int fd;
	fd = open("/dev/urandom", O_RDONLY);
	read(fd, s, 30);
	close(fd);
	for (i = 0; i < 30; i++)
		s[i] = 65 + s[i] % 26;
#else
	time_t now_t;
	now_t = time(NULL);
	srandom(now_t - 19751016);
	for (i = 0; i < 30; i++)
		s[i] = 65 + random() % 26;
#endif
	s[30] = 0;
}

void
getrandomval(unsigned char *s, size_t size)
{
#ifdef LINUX
	int fd;
	fd = open("/dev/urandom", O_RDONLY);
	read(fd, s, size);
	close(fd);
#else
	int i;
	time_t now_t;
	now_t = time(NULL);
	srandom(now_t - 19751016);
#endif
	s[size] = 0;
}

int
initmsq(key_t key)
{
	int msqid;
	msqid = msgget(getBBSKey(key), IPC_CREAT | 0664);
	if (msqid < 0)
		return -1;
	return msqid;
}

void
newtrace(s)
char *s;
{
	static int disable = 0;
	static int msqid = -1;
	time_t dtime;
	char buf[512];
	char timestr[16];
	char *ptr;
	struct tm *n;
	struct mymsgbuf *msg = (struct mymsgbuf *) buf;
	if (disable)
		return;
	time(&dtime);
	n = localtime(&dtime);
	sprintf(timestr, "%02d:%02d:%02d", n->tm_hour, n->tm_min, n->tm_sec);
	snprintf(msg->mtext, sizeof (buf) - sizeof (msg->mtype),
		 "%s %s\n", timestr, s);
	ptr = msg->mtext;
	while ((ptr = strchr(ptr, '\n'))) {
		if (!ptr[1])
			break;
		*ptr = '*';
	}
	msg->mtype = 1;
	if (msqid < 0) {
		msqid = initmsq(BBSLOG_MSQ);
		if (msqid < 0) {
			disable = 1;
			return;
		}
	}
	msgsnd(msqid, msg, strlen(msg->mtext), IPC_NOWAIT | MSG_NOERROR);
	return;
}

void
tracelog(char *fmt, ...)
{
	char buf[512];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);
	newtrace(buf);
}

void ailog(char *str) {
	static int disable = 0;
	static int msqid = -1;
	char buf[512];
	struct mymsgbuf *msg = (struct mymsgbuf *) buf;

	if(disable > 3)
		return;

	msg->mtype = 1;
	snprintf(msg->mtext, sizeof (buf) - sizeof (msg->mtype), "%s", str);
        if (msqid < 0) {
                msqid = initmsq(AID_MSQ);
                if (msqid < 0) {
                        disable++;
                        return;
                }
        }
        if(msgsnd(msqid, msg, strlen(msg->mtext), IPC_NOWAIT | MSG_NOERROR) == -1) {
		msqid = -1;
		disable = 0;
	}
	return;
}

void friendslog(char *str) {
	static int count = 0;
	static int msqid = -1;
	char buf[64];
	struct mymsgbuf *msg = (struct mymsgbuf *) buf;

	if(count > 3)
		return;

	msg->mtype = 1;
	snprintf(msg->mtext, sizeof (buf) - sizeof (msg->mtype), "%s", str);
        if (msqid < 0) {
                msqid = initmsq(FRIENDS_MSQ);
                if (msqid < 0) {
                        count++;
                        return;
                }
        }
	if(msgsnd(msqid, msg, strlen(msg->mtext), IPC_NOWAIT | MSG_NOERROR) == -1) {
		msqid = -1;
		count = 0;
	}	
	return;
}


int
deltree(const char *dst)
{
	char rpath[PATH_MAX + 1 + 10], buf[PATH_MAX + 1];
	int i = 0, j = 0, isdir = 0, fd;
	static char *const (disks[]) = {
		MY_BBS_HOME "/home/",
		MY_BBS_HOME "/0Announce/",
		MY_BBS_HOME "/boards/",
		MY_BBS_HOME "/mail/", MY_BBS_HOME "/", NULL
	};
	if (lfile_islnk(dst)) {
		unlink(dst);
		return 1;
	}
	if (realpath(dst, rpath) == NULL)
		return 0;
	j = strlen(rpath);
	if (rpath[j - 1] == '/')
		rpath[j - 1] = 0;
	if (strncmp(rpath, MY_BBS_HOME "/", sizeof (MY_BBS_HOME)))
		return 0;
	strcpy(buf, rpath);
	if (file_isdir(dst))
		isdir = 1;
	for (i = 0; disks[i]; i++) {
		j = strlen(disks[i]);
		if (!strncmp(rpath, disks[i], j))
			break;
	}
	memmove(rpath + j + 6, rpath + j, sizeof (rpath) - j - 6);
	memcpy(rpath + j, ".junk/", 6);
	j += 6;
	normalize(rpath + j);
	rpath[PATH_MAX - 10] = 0;
	j = strlen(rpath);
	i = 0;
	if (isdir) {
		while (mkdir(rpath, 0770) != 0 && i < 1000) {
			sprintf(rpath + j, "+%d", i);
			i++;
		}
		if (i == 1000)
			return 0;
	} else {
		while (((fd = open(rpath, O_CREAT | O_EXCL | O_WRONLY, 0660)) <
			0) && i < 1000) {
			sprintf(rpath + j, "+%d", i);
			i++;
		}
		if (i == 1000)
			return 0;
		close(fd);
	}
	rename(buf, rpath);
	if (lfile_islnk(rpath))
		unlink(rpath);
	return 1;
}

