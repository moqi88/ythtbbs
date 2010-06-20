#include "ythtlib.h"
#include "bbs.h"
#include "sysrecord.c"


#define	BOARD		"test"
#define TMPFILE		MY_BBS_HOME "/bbstmpfs/tmp/weather.html"
#define POSTFILE	MY_BBS_HOME "/bbstmpfs/tmp/deliver"

const static struct source {
	char *title;
	char *url;
	int type;		//类型 0:国内 1:国际
} sina[] = {
	{"华北地区", "http://weather.news.sina.com.cn/text/gnhb24w.html", 0}, 
	{"华东地区", "http://weather.news.sina.com.cn/text/gnhd24w.html", 0}, 
	{"华南地区", "http://weather.news.sina.com.cn/text/gnhn24w.html", 0}, 
	{"华中地区", "http://weather.news.sina.com.cn/text/gnhz24w.html", 0}, 
	{"东北地区", "http://weather.news.sina.com.cn/text/gndb24w.html", 0}, 
	{"西北地区", "http://weather.news.sina.com.cn/text/gnxb24w.html", 0}, 
	{"西南地区", "http://weather.news.sina.com.cn/text/gnxn24w.html", 0}, 
	{"港澳台地区", "http://weather.news.sina.com.cn/text/gnga24w.html", 0}, 
	{"亚洲地区", "http://weather.news.sina.com.cn/text/gwasia24.html", 1}, 
	{"欧洲地区", "http://weather.news.sina.com.cn/text/gweuro24.html", 1}, 
	{"美洲地区", "http://weather.news.sina.com.cn/text/gwamer24.html", 1}, 
	{"大洋州地区", "http://weather.sina.com.cn/text/gwocea24.html", 1}, 
	{"非洲地区", "http://weather.sina.com.cn/text/gwafri24.html", 1}, 
	{NULL, NULL, 0}
};

struct weather {
	char province[20];	//省份
	char city[20];		//城市
	char weather1[20];	//天气一
	char wind1[10];		//风力一
	int temperature1;	//温度一
	char weather2[10];	//天气二
	char wind2[20];		//风力二
	int temperature2;	//温度二
};

void getfile(char *url) {
	char buf[1024];
	
	if (!*url)
		return;
	sprintf(buf, "cd " MY_BBS_HOME "/bbstmpfs/tmp/; "
			"/usr/bin/wget -c -q %s -O " TMPFILE " "
			"> /dev/null", url);
	system(buf);
	return;
}

inline char *newstrstr(char *start, char *tag, int num) {
	int i;
	char *ptr;
	
	ptr = start;
	if (!ptr)
		return NULL;
	for (i = 0; i < num; i++) {
		if (!(ptr = strcasestr(ptr, tag) + 1))
			return NULL;
		ptr++;
	}
	return ptr;
}

inline char *strhtmltag(char *ptr, char *tag){
	char buf[24];

	sprintf(buf, "<%s", tag);
	ptr = strcasestr(ptr, buf);
	ptr = strchr(ptr, '>');

	if (ptr)
		return ++ptr;
	else
		return NULL;
}

void print_head(FILE *fp, char *date1, char *date2, char *title, int type) {
	char buf[75];
	int i, j;

	sprintf(buf, "│%70s│", "");

	i = strlen(title);
	j = (56 - i) / 2 + 2;

	strncpy(buf + j, title, i);
	strncpy(buf + j + i, "24小时天气预报", 14);

	if (NULL == fp)
		return;
	fprintf(fp, "\033[<tt>");
	fprintf(fp, "┌──────────────"
			"─────────────────────┐\n");
	if (type)
		fprintf(fp, "%s\n"
				"├─────────┬────────"
				"────┬────────────┤\n", buf);
	else
		fprintf(fp, "%s\n"
			"├────┬────┬───────"
			"─────┬────────────┤\n", buf);
	sprintf(buf, "│%8s%2s%8s│%24s│%24s│", "", type ? "" : "│", 
			"", "", "");
	i = strlen(date1);
	j = 22 + (24 - i) / 2;
	strncpy(buf + j, date1, i);
	i = strlen(date2);
	j = 48 + (24 - i) / 2;
	strncpy(buf + j, date2, i);

	fprintf(fp, "%s\n", buf);
	if (type)
		fprintf(fp, "│     城    市     ├────┬─"
				"───┬──┼────┬────┬──┤\n");
	else
		fprintf(fp, "│  地区  │ 城  市 ├────"
			"┬────┬──┼────┬────┬──┤\n");
	fprintf(fp, "│        %2s        │ 天  气 │"
		"风向风力│温度│ 天  气 │风向风力│温度│\n", 
		type ? "" : "│");
	return;
}

void print_item(FILE *fp, struct weather item, int type, int rowspan) {
	char buf[75], buf2[5];
	int i, j;

	if (NULL == fp)
		return;

	if (!item.city || !item.weather1 || !item.wind1 || 
			!item.weather2 || !item.wind2)
		return;

	if (strchr(item.province, '<'))
		*strchr(item.province, '<') = 0;
	if (strchr(item.city, '<'))
		*strchr(item.city, '<') = 0;
	if (strchr(item.weather1, '<'))
		*strchr(item.weather1, '<') = 0;
	if (strchr(item.wind1, '<'))
		*strchr(item.wind1, '<') = 0;
	if (strchr(item.weather2, '<'))
		*strchr(item.weather2, '<') = 0;
	if (strchr(item.wind2, '<'))
		*strchr(item.wind2, '<') = 0;

	bzero(buf, sizeof(buf));
	if (!type) {
		strncpy(buf, "│        │        │        "
			"│        │    │        │        │    │", 74);
		if ((i = strlen(item.province))) {
			j = 2 + (8 - i) / 2, 0;
			strncpy(buf + j, item.province, i);
		}
	} else 
		strncpy(buf, "│                  │        │        │    │        │        │    │", 74);
	i = strlen(item.city);
	if (!type)
		j = 12 + (8 - i) / 2;
	else
		j = 2 + (18 - i) / 2;
	strncpy(buf + j, item.city, i);

	i = strlen(item.weather1);
	j = 22 + (8 - i) / 2;
	strncpy(buf + j, item.weather1, i);

	i = strlen(item.wind1);
	j = 32 + (8 - i) / 2;
	strncpy(buf + j, item.wind1, i);

	if (item.temperature1 != -100)
		sprintf(buf2, "%d", item.temperature1);
	else
	    	sprintf(buf2, "NA");
	i = strlen(buf2);
	j = 45 - i;
	strncpy(buf + j, buf2, i);

	i = strlen(item.weather2);
	j = 48 + (8 - i) / 2;
	strncpy(buf + j, item.weather2, i);

	i = strlen(item.wind2);
	j = 58 + (8 - i) / 2;
	strncpy(buf + j, item.wind2, i);

	if (item.temperature2 != -100)
		sprintf(buf2, "%d", item.temperature2);
	else
	    	sprintf(buf2, "NA");
	i = strlen(buf2);
	j = 71 - i;
	strncpy(buf + j, buf2, i);

	fprintf(fp, "%s\n", buf);
}

char *weather_date(FILE *fp, char *ptr, char *title, int type) {
	char date1[20], date2[20];

	if (NULL == fp || !ptr)
		return NULL;
	if (NULL == (ptr = strstr(ptr, "标准头结束")))
		return NULL;
	if (NULL == (ptr = newstrstr(ptr, "<TABLE", 12)))
		return NULL;
	if (NULL == (ptr = newstrstr(ptr, "<TD", 3)))
		return NULL;
	if (NULL == (ptr = strchr(ptr, '>')))
		return NULL;
	strncpy(date1, ++ptr, sizeof(date1));
	if (strchr(date1, '<'))
		*strchr(date1, '<') = 0;
	ptr = newstrstr(ptr, ">", 2);
	if (!ptr)
		return 0;
	strncpy(date2, --ptr, sizeof(date2));
	if (strchr(date2, '<'))
		*strchr(date2, '<') = 0;
	print_head(fp, date1, date2, title, type);
	ptr = newstrstr(ptr, "<tr", 2);
	ptr = strhtmltag(ptr, "/td");
	return ptr;
}

void weather_item(FILE *fp, char *ptr, int type) {
	int size, pos, rowspan = 0;
	struct weather item;
	char *str;

	if (NULL == (str = strstr(ptr, "<!--开始：底部-->")))
		return;
	size = str - ptr;
	pos = 0;

	while (pos < size) {
		str = ptr;
		ptr = strcasestr(ptr, "<td");
		if (!ptr || ptr - str + pos >= size)
			break;
		if (strchr(ptr, '>') && strcasestr(ptr, "rowspan")) {
			if (strcasestr(ptr, "rowspan") < strchr(ptr, '>')) {
				strncpy(item.province, strhtmltag(ptr, "td"), 
						sizeof(item.province));
				rowspan = atoi(strstr(ptr, "rowspan") + 9);
				ptr = strhtmltag(ptr, "/td");
				if (!ptr)
					break;
				fprintf(fp, "├────┼────┼──"
					"──┼────┼──┼────┼"
					"────┼──┤\n");
			} else
				strcpy(item.province, "");
		} else 
			strcpy(item.province, "");
		if (!*item.province) {
			if (!type)
				fprintf(fp, "│        ├────┼──"
					"──┼────┼──┼────┼"
					"────┼──┤\n");
			else 
				fprintf(fp, "├─────────┼───"
					"─┼────┼──┼────┼"
					"────┼──┤\n");
		}
		if (!(ptr = strhtmltag(ptr, "td")))
			break;
		if (!(ptr = strhtmltag(ptr, "a")))
			break;
		strncpy(item.city, ptr, type ? sizeof(item.city): 8);

		if (!(ptr = strhtmltag(ptr, "td")))
			break;
		if (!(strncmp(ptr, "&nbsp;", 6)))
		    	strncpy(item.weather1, "不详", sizeof(item.weather1));
		else
			strncpy(item.weather1, ptr, sizeof(item.weather1));
	
		if (!(ptr = strhtmltag(ptr, "td")))
			break;
		if (!(strncmp(ptr, "&nbsp;", 6)))
		    	strncpy(item.wind1, "不详", sizeof(item.weather1));
		else
			strncpy(item.wind1, ptr, sizeof(item.wind1));
	
		if (!(ptr = strhtmltag(ptr, "td")))
			break;
		if (!(strncmp(ptr, "&nbsp;", 6)))
		    	item.temperature1 = -100;
		else
			item.temperature1 = atoi(ptr);
		
		if(!(ptr = strhtmltag(ptr, "td")))
			break;
		if (!(strncmp(ptr, "&nbsp;", 6)))
		    	strncpy(item.weather2, "不详", sizeof(item.weather1));
		else
			strncpy(item.weather2, ptr, sizeof(item.weather2));

		if(!(ptr = strhtmltag(ptr, "td")))
			break;
		if (!(strncmp(ptr, "&nbsp;", 6)))
		    	strncpy(item.wind2, "不详", sizeof(item.weather1));
		else
			strncpy(item.wind2, ptr, sizeof(item.wind2));
		if(!(ptr = strhtmltag(ptr, "td")))
			break;
		if (!(strncmp(ptr, "&nbsp;", 6)))
			item.temperature2 = -100;
		else
			item.temperature2 = atoi(ptr);

		print_item(fp, item, type, rowspan);
		if (!(ptr = strhtmltag(ptr, "/tr")))
			break;
		pos += ptr - str;
	}

	if (type)
		fprintf(fp, "└─────────┴────┴────┴"
				"──┴────┴────┴──┘");
	else
		fprintf(fp, "└────┴────┴────┴────┴─"
				"─┴────┴────┴──┘\n");
	fprintf(fp, "\033[</tt>");
	return;
}

int main (void) {
	struct mmapfile mf = { ptr:NULL, size:0 };
	int i = 0;
	char *ptr, title[64];
	FILE *fp;

	while (sina[i].title) {
		getfile(sina[i].url);
		if (!file_exist(TMPFILE)) {
			i++;
			continue;
		}
		if (NULL == (fp = fopen(POSTFILE, "w")))
			return 0;

		if (mmapfile(TMPFILE, &mf) < 0) {
			i++;
			continue;
		}
		if (mf.size <= 0) {
			i++;
			continue;
		}
		ptr = mf.ptr;
		MMAP_TRY {
			if (NULL == 
				(ptr = weather_date(fp, ptr, 
				 sina[i].title, sina[i].type))) {

				MMAP_UNTRY;
				mmapfile(NULL, &mf);
				continue;
			}
			weather_item(fp, ptr, sina[i].type);
		} MMAP_CATCH {
		} MMAP_END mmapfile(NULL, &mf);
		fclose(fp);
		sprintf(title, "%s城市24小时天气预报(%s)", 
				sina[i].type ? "国际" : "国内", 
				sina[i].title);
		postfile(POSTFILE, "deliver", BOARD, title);
		unlink(TMPFILE);
		unlink(POSTFILE);
		i++;
	}
	return 1;
}
