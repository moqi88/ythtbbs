#include <stdio.h>
#include <sys/mman.h>
#include "bbs.h"
#include "ythtlib.h"
#include "www.h"

#define FILTERTITLE	MY_BBS_HOME "/etc/filtertitle"
#define BADBOARDS	MY_BBS_HOME "/etc/noboaboards"
#define NAVFILE		MY_BBS_HOME "/wwwtmp/navpart.txt"
//#define OUTPUT		MY_BBS_HOME "/wwwtmp/digest.js"
//for test
#define OUTPUT		MY_BBS_HOME "/wwwtmp/daodu.htm"
#define DAODUMORE	MY_BBS_HOME "/wwwtmp/daodumore.htm"
#define JINGHUA     MY_BBS_HOME "/wwwtmp/jinghua.htm"
#define JINGHUAMORE MY_BBS_HOME "/wwwtmp/jinghuamore.htm"
#define TEMPOUT		MY_BBS_HOME "/wwwtmp/tempout.txt"
#define COLUM		MY_BBS_HOME "/wwwtmp/digestcolum"
#define MAXBADTITLE	64
#define MAXBADBOARDS	64
#define MAXDIGESTTIME	7 * 24 * 3600
#define MAXJINGHUA	20
#define MAXDAODU 33
// the average evaluation used in daodu selection
#define AVGEVA 2

struct bbsinfo bbsinfo;
struct BCACHE *shm_bcache;
struct digestitem {
	int bnum;
	int ftime;
	char title[60];
	char owner[14];
	char bname[24];
	char sec;
	int eva;
	int neweva;
} *pdigest;
//  for backup
//	float neweva;  // new evaluation depends on file time and eva   
//  we do not use this variable for the digest only sort with ftime

static int
cmp_ftime(const void *a, const void *b)
{
	return ((struct digestitem *) b)->ftime -
	    ((struct digestitem *) a)->ftime;
}

static int
cmp_neweva(const void *a, const void *b)
{
	return ((struct digestitem *) b)->neweva -
	    ((struct digestitem *) a)->neweva;
}


int
getbnum(char *board)
{
	int i;
	char *ptr;
	ptr = strchr(board, '\n');
	if (ptr)
		*ptr = 0;
	struct boardmem *bcache;
	bcache = bbsinfo.bcacheshm->bcache;
	for (i = 0; i < MAXBOARD && i < bbsinfo.bcacheshm->number; i++) {
		if (!strcasecmp(board, bcache[i].header.filename))
			return i;
	}
	return -1;
}

char *
getbtitle(int bnum) {
	struct boardmem *bcache;
	bcache = bbsinfo.bcacheshm->bcache;
	if (bnum < 0 || bnum > min(MAXBOARD, bbsinfo.bcacheshm->number))
		return NULL;
	return bcache[bnum].header.title;
}


void encodetitle(char *newtitle, char *oldtitle) {
	int d, k;
	int count = 0;
	newtitle[0] = 0;
	for (d = 0; d < strlen(oldtitle); d++) {
		if (oldtitle[d] == '\'')
			strcat(newtitle, "\\\'");
		else if (oldtitle[d] == '\\')
			strcat(newtitle, "\\\\");
		else
			sprintf(newtitle, "%s%c", 
					newtitle, oldtitle[d]);
	}
	d = 0;
	k = strlen(newtitle);
	while(d < k) {
		if ((unsigned char)newtitle[d] >= 128)
			count++;
		else if(count % 2)
			count++;
		d++;
	}
	if (count % 2) {
		for(d = 0; d < k; d++) {
			if ((unsigned char)newtitle[k - d - 1] >= 128) {
				newtitle[k - d - 1] = '\0';
				break;
			}
		}
	}
}

int checkold(int digesttime, int now) {
	if (now - digesttime > MAXDIGESTTIME)	//一个月以前的就不进文摘了
		return 1;
	return 0;
}

int 
checkmarked(char *digestboard, int ftime, char *title, char *owner) {
	char dir[STRLEN];
	struct fileheader fhdr;
	int fd, total, n, allsitedigest, retv = 0;

	allsitedigest = !strcmp("Digest", digestboard);

	sprintf(dir, MY_BBS_HOME "/boards/%s/.DIR", digestboard);
	if ((total = file_size(dir) / sizeof (fhdr)) <= 0)
		return retv;
	if ((fd = open(dir, O_RDONLY, 0)) == -1)
		return retv;
	for (n = total -1; n >= 0 && total - n < 3000; n--) {
		if (lseek(fd, n * sizeof (fhdr), SEEK_SET) < 0)
			break;
		if (read(fd, &fhdr, sizeof (fhdr)) != sizeof (fhdr))
			break;
		if (allsitedigest && ftime == fhdr.filetime) {
			if (!(fhdr.accessed & FH_DEL)) {
				sprintf(title, fhdr.title);
				sprintf(owner, fhdr.owner);
				retv = 1;
			}
			break;
		}
		if ((fhdr.accessed & FH_MARKED) && ftime == fhdr.filetime) {
			sprintf(title, fhdr.title);
			sprintf(owner, fhdr.owner);
			retv = 1;
			break;
		}
	}
	close(fd);

	return retv;
}

int checkexist(FILE *fp, int bnum, int oldbnum, 
		int oldftime, int newbnum, int newftime, 
		int digesttime, char secstr, int loop, int *eva) {
//检查DIGESTLOG中涉及的文章是否仍然存在
//如果在文摘版面已经不存在，则返回0
//如果在文章版面仍然存在，检查是否在原始版面存在
//如果在原始版面存在，则将原记录重新写回DIGESTLOG
//如果在原始版面不存在，则将文章版面的记录写入DIGESTLOG
	char dir[STRLEN], boardname[STRLEN];
	struct fileheader fhdr;
	int fd, total, n, indigest, retv = 0;

	if (NULL == fp)
		return retv;

	strncpy(boardname, bbsinfo.bcache[bnum - 1].header.filename, STRLEN);
	if (!strcmp("Digest", boardname) || !strcmp("Digest", boardname + 1))
		indigest = 1;
	else
		indigest = 0;
	sprintf(dir, MY_BBS_HOME "/boards/%s/.DIR",  boardname);
	
	if ((total = file_size(dir) / sizeof (fhdr)) <= 0)
		return retv;
	if ((fd = open(dir, O_RDONLY, 0)) == -1)
		return retv;
	for (n = total - 1; n >= 0 && total - n < 3000; n--) {
		if (lseek(fd, n * sizeof (fhdr), SEEK_SET) < 0)
			break;
		if (read(fd, &fhdr, sizeof (fhdr)) != sizeof (fhdr))
			break;
		if (indigest && newftime == fhdr.filetime) {
			retv = 1;
			break;
		} else if((!indigest || (indigest && !loop))
				&& oldftime == fhdr.filetime) {
			retv = 1;
			break;
		}
	}
	if (indigest && loop) {
		if (retv != 0) {
			retv =  checkexist(fp, oldbnum, oldbnum, 
					oldftime, newbnum, newftime, 
					digesttime, secstr, 0, eva);
			if (retv == 0) {
				fprintf(fp, "%d\t%d\t%d\t%d\t%d\t%c\n", 
						digesttime,
						newbnum, newftime, newbnum, 
						newftime, secstr);
				retv = 1;
			}
		}
	} else if (retv != 0)
		fprintf(fp, "%d\t%d\t%d\t%d\t%d\t%c\n", digesttime, 
				oldbnum, oldftime, newbnum, newftime, secstr);
	close(fd);

	if (retv)
		*eva = fhdr.staravg50 * fhdr.hasvoted / 50;
	else
		*eva = 0;
	
	return retv;
}

int write_detail(FILE *wwwfp, char *filename) {
	char *ptr, *str, tmp = 0;
	struct mmapfile mf = { ptr:NULL };
	int len = 0, i, half = 0;
	int ischangeline=1;

	if (!wwwfp || !filename)
		return 0;
	MMAP_TRY {
		if (mmapfile(filename, &mf) == -1)
			MMAP_RETURN(0);
		str = mf.ptr;
		for (i = 0; i < 4; i++) {
			if (!(ptr = strchr(str, '\n')))
				break;
			str = ++ptr;
		}
		
		while (*str && str < mf.ptr + mf.size && len < 400) {
			switch (*str) {
				case '\r':			//不要break
					str++;
				case '\n':
					str++;
					if(ischangeline==0)  //防止第一次换行与多次连续换行
					{
						fprintf(wwwfp, "<br>");
						ischangeline=1;
					}
					continue;
				case '\033':
					if (!strncmp(str, "\033[<", 3))
						ptr = strchr(str, '>');
					else if (!strncmp(str, "\033[", 2))
						ptr = strchr(str, 'm');
					if (NULL == ptr)
						str = mf.ptr + mf.size;
					else
						str = ++ptr;
					continue;
				case '\\':
					fprintf(wwwfp, "\\\\");
					len++;
					str++;
					continue;
				case '\'':
					fprintf(wwwfp, "\\\'");
					len++;
					str++;
					continue;
				case '\"':
					fprintf(wwwfp, "\\\"");
					len++;
					str++;
					continue;
				case '<':
					fprintf(wwwfp, "&lt;");
					len++;
					str++;
					continue;
				case '>':
					fprintf(wwwfp, "&gt;");
					len++;
					str++;
					continue;
				case '&':
					fprintf(wwwfp, "&amp;");
					len++;
					str++;
					continue;
				default:
					if (ischangeline==1)  //回复\n的判断
					{
						ischangeline = 0;
					}
					if (!strncmp(str, "beginbinaryattach", 18)) {
						if (NULL == (ptr = strchr(str, '\n')))
							str = mf.ptr + mf.size;
						else
							str = ++ptr;
						continue;
					}
					if (!strncmp(str, "--\n", 3)) {
						str = mf.ptr + mf.size;
						continue;
					}
					if ((unsigned char)(*str) > 128) {
						half = (half + 1) % 2;
						if (half) {
							tmp = *str;
							str++;
							continue;
						} else {
							if (!tmp) 
								continue;
							fprintf(wwwfp, "%c%c", tmp, *str);
							tmp = 0;
							str++;
							len += 2;
							continue;
						}
					} else {
						tmp = 0;
						fprintf(wwwfp, "%c", *str);
						str++;
						len++;
						continue;
					}
			}  //end switch
		}  //end while
	} MMAP_CATCH {
		MMAP_RETURN(0);
	} MMAP_END mmapfile(NULL, &mf);
	return 1;
}

int show_detail(FILE *wwwfp, FILE *jinghuafp) {
	char buf[STRLEN], buf2[STRLEN];
	char colum, board[STRLEN], title[STRLEN], owner[IDLEN + 2];
	FILE *colum_fp, *log_fp;
	int fd, total, i; 
	int newftime, oldftime, oldbnum, tmpoldftime, tmpoldbnum, tmpnewftime;
	struct fileheader fhdr;

	if (!wwwfp)
		return 0;
	if (!jinghuafp)
		return 0;
	sprintf(buf, MY_BBS_HOME "/wwwtmp/digestcolum");
	if (NULL == (colum_fp = fopen(buf, "r"))) {
		return 0;
	}
	while (fgets(buf, sizeof(buf), colum_fp)) {
		if (buf[0] == '#')
			continue;
		board[0] = 0;
		bzero(&fhdr, sizeof(fhdr));
		newftime = 0;
		sscanf(buf, "%c\t%*s\t%*s\t%*d\t%*d\t%s", 
				&colum, board);
		
		//查找最后一篇被g的文章。
		sprintf(buf, MY_BBS_HOME "/boards/%s/.DIGEST", board);
		if ((total = file_size(buf) / sizeof (fhdr)) <= 0)
			continue;
		if ((fd = open(buf, O_RDONLY, 0)) == -1)
			//当前版面没有被g的文章
			continue;
		if (lseek(fd, (total - 1) * sizeof (fhdr), SEEK_SET) < 0)
			continue;
		if (read(fd, &fhdr, sizeof (fhdr)) != sizeof (fhdr))
			continue;
		sprintf(buf2, MY_BBS_HOME "/boards/%s/G.%d.A", 
				board, fhdr.filetime);
		strncpy(title, fhdr.title, sizeof(title));
		strncpy(owner, fhdr.owner, sizeof(owner));
		close(fd);
		sprintf(buf, MY_BBS_HOME "/boards/%s/.DIGEST", board);
		if ((total = file_size(buf) / sizeof(fhdr)) <= 0) 
			continue;
		if ((fd = open(buf, O_RDONLY, 0)) == -1)
			continue;
		for (i = total - 1; i >= 0 && i >= total - 1000; i++) {
			if (lseek(fd, i * sizeof(fhdr), SEEK_SET) < 0)
				break;
			if (read(fd, &fhdr, sizeof(fhdr)) != sizeof(fhdr))
				break;
			if (!strcmp(fhdr.title, title) && 
					!strcmp(fhdr.owner, owner)) {
				newftime = fhdr.filetime;
				break;
			}
		}

		oldftime = 0;
		oldbnum = 0;
		if (newftime){
			sprintf(buf, MY_BBS_HOME "/boards/%s/.DIGESTLOG", 
					board);
			if (NULL != (log_fp = fopen(buf, "r"))) {
				while (fgets(buf, sizeof(buf), log_fp)) {
					sscanf(buf, 
						"%*d\t%d\t%d\t%*d\t%d\t%*c", 
						&tmpoldbnum, &tmpoldftime, 
						&tmpnewftime);
					if (tmpnewftime == newftime) {
						oldbnum = tmpoldbnum;
						oldftime = tmpoldftime;
					}
				}
			}
			fclose(log_fp);
		}

		fprintf(jinghuafp, "<div id=\"digest_%c_detail\" class=\"digest_detail_box_box\">\n", colum);
		encodetitle(title, fhdr.title);
		fprintf(jinghuafp, "<div class=\"digest_detail_box\">\n <div class=\"digest_detail_title\">%s </div>\n", title);
		fprintf(jinghuafp, "<div class=\"digest_detail_content\">\n");
		write_detail(jinghuafp, buf2);
		fprintf(jinghuafp, "<div align=\"right\"> [<a title=\"查看全文\" href=\"/HT/con_%d_M.%d.A.htm\">查看全文</a>] </div>\n", oldbnum - 1, oldftime);
		fprintf(jinghuafp, "</div>\n</div>\n</div>\n");
	}
	fclose(colum_fp);
	return 1;
}

void printDigestColumn(FILE *fp, char flag, char* title, char* sec, int maxitem, int order)
{
	fprintf(fp, "<div id=\"digest_%c\" class=\"digest_box\">\n", flag); 
	fprintf(fp, "<h2 id=\"digest_%c_title\"> %s \n", flag, title);
	// show more
	fprintf(fp,	"<span class=\"digest_more\"><a href=\"/HT/digest?C=%c\" title=\"更多%s栏目文章\">[更多]</a></span>\n", flag, title);
	if(flag=='1')
		fprintf(fp, "<a href=\"/HT/bbsrss?rssid=130&m=0\" target=\"_blank\"><img src=\"/rss.gif\" border=\"0\" /></a>");
	fprintf(fp, "</h2>\n");
}

/*
		fprintf(fp, "DL.addcolum('%c', '%s', '%s', %d, %d);\n", 
				flag, title, sec, maxitem, order);
		change javascript to C  for SEO, search engine could not recognize contents in javascript
		printDigestColum(fp, flat, title, sec, maxitem, order);
DL.addcolum('1', '文摘', '0123456789TYSLM', 8, 1);
DL.addcolum('0', '近日精彩话题', '@', 22, 1);
*/

int initcolum(FILE *fp, FILE *jfp) {
	//init digest colum from digestcolum
	//for more details, see digestcolum
	FILE *colum;
	char buf[256];
	char flag, title[40], sec[20], *ptr, *str;
	int maxitem, order;

	if (!fp)
		return 0;
	if (!jfp)
		return 0;
//change javascript to C for SEO
/*
	fprintf(fp, "<script>\n"
			"var DL = new digest();\n");
			*/
	if (!(colum = fopen(COLUM, "r")))
		return 0;

	while (fgets(buf, sizeof(buf), colum)) {
		if (*buf == '#')
			continue;
		str = buf;
		flag = *str;
		if (!(ptr = strchr(str, '\t')))
			continue;
		while (*ptr == '\t')
			ptr++;
		str = ptr;
		if (!(ptr = strchr(str, '\t')))
			continue;
		*ptr++ = 0;
		strncpy(title, str, sizeof(title));
		str = ptr;
		while (*ptr == '\t')
			ptr++;
		str = ptr;
		if (!(ptr = strchr(str, '\t')))
			continue;
		*ptr++ = 0;
		strncpy(sec, str, sizeof(sec));
		while (*ptr == '\t')
			ptr++;
		str = ptr;
		maxitem = atoi(str);
		if (!(ptr = strchr(str, '\t')))
			continue;
		while (*ptr == '\t')
			ptr++;
		str = ptr;
		order = atoi(str);
		/*
		fprintf(fp, "DL.addcolum('%c', '%s', '%s', %d, %d);\n", 
				flag, title, sec, maxitem, order);
				*/
//		change javascript to C  for SEO, search engine could not recognize contents in javascript
		if(flag=='0')
			printDigestColumn(fp, flag, title, sec, maxitem, order);
		else if(flag=='1')
			printDigestColumn(jfp, flag, title, sec, maxitem, order);
		else
			;
	}
	fclose(colum);
	return 1;
}

int write_item(FILE *fp, struct digestitem *pdigest) {
	if (!fp || !pdigest)
		return 0;
/*	fprintf(fp, "DL.additem(%d, %d, '%s', '%s', '%s', '%c', %d);\n", 
			pdigest->bnum, pdigest->ftime, pdigest->title,
			pdigest->owner, pdigest->bname, 
			pdigest->sec, pdigest->eva);
	DL.additem(65, 1234083749, '健康男人离不开8种食物', '远方', '保健养生', '@', 3);
*/
	fprintf(fp, "<div class=\"overflow\">\n	<li>\n");
	fprintf(fp, "<a class=\"blu\" title=\"%s\" href=\"/HT/con_%d_M.%d.A.htm\">%s</a>\n", pdigest->title, pdigest->bnum, pdigest->ftime, pdigest->title);
	fprintf(fp, "<<a class=\"blk\" title=\"%s\" href=\"/HT/home?B=%d\">%s</a>>", pdigest->bname, pdigest->bnum, pdigest->bname);
	if(!strchr(pdigest->owner, '.'))
		fprintf(fp,	"[<a class=\"blk\" title=\"%s\" href=\"/HT/qry?U=%s\">%s</a>][%d/%d]</li></div>", pdigest->owner, pdigest->owner, pdigest->owner, pdigest->neweva, pdigest->eva);
	else
		fprintf(fp,	"[%s][%d/%d]</li></div>", pdigest->owner, pdigest->neweva, pdigest->eva);
	return 1;
}

int main () {
	int i, j, size = sizeof(struct digestitem), pos;
	int digesttime, oldbnum, oldftime, newbnum, newftime, eva, hasvoted;
	FILE *fp, *tmpfp, *wwwfp, *jinghuafp, *tempoutfp, *jinghuamorefp;
	char digestboard[STRLEN], digestlog[STRLEN];
	char buf[STRLEN * 3], tmptitle[STRLEN], secstr;
	char title[100], owner[20], *bname, *ptr;
	time_t now = time(NULL);
	int bt_pos = 0, bt_fd, bt_size = 0, bb_bnum[MAXBADBOARDS], retv;
						//for badtitles and bad boards
	char *bt_ptr, *bt_buffer = NULL, *bt_list[MAXBADTITLE];	
						//for badtitles
	char digesttemp[100], *tempptrs, *tempptre;
						//for digest chosen
	FILE *bd_fp;
	float avgeva;
	int lineoftempfile, lineofjinghua;
	struct digestitem *evafile;   //for digest item list

	if (initbbsinfo(&bbsinfo) < 0)
		return -1;
	shm_bcache = bbsinfo.bcacheshm;

	if (!(wwwfp = fopen(OUTPUT, "w")))
		return 0;
	if (!(jinghuafp = fopen(JINGHUA, "w")))
		return 0;
	// this temp out file is for filter and sort
	if (!(tempoutfp = fopen(TEMPOUT, "w")))
		return 0;

	initcolum(wwwfp, jinghuafp);

	show_detail(wwwfp, jinghuafp);

// read bad title file into bt_list[MAXBADTITLE]
	if ((bt_fd = open(FILTERTITLE, O_RDWR, 0600)) != -1) {
		bt_size = file_size(FILTERTITLE);
		bt_buffer = (char *)mmap(0, bt_size, 
				PROT_WRITE | PROT_READ, MAP_PRIVATE, bt_fd, 0);
		close(bt_fd);
		i = 0;
		bzero(bt_list, MAXBADTITLE * sizeof(char *));
		while (bt_buffer && *(bt_buffer + bt_pos) == '\n')
			bt_pos++;
		while (i < MAXBADTITLE && bt_pos < bt_size) {
			bt_list[i] = bt_buffer + bt_pos;
			i++;
			bt_ptr = strchr(bt_buffer + bt_pos, '\n');
			if (bt_ptr) {
				*bt_ptr = 0;
				bt_pos = bt_ptr - bt_buffer + 1;
				while (*(bt_buffer + bt_pos) == '\n')
					bt_pos++;
			} else
				break;
		}
	}

	if (NULL != (bd_fp = fopen(BADBOARDS, "r"))) {
		i = 0;
		while (i < MAXBADBOARDS && fgets(buf, sizeof(buf), bd_fp)) {
			
			bb_bnum[i] = getbnum(buf);
			if (bb_bnum[i] >= 0) 
				i++;
		}
		fclose(bd_fp);
		if (i < MAXBADBOARDS)
			bb_bnum[i] = -1;
	}

	for (i = -1; i < sectree.nsubsec; i++) {
		if (i == -1)
			strcpy(digestboard, "Digest");
		else
			sprintf(digestboard, "%sDigest", 
					sectree.subsec[i]->basestr);
		sprintf(digestlog, MY_BBS_HOME "/boards/%s/.DIGESTLOG", 
				digestboard);
		if ((fp = fopen(digestlog, "r")) == NULL)
			continue;
		sprintf(digestlog, "%s.new", digestlog);
		if ((tmpfp = fopen(digestlog, "w+")) == NULL) {
			fclose(fp);
			continue;
		}
		
		pdigest = (struct digestitem *)malloc(size);
		while (fscanf(fp, "%d\t%d\t%d\t%d\t%d\t%c", &digesttime,
					&oldbnum, &oldftime, &newbnum, 
					&newftime, &secstr) > 0) {
			if (checkold(digesttime, (int)now))
				//看看是否toooooooooooooold了
				continue;
			if (!checkexist(tmpfp, newbnum, oldbnum, 
						oldftime, newbnum, 
						newftime, digesttime, 
						secstr, 1, &eva))
				//检查是否在文摘版及原始
				continue;
			if (!checkmarked(digestboard, newftime, 
						title, owner))	
				//看看是否在文摘版被M
				continue;
			
			retv = 0;
			for (j = 0; j < MAXBADTITLE; j++) {
				if (!bt_list[j])
					break;
				if (strstr(title, bt_list[j])) {
					retv = 1;
					break;
				}
			}
#if 0
			for (j = 0; j < MAXBADBOARDS; j++) {
				if (bb_bnum[j] < 0)
					break;
				if (oldbnum == bb_bnum[j] + 1) {
					retv = 1;
					break;
				}
			}
#endif
			if (retv)
				continue;

			tmptitle[0] = 0;
			encodetitle(tmptitle, title);
			pdigest->bnum = oldbnum - 1;
			pdigest->ftime = oldftime;
			strncpy(pdigest->title, tmptitle, sizeof(pdigest->title));
			strncpy(pdigest->owner, owner, sizeof(pdigest->owner));
			strncpy(pdigest->bname, bbsinfo.bcache[oldbnum - 1].header.title, 
					sizeof (pdigest->bname));
			pdigest->sec = secstr;
			pdigest->eva = eva;
//			write_item(jinghuafp, pdigest);
//  先输入一个临时文件，然后排序，最后生成两个文件，一个给更多链接使用，另一个给首页导读
			fprintf(tempoutfp, "%d, %d, '%s', '%s', '%s', '%c', %d\n", 
				pdigest->bnum, pdigest->ftime, pdigest->title,
				pdigest->owner, pdigest->bname, 
				pdigest->sec, pdigest->eva);
		}
		fclose(tempoutfp);
//  取出tempoutfp，也就是精华文章候选的长度
		tempoutfp = fopen(TEMPOUT, "r");
		if(!tempoutfp) {
			printf("can not find TEMPOUT\n");
			return -1;
		}
		lineoftempfile=0;
		while (fgets(buf, 1024, tempoutfp) != NULL) {
			lineoftempfile++;
		}
		fclose(tempoutfp);
		if (lineoftempfile<1)
		{
			printf("Error file line=0\n");
			return 0;
		}
//从文件里面再把这个列表读出来
		tempoutfp = fopen(TEMPOUT, "r");
		evafile=(struct digestitem *)calloc(lineoftempfile, size);
		pdigest=&evafile[0];

		while (fgets(buf, 1024, tempoutfp) != NULL)
		{
//			sscanf(buf, "%d,%d,\'%s\',\'%s\',\'%s\',\'%c\',%d", &pdigest->bnum, &pdigest->ftime, pdigest->title, pdigest->owner, pdigest->bname, &pdigest->sec, &pdigest->eva);
			tempptre = strchr(buf, ',');
			tempptrs = &buf[0];
			strncpy(digesttemp, tempptrs, tempptre-tempptrs);
			digesttemp[tempptre-tempptrs]='\0';
//			printf("digesttemp1= %s", digesttemp);
			pdigest->bnum = atoi(digesttemp);
			tempptrs = tempptre+1;
			tempptre = strchr(tempptre+1, ',');
			strncpy(digesttemp, tempptrs, tempptre-tempptrs);
			digesttemp[tempptre-tempptrs]='\0';
//			printf("digesttemp2= %s", digesttemp);
			pdigest->ftime = atoi(digesttemp);
			tempptrs = tempptre+1;
			tempptre = strchr(tempptre+1, ',');
			strncpy(digesttemp, tempptrs, tempptre-tempptrs);
			digesttemp[tempptre-tempptrs]='\0';
//			printf("digesttemp3= %s\n", digesttemp);
			delete_redundant_word(digesttemp, "[转寄]");
			delete_redundant_word(digesttemp, "[转载]");
			delete_redundant_word(digesttemp, "[Blog]");
			delete_redundant_word(digesttemp, "\'");
			delete_redundant_word(digesttemp, " ");
//			printf("digesttemp3= %s\n", digesttemp);
			if(sizeof(pdigest->title)<=strlen(digesttemp)+1)  //防止标题越界
			{
//				printf("pdigest->title=%d  strlen(digesttemp)=%d\n", sizeof(pdigest->title), strlen(digesttemp));
				strncpy(pdigest->title, digesttemp, sizeof(pdigest->title)-1);
				pdigest->title[sizeof(pdigest->title)-1]='\0';
			}
			else
			{
//				printf("pdigest->title=%d  strlen(digesttemp)=%d\n", sizeof(pdigest->title), strlen(digesttemp));
				strncpy(pdigest->title, digesttemp, strlen(digesttemp)+1);
			}
			tempptrs = tempptre+1;
			tempptre = strchr(tempptre+1, ',');
			strncpy(digesttemp, tempptrs, tempptre-tempptrs);
			digesttemp[tempptre-tempptrs]='\0';
			delete_redundant_word(digesttemp, "\'");
			delete_redundant_word(digesttemp, " ");
//			printf("digesttemp4= %s", digesttemp);
			strcpy(pdigest->owner , digesttemp);
			tempptrs = tempptre+1;
			tempptre = strchr(tempptre+1, ',');
			strncpy(digesttemp, tempptrs, tempptre-tempptrs);
			digesttemp[tempptre-tempptrs]='\0';
			delete_redundant_word(digesttemp, "\'");
			delete_redundant_word(digesttemp, " ");
//			printf("digesttemp5= %s", digesttemp);
			strcpy(pdigest->bname , digesttemp);
			tempptrs = tempptre+1;
			tempptre = strchr(tempptre+1, ',');
			strncpy(digesttemp, tempptrs, tempptre-tempptrs);
			digesttemp[tempptre-tempptrs]='\0';
//			printf("digesttemp6= %s", digesttemp);
			pdigest->sec=digesttemp[tempptre-tempptrs-2];
			tempptrs = tempptre+1;
			tempptre = strchr(tempptre+1, '\0');
			strncpy(digesttemp, tempptrs, tempptre-tempptrs);
			digesttemp[tempptre-tempptrs]='\0';
//			printf("digesttemp7= %s", digesttemp);
			pdigest->eva = atoi(digesttemp);
			pdigest->neweva = 0;

//			printf("%d | %d | %s | %s | %s | %c | %d \n", pdigest->bnum, pdigest->ftime, pdigest->title, pdigest->owner, pdigest->bname, pdigest->sec, pdigest->eva);
			pdigest++;
		}
		fclose(tempoutfp);
//  对取出来的数组排序
//		printf("sort begin\n");
		qsort(evafile, lineoftempfile, size, cmp_ftime);
//		printf("sort end\n");

		pdigest=&evafile[0];
//最后输出文件
		if (!(jinghuamorefp= fopen(JINGHUAMORE, "w")))
			return 0;
		lineofjinghua = 0;
		srand((int)now);
		for (j = 0; j < lineoftempfile; j++ )
		{
			if (lineofjinghua <= MAXJINGHUA) 
			{
//输出到导读精华推荐
				if( 1/(1+((int)now-pdigest->ftime)/86400.0) > rand()/RAND_MAX)
					write_item(jinghuafp, pdigest);
				lineofjinghua++;
			}
//输出到更多精华
			write_item(jinghuamorefp, pdigest);
			pdigest++;
		}

		free(evafile);

		fclose(jinghuamorefp);
		fclose(tmpfp);
		fclose(fp);
		if (i == -1)
			strncpy(buf, MY_BBS_HOME "/boards/Digest/.DIGESTLOG", 
					sizeof (buf));
		else
			sprintf(buf, MY_BBS_HOME "/boards/%sDigest/.DIGESTLOG", 
					sectree.subsec[i]->basestr);
		rename(digestlog, buf);
	}

	//生成导读 NAVFILE是 navpart.txt，由/bin/nav调用sql生成，导读必须写在Digest前面，以免被Digest重复而删掉

	// this temp out file is for filter and sort
	if (!(tempoutfp = fopen(TEMPOUT, "w")))
		return 0;

	if (NULL != (fp = fopen(NAVFILE, "r"))) {
		while (fgets(buf, sizeof(buf), fp)) {
			avgeva = atof(buf);  //average evaluation should be float
			ptr = strchr(buf, ' ');
			if (!ptr)
				continue;
			*ptr = 0;
			ptr++;
			hasvoted = atoi(ptr);
			ptr = strchr(ptr, ' ');
			if (!ptr)
				continue;
			*ptr = 0;
			ptr++;
			bname = ptr;
			ptr = strchr(ptr, ' ');
			if (!ptr)
				continue;
			*ptr = 0;
			ptr++;
			if (strchr(ptr, ' ')) {
				pos = strchr(ptr, ' ') - ptr;
				*(strchr(ptr, ' ')) = 0;
			}
			else
				continue;
			strncpy(owner, ptr, sizeof(owner));
			ptr += pos + 1;
			newftime = atoi(ptr);
			ptr = strchr(ptr, ' ');
			if (!ptr)
				continue;
			ptr++;
			if (strchr(ptr, '\n'))
				*(strchr(ptr, '\n')) = 0;
			strncpy(title, ptr, sizeof(title));
			newbnum = getbnum(bname);
			if (newbnum < 0)
				continue;
			pdigest->bnum = newbnum;
			pdigest->ftime = newftime;
			encodetitle(tmptitle, title);
			strncpy(pdigest->title, tmptitle, 
					sizeof(pdigest->title));
			strncpy(pdigest->owner, owner, 
					sizeof(pdigest->owner));
			strncpy(pdigest->bname, getbtitle(newbnum), 
					sizeof(pdigest->bname));
			pdigest->sec = '@';
			pdigest->eva = (int)(avgeva * hasvoted + 0.5);  //总评分四舍五入后写入int型的eva中

			retv = 0;
			for (j = 0; j < MAXBADTITLE; j++) {
				if (!bt_list[j])
					break;
				if (strstr(pdigest->title, bt_list[j])) {
					retv = 1;
					break;
				}
			}

			for (j = 0; j < MAXBADBOARDS; j++) {
				if (bb_bnum[j] < 0)
					break;
				if (pdigest->bnum == bb_bnum[j]) {
					retv = 1;
					break;
				}
			}
			if (retv)
				continue;

//			write_item(wwwfp, pdigest);
//  先输入一个临时文件，然后排序，最后生成两个文件，一个给更多链接使用，另一个给首页导读
			fprintf(tempoutfp, "%d, %d, '%s', '%s', '%s', '%c', %d\n", 
				pdigest->bnum, pdigest->ftime, pdigest->title,
				pdigest->owner, pdigest->bname, 
				pdigest->sec, pdigest->eva);

		}
		fclose(tempoutfp);
		fclose(fp);
//  取出tempoutfp，也就是导读文章候选的长度
		tempoutfp = fopen(TEMPOUT, "r");
		if(!tempoutfp) {
			printf("can not find TEMPOUT\n");
			return -1;
		}
		lineoftempfile=0;
		while (fgets(buf, 1024, tempoutfp) != NULL) {
			lineoftempfile++;
		}
		fclose(tempoutfp);
		if (lineoftempfile<1)
		{
			printf("Error file line=0\n");
			return 0;
		}
//从文件里面再把这个列表读出来
		tempoutfp = fopen(TEMPOUT, "r");
		evafile=(struct digestitem *)calloc(lineoftempfile, size);
		pdigest=&evafile[0];

		while (fgets(buf, 1024, tempoutfp) != NULL)
		{
//			sscanf(buf, "%d,%d,\'%s\',\'%s\',\'%s\',\'%c\',%d", &pdigest->bnum, &pdigest->ftime, pdigest->title, pdigest->owner, pdigest->bname, &pdigest->sec, &pdigest->eva);
			tempptre = strchr(buf, ',');
			tempptrs = &buf[0];
			strncpy(digesttemp, tempptrs, tempptre-tempptrs);
			digesttemp[tempptre-tempptrs]='\0';
//			printf("digesttemp1= %s", digesttemp);
			pdigest->bnum = atoi(digesttemp);
			tempptrs = tempptre+1;
			tempptre = strchr(tempptre+1, ',');
			strncpy(digesttemp, tempptrs, tempptre-tempptrs);
			digesttemp[tempptre-tempptrs]='\0';
//			printf("digesttemp2= %s", digesttemp);
			pdigest->ftime = atoi(digesttemp);
			tempptrs = tempptre+1;
			tempptre = strchr(tempptre+1, ',');
			strncpy(digesttemp, tempptrs, tempptre-tempptrs);
			digesttemp[tempptre-tempptrs]='\0';
//			printf("digesttemp3= %s\n", digesttemp);
			delete_redundant_word(digesttemp, "[转寄]");
			delete_redundant_word(digesttemp, "[转载]");
			delete_redundant_word(digesttemp, "[Blog]");
			delete_redundant_word(digesttemp, "\'");
			delete_redundant_word(digesttemp, " ");
			if(sizeof(pdigest->title)<=strlen(digesttemp)+1)  //防止标题越界
			{
//				printf("dao du pdigest->title=%d  strlen(digesttemp)=%d\n", sizeof(pdigest->title), strlen(digesttemp));
				strncpy(pdigest->title, digesttemp, sizeof(pdigest->title)-1);
				pdigest->title[sizeof(pdigest->title)-1]='\0';
			}
			else
			{
//				printf("dao du pdigest->title=%d  strlen(digesttemp)=%d\n", sizeof(pdigest->title), strlen(digesttemp));
				strncpy(pdigest->title, digesttemp, strlen(digesttemp)+1);
			}
			tempptrs = tempptre+1;
			tempptre = strchr(tempptre+1, ',');
			strncpy(digesttemp, tempptrs, tempptre-tempptrs);
			digesttemp[tempptre-tempptrs]='\0';
			delete_redundant_word(digesttemp, "\'");
			delete_redundant_word(digesttemp, " ");
//			printf("digesttemp4= %s", digesttemp);
			strcpy(pdigest->owner , digesttemp);
			tempptrs = tempptre+1;
			tempptre = strchr(tempptre+1, ',');
			strncpy(digesttemp, tempptrs, tempptre-tempptrs);
			digesttemp[tempptre-tempptrs]='\0';
			delete_redundant_word(digesttemp, "\'");
			delete_redundant_word(digesttemp, " ");
//			printf("digesttemp5= %s", digesttemp);
			strcpy(pdigest->bname , digesttemp);
			tempptrs = tempptre+1;
			tempptre = strchr(tempptre+1, ',');
			strncpy(digesttemp, tempptrs, tempptre-tempptrs);
			digesttemp[tempptre-tempptrs]='\0';
//			printf("digesttemp6= %s", digesttemp);
			pdigest->sec=digesttemp[tempptre-tempptrs-2];
			tempptrs = tempptre+1;
			tempptre = strchr(tempptre+1, '\0');
			strncpy(digesttemp, tempptrs, tempptre-tempptrs);
			digesttemp[tempptre-tempptrs]='\0';
//			printf("digesttemp7= %s", digesttemp);
			pdigest->eva = atoi(digesttemp);
//	calculate total evaluation
			pdigest->neweva=(int)(pdigest->eva * 72 / ( 7 + ((int)now - pdigest->ftime) / 3600.0 ));

//			printf("%d | %d | %s | %s | %s | %c | %d | %d\n", pdigest->bnum, pdigest->ftime, pdigest->title, pdigest->owner, pdigest->bname, pdigest->sec, pdigest->eva, pdigest->neweva);
			pdigest++;
		}
		fclose(tempoutfp);
//  对取出来的数组排序
		qsort(evafile, lineoftempfile, size, cmp_neweva);
		pdigest=&evafile[0];
//最后输出文件
		if (!(jinghuamorefp= fopen(DAODUMORE, "w")))
			return 0;
		lineofjinghua = 0;
		srand((int)now);
		for (j = 0; j < lineoftempfile; j++ )
		{
//			printf("%d | %d | %s | %s | %s | %c | %d | %d\n", pdigest->bnum, pdigest->ftime, pdigest->title, pdigest->owner, pdigest->bname, pdigest->sec, pdigest->eva, pdigest->neweva);
			if (lineofjinghua <= MAXDAODU) 
			{
//输出到导读精华推荐
				if( pdigest->neweva>AVGEVA || AVGEVA/(2*AVGEVA-pdigest->neweva) > rand()/RAND_MAX)
					write_item(wwwfp, pdigest);
				lineofjinghua++;
			}
//输出到更多导读
			write_item(jinghuamorefp, pdigest);
			pdigest++;
		}

		free(evafile);

	}

//  for SEO , move show_detail above
//	show_detail(wwwfp);
//	fprintf(wwwfp, "</script>");
//	fprintf(jinghuafp, "</script>");
	fprintf(wwwfp, "</div>");
	fprintf(jinghuafp, "</div>");
	fclose(wwwfp);
	fclose(jinghuafp);
	if (bt_buffer)
		munmap(bt_buffer, bt_size);
	return 1;
}
