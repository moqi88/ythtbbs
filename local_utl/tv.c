#include "bbs.h"
#include "sysrecord.c"

#define TMPFILE 	MY_BBS_HOME "/bbstmpfs/tmp/tv.html"
#define POSTFILE	MY_BBS_HOME "/bbstmpfs/tmp/tvdeliver"
#define MAXDAYS		14
#define MAXSECS		20
#define BOARD		"TV"

const static struct source {
	int day;
	char *url;
} sina[] = {
	{1, "http://ent.sina.com.cn/tv/g/1.html"},
	{2, "http://ent.sina.com.cn/tv/g/2.html"},
	{3, "http://ent.sina.com.cn/tv/g/3.html"},
	{4, "http://ent.sina.com.cn/tv/g/4.html"},
	{5, "http://ent.sina.com.cn/tv/g/5.html"},
	{6, "http://ent.sina.com.cn/tv/g/6.html"},
	{7, "http://ent.sina.com.cn/tv/g/7.html"},
	{8, "http://ent.sina.com.cn/tv/g/8.html"},
	{0, NULL}
};

const static char *week[] = {"日", "一", "二", "三", "四", "五", "六"};

int getfile(char *url) {
	char buf[1024];

	if (!url)
		return 0;
	unlink(TMPFILE);
	
	snprintf(buf, sizeof(buf), "cd " MY_BBS_HOME "/bbstmpfs/tmp/; "
			"/usr/bin/wget -c -q %s -O " TMPFILE " > /dev/null", 
			url);
	system(buf);
	return 1;
}

int tv_line(FILE *fp, char *buf) {
	char *ptr, *str, *time, *name;
	int len = strlen(buf);

	if (!fp || !buf)
		return 0;

	str = buf;
	if (!(time = strchr(str, ':')))
		return 0;
	time -= 2;
	if (time <= str)
		return 0;
	if ((ptr = strstr(str, "　")) && ptr < time)
		*ptr = 0;
	if ((ptr = strchr(str, ' ')) && ptr < time)
		*ptr = 0;
	*(time - 1) = 0;
	name = time + 5;
	if (!strncmp(name, "　", 2))
		name += 2;
	while (*name == ' ')
		name++;
	if (name - buf >= len || time + 5 - buf >= len)
		return 0;
	*(time + 5) = 0;
	fprintf(fp, "    \033[1;32m%-15s\033[1;33m%-7s\033[1;36m", str, time);
	str = name;
	if (!(ptr = strchr(str, '<'))) {
		fprintf(fp, "%s\033[m\n", str);
		return 1;
	}
	while (ptr) {
		*ptr++ = 0;
		fprintf(fp, "%s", str);
		str = ptr;
		if (!(ptr = strchr(str, '>'))) {
			fprintf(fp, "%s\033[m\n", str);
			return 1;
		}
		str = ++ptr;
		if (!(ptr = strchr(str, '<'))) {
			fprintf(fp, "%s\033[m\n", str);
			return 1;
		}
	}
	return 1;
}

int tv_item(char *str, char *daytitle, time_t now) {
	char *ptr, *end, *start;
	char sectitle[10], buf[256];
	FILE *fp;
	int line = 1;
	struct tm *nowday = localtime(&now);

	if (!str || !daytitle)
		return 0;
	if (!(ptr = strstr(str, "<td")))
		return 0;
	if (!(ptr = strchr(ptr, '>')))
		return 0;
	ptr++;
	strncpy(sectitle, ptr, sizeof(sectitle));
	if (strchr(sectitle, '<'))
		*strchr(sectitle, '<') = 0;
	if (!(ptr = strstr(ptr, "<td")))
		return 0;
	if (!(ptr = strchr(ptr, '>')))
		return 0;
	start = ++ptr;
	while (*start == ' ' || *start == '\n' || *start == '\r')
		start++;
	if (!(end = strstr(ptr, "</td")))
		return 0;
	ptr = start;
	if (!(fp = fopen(POSTFILE, "w")))
		return 0;
	fprintf(fp, "\n\033[1;36m"
			"────────────────────"
			"───────────────────\033[m\n"
			"\033[1;32m    电视节目预告[%s]  " 
			"%d年%d月%d日(星期%s)\033[m"
			"\n\033[1;36m"
			"────────────────────"
			"───────────────────\033[m\n\n",
			sectitle, nowday->tm_year + 1900, nowday->tm_mon + 1, 
			nowday->tm_mday, week[nowday->tm_wday]);
	while (*ptr && ptr < end) {
		start = ptr;
		if (!(ptr = strstr(start, "<br")))
			ptr = end;
		bzero(&buf, sizeof(buf));
		strncpy(buf, start, min(ptr - start, sizeof(buf)));
		tv_line(fp, buf);
		if (!(line++ % 5))
			fprintf(fp, "\n");
		ptr += 4;
		while (*ptr && !isalpha(*ptr) && !isdigit(*ptr) && 
				!((unsigned char)(*ptr) > 175 
					&& (unsigned char)*ptr < 248))
			ptr++;
	}
	fprintf(fp, "\n");
	fclose(fp);
	snprintf(buf, sizeof(buf), "%s电视节目预告(%s)", daytitle, sectitle);
	postfile(POSTFILE, "deliver", BOARD, buf);
	unlink(POSTFILE);
	return 1;
}

int tv_day(char *url, char *daytitle) {
	struct mmapfile mf = { ptr: NULL };
	char *ptr, *(tag[MAXSECS]), *end;
	int i;
	time_t now = time(NULL);

	if (!url || !daytitle)
		return 0;
	if (!getfile(url))
		return 0;
	if (!file_exist(TMPFILE))
		return 0;
	if (mmapfile(TMPFILE, &mf) < 0)
		return 0;

	if (!(ptr = strstr(mf.ptr, "<!--开始:节目预告")))
		return 0;
	if (!(ptr = strstr(ptr, "<table")))
		return 0;
	if (!(end = strstr(ptr, "</table")))
		return 0;
	bzero(&tag, sizeof(tag));
	for (i = 0; i < MAXSECS; i++) {
		ptr = strstr(ptr, "<tr");
		if (!ptr || ptr > end)
			break;
		tag[i] = ptr++;
	}
	if (!strcmp(daytitle, "明日"))
		now += 24 * 60 * 60;
	for (i = 0; i < MAXSECS && tag[i]; i++) {
		tv_item(tag[i], daytitle, now);
	}
	return 1;
}

int main() {
	time_t now = time(0);
	struct tm *nowday = localtime(&now);
	int day = nowday->tm_wday == 0 ? 7 : nowday->tm_wday;
	int i = 0;

	for (i = 0; i < MAXDAYS; i++) {
		if (!sina[i].day)
			break;
		else if (sina[i].day == day)
			tv_day(sina[i].url, "今日");
		else if (sina[i].day == day + 1)
			tv_day(sina[i].url, "明日");
	}
	return 1;
}
