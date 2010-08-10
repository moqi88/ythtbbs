#include "ythtlib.h"
void
_errlog(char *fmt, ...)
{
	FILE *fp;
	int proc;
	char buf[1024], timestr[16], progname[256], *thetime;
	time_t dtime;
	int i = 0, k;
	pid_t pid;
	va_list ap;

	pid = getpid();
	snprintf(buf, sizeof (buf), "/proc/%d/cmdline", pid);
	if ((proc = open(buf, O_RDONLY)) >= 0) {
		i = read(proc, progname, sizeof (progname));
		close(proc);
	}
	if (i > 0) {
		for (k = 0; k < i; k++)
			if (!progname[k])
				progname[k] = ' ';
		snprintf(buf, i + 1, "%s", progname);
		strcpy(buf + i, "|");
		i++;
	} else
		i = 0;
	va_start(ap, fmt);
	vsnprintf(buf + i, 1000 - i, fmt, ap);
	va_end(ap);
	buf[1000] = 0;

	time(&dtime);
	thetime = ctime(&dtime);
	strncpy(timestr, &(thetime[4]), 15);
	timestr[15] = '\0';
	fp = fopen(ERRLOG, "a");

	if (fp != NULL) {
		fprintf(fp, "%s %d %s\n", timestr, pid, buf);
		fclose(fp);
	}
}

void writelog(char* logfilename, char* log)
{
	FILE *fp;
	fp=fopen(logfilename, "a+");
	fputs(log,fp);
	fclose(fp);
}

void writesyslog(int logtype, char* log)
{
	char logfilename[256];
	FILE *fp;
	char yearmonth[5];  // current year and month
	time_t now;
	now=time(NULL);
	strftime(yearmonth, 5, "%y%m", localtime(&now));
	// log type can be error, admin and test, which is defined as ERRORLOG,ADMINLOG and TESTLOG
	if (logtype == ERRORLOG)
		sprintf(logfilename, "%s%s%s", "/home/bbs/reclog/", yearmonth, "error.log");
	else if (logtype == ADMINLOG)
		sprintf(logfilename, "%s%s%s", "/home/bbs/reclog/", yearmonth, "admin.log");
	else if (logtype == TESTLOG)
		sprintf(logfilename, "%s%s%s", "/home/bbs/reclog/", yearmonth, "test.log");
	else if (logtype == TESTLOGLOG)
		sprintf(logfilename, "/home/bbs/reclog/testlog.log");
	else
		sprintf(logfilename, "%s%s%s", "/home/bbs/reclog/", yearmonth, "illegal.log");
	// write log in the selected file
	fp = fopen(logfilename , "a+");
	fputs(log,fp);
	fclose(fp);
}

int
mystrtok(char *buf, int c, char *tmp[], int max)
{
	int i;
	char *a;
	for (i = 0; i < max - 1; i++) {
		a = index(buf, c);
		if (a != NULL) {
			strncpy(tmp[i], buf, a - buf);
			tmp[i][a - buf] = 0;
			buf = a + 1;
			continue;
		}
		break;
	}
	strcpy(tmp[i], buf);
	return i + 1;
}
