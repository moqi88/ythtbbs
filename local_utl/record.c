/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    Firebird Bulletin Board System
    Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
                        Peng Piaw Foong, ppfoong@csie.ncu.edu.tw
    
    Copyright (C) 1999, KCN,Zhou Lin, kcn@cic.tsinghua.edu.cn

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include "bbs.h"
#include "ythtbbs.h"
#include "record.h"

#define BUFSIZE (8192)

const struct ANSI2HTML_UBB ubb2[] = {
	{"30", "<font class=c30>", 16, "[color=white]"},
	{"31", "<font class=c31>", 16, "[color=red]"},
	{"32", "<font class=c32>", 16, "[color=green]"},
	{"33", "<font class=c33>", 16, "[color=yellow]"},
	{"34", "<font class=c34>", 16, "[color=blue]"},
	{"35", "<font class=c35>", 16, "[color=purple]"},
	{"36", "<font class=c36>", 16, "[color=cyan]"},
	{"37", "<font class=c37>", 16, "[color=black]"},
	{"40", "<font class=b40>", 16, "[bcolor=white]"},
	{"41", "<font class=b41>", 16, "[bcolor=red]"},
	{"42", "<font class=b42>", 16, "[bcolor=green]"},
	{"43", "<font class=b43>", 16, "[bcolor=yellow]"},
	{"44", "<font class=b44>", 16, "[bcolor=blue]"},
	{"45", "<font class=b45>", 16, "[bcolor=purple]"},
	{"46", "<font class=b46>", 16, "[bcolor=cyan]"},
	{"47", "<font class=b47>", 16, "[bcolor=black]"}
};

long
get_num_records(filename, size)
char *filename;
int size;
{
	struct stat st;

	if (stat(filename, &st) == -1)
		return 0;
	return (st.st_size / size);
}

void
toobigmesg()
{
/*
    prints( "record size too big!!\n" );
    oflush();
*/
}

int
get_record(filename, rptr, size, id)
char *filename;
void *rptr;
int size, id;
{
	int fd;

	if ((fd = open(filename, O_RDONLY, 0)) == -1)
		return -2;
	if (lseek(fd, size * (id - 1), SEEK_SET) == -1) {
		close(fd);
		return -3;
	}
	if (read(fd, rptr, size) != size) {
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int
get_records(filename, rptr, size, id, number)
char *filename;
void *rptr;
int size, id, number;
{
	int fd;
	int n;

	if ((fd = open(filename, O_RDONLY, 0)) == -1)
		return -1;
	if (lseek(fd, size * (id - 1), SEEK_SET) == -1) {
		close(fd);
		return 0;
	}
	if ((n = read(fd, rptr, size * number)) == -1) {
		close(fd);
		return -1;
	}
	close(fd);
	return (n / size);
}

int
substitute_record(filename, rptr, size, id)
char *filename;
void *rptr;
int size, id;
{
#ifdef LINUX
	struct flock ldata;
	int retval;
#endif
	int fd;
	if ((fd = open(filename, O_WRONLY | O_CREAT, 0660)) == -1)
		return -1;
#ifdef LINUX
	ldata.l_type = F_WRLCK;
	ldata.l_whence = 0;
	ldata.l_len = size;
	ldata.l_start = size * (id - 1);
	if ((retval = fcntl(fd, F_SETLKW, &ldata)) == -1) {
		return -1;
	}
#else
	flock(fd, LOCK_EX);
#endif
	if (lseek(fd, size * (id - 1), SEEK_SET) == -1) 
		return -1;
	if (safewrite(fd, rptr, size) != size)
		return -1;
#ifdef LINUX
	ldata.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &ldata);
#else
	flock(fd, LOCK_UN);
#endif
	close(fd);
	return 0;
}

int
update_file(dirname, size, ent, filecheck, fileupdate)
char *dirname;
int size, ent;
int (*filecheck) ();
void (*fileupdate) ();
{
	char abuf[BUFSIZE];
	int fd;

	if (size > BUFSIZE) {
		toobigmesg();
		return -1;
	}
	if ((fd = open(dirname, O_RDWR)) == -1)
		return -1;
	flock(fd, LOCK_EX);
	if (lseek(fd, size * (ent - 1), SEEK_SET) != -1) {
		if (read(fd, abuf, size) == size)
			if ((*filecheck) (abuf)) {
				lseek(fd, -size, SEEK_CUR);
				(*fileupdate) (abuf);
				if (safewrite(fd, abuf, size) != size) {
					flock(fd, LOCK_UN);
					close(fd);
					return -1;
				}
				flock(fd, LOCK_UN);
				close(fd);
				return 0;
			}
	}
	lseek(fd, 0, SEEK_SET);
	while (read(fd, abuf, size) == size) {
		if ((*filecheck) (abuf)) {
			lseek(fd, -size, SEEK_CUR);
			(*fileupdate) (abuf);
			if (safewrite(fd, abuf, size) != size) {
				flock(fd, LOCK_UN);
				close(fd);
				return -1;
			}
			flock(fd, LOCK_UN);
			close(fd);
			return 0;
		}
	}
	flock(fd, LOCK_UN);
	close(fd);
	return -1;
}

unsigned int separateattachment(char* filepath, unsigned int currentpos, struct attachmentstruct *single_attach )
{
	short int attachnum, i, ispicture;
	// attachnum 这个数值应该需要返回，因为会写入forum_post表
	unsigned int attachlen; // 附件大小
    struct mmapfile mf = { ptr:NULL };
    char buf[512];
    short int len;
	char *ptr, *str; // 用于处理附件
	int size;
	char *ext;  // 附件的扩展名
	char extname[10];
	char attachname[256]; //附件显示在帖子里的名称
	FILE *attachfp;  // 附件文件指针
	char attachfilepath[1024]; //附件目录
	char attachfilename[1024]; //附件在文件系统中的真实名称
	char attachfilefullpath[1024]; //附件加文件目录
	char yearmonthdate1[7];  // current year, month
	char yearmonthdate2[3];  // current date
	char hourminsec[7];  //current hour, minute and second
	char randomnumalpha16[17]; // save 16-length random number+lower-case alphabet string
	time_t now;
	struct tm time_p = {0};
	unsigned int total_attachlen;  // total size occupied by attachments
	char attachfiledbpath[1024]; //附件写入数据库中的目录
	char attachfiledbfullpath[1024]; //附件加文件写入数据库中的位置

	printf("It has attachment\n");
	attachnum = 0;
	if (mmapfile(filepath, &mf) < 0)
	{
		printf("mmapfile failed in %s\n", filepath);
	}

	str = mf.ptr + currentpos;
	size = mf.size;   // 指向文件映像内存的指针，以及原始文件的大小，用于打印与控制指针位置

	size = size - currentpos;
//	printf("currentpos=%d  size=%d\n", currentpos, size);
	total_attachlen = 0;
	while (size > 18)
	{
		// 先步进到第一个换行处
		ptr = str;
		len = min(size, sizeof(buf)-1);
		str = memchr(ptr, '\n', len);

		if (!str)
			str = ptr + len;
		else {
			str++;
			len = str - ptr;
		}
		size -= len;

		// 如果是附件
		if (len > 18 && !strncmp(ptr, "beginbinaryattach ", 18)) {
			strsncpy(buf, ptr, len); // 将本行存入buf字符串内
			ptr = strchr(buf, ' ');  // 查找第一个空格出现的位置，后面那个到换行符为止就是attachname
			strcpy(attachname, ptr);
			// 查找出附件类型，判别是否图片，以及图片类型
			printf("Attachname: %s\n", attachname);
			ispicture = 0;
			ext = strrchr(attachname, '.');
			if (ext != NULL) {
				if (!strcasecmp(ext, ".bmp") || !strcasecmp(ext, ".jpg")
				    || !strcasecmp(ext, ".gif") || !strcasecmp(ext, ".jpeg")
				    || !strcasecmp(ext, ".png"))
				{
					ispicture = 1;
				}
				else
				{
					ispicture = 0;
				}
			}
			i = 0;
			while((*(ext+i)!='\n') &&(*(ext+i)!='\0')&& (i<9))
			{
				extname[i] = *(ext+i);
//				printf("extname[%d] = %c\n", i, extname[i]);
				i++;
			}
			extname[i] = '\0';
			printf("extname=%s\n", extname);

			// 步进到第二个换行处
			ptr = str;
			// str指针指向附件的开头，在此处处理
			// 存成文件，记下链接，然后写入附件表
			attachlen = ntohl(*(unsigned int *) (ptr + 1));
			// 把字符串写成文件，从ptr+1一直写attachlen个字符，到行尾
			printf("attachlen=%d\n", attachlen);
			time(&now);
			localtime_r(&now, &time_p);
			strftime(yearmonthdate1, 7, "%Y%m", &time_p);
			strftime(yearmonthdate2, 3, "%d", &time_p);
			strftime(hourminsec, 7, "%H%M%S", &time_p);
//			strftime(yearmonthdate1, 7, "%Y%m", localtime(&now));
//			strftime(yearmonthdate2, 3, "%d", localtime(&now));
//			strftime(hourminsec, 7, "%H%M%S", localtime(&now));
			//testend
			printf("%s\n",yearmonthdate1);
			sprintf(attachfilepath, "/var/www/dx/data/attachment/forum/%s", yearmonthdate1);
			sprintf(attachfiledbpath, "%s", yearmonthdate1);
			printf("attachfilepath here=%s\n", attachfilepath);
			if(access(attachfilepath,0)) // 判断目录是否存在 0也就是F_OK表示check existence
			{
				printf("directory %s does not exist!\n", attachfilepath);
				// 创建相应的目录
				mkdir(attachfilepath, 0700);
			}
			printf("successfully here\n");
			sprintf(attachfilepath, "%s/%s", attachfilepath, yearmonthdate2);
			sprintf(attachfiledbpath, "%s/%s", attachfiledbpath, yearmonthdate2);
			if(access(attachfilepath,0)) // 日期的目录是否存在
			{
				printf("directory %s does not exist!\n", attachfilepath);
				// 创建相应的目录
				mkdir(attachfilepath, 0700);
			}
			//			printf("attachfilepath: %s\n", attachfilepath);
			//			printf("hourminsec: %s\n", hourminsec);
			randomnumalpha(randomnumalpha16, 16); // generate a string contains 16 random number or lower-case alphabets
			sprintf(attachfilename, "%s%s%s", hourminsec, randomnumalpha16, extname);
			sprintf(attachfilefullpath, "%s/%s", attachfilepath, attachfilename);
			sprintf(attachfiledbfullpath, "%s/%s", attachfiledbpath, attachfilename);
			printf("attachfilename: %s\n", attachfilename);
			printf("attachfilefullpath: %s\n", attachfilefullpath);
//			printf("attachlen=%d\n", attachlen);

			// 将附件存储为文件
			attachfp = fopen(attachfilefullpath, "wb");  //这是一个二进制文件
			ptr = ptr + 5;
			fwrite(ptr, 1, attachlen, attachfp);
			fclose(attachfp);

			// 传回写入附件数据表的结构体
//			single_attach->ano = attachnum;
			single_attach->dateline = now;
			strcpy(single_attach->filename, attachname);
			strcpy(single_attach->attachment, attachfiledbfullpath);
			single_attach->filesize = attachlen;
			if(ispicture == 1)
			{
				single_attach->isimage = 1;
				single_attach->width = 640;
			}
			else
			{
				single_attach->isimage = 0;
				single_attach->width = 0;
			}

			str = str + attachlen +5;
			size = size - attachlen - 5;
//			total_attachlen = total_attachlen + single_attach->filesize + 5;
			total_attachlen = total_attachlen + attachlen + 5;
			break;
		}//end 如果是附件
	}
//	printf("successfully extract file!!!\n");
	return total_attachlen;
//	exit(0);  // for test
}

int ythtfiletodiscuzxubb(char* filepath, char* ubbresult, char* useip, struct attachmentstruct *attinfo)
{
	// Drop Article Head
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int i, m;
	char ch;
	char *newline = NULL;
	char ansibuf[80], ubbbuf[80];
	char errormsg[256];
	char *tmp;
	char ubbcontrolstack[20];  // ubb控制符右侧闭环都是由\033[m控制，因此需要考虑匹配关系
	// 'u' 为underline 'c' 为color 'b'为 bgcolor
	short unsigned int ubbcontrolptr;  //指向上个stack的指针
	int ansinum;
	short int color, underline, bgcolor, bold, italic;
	short int remove_telnet_head, ipplace, islastline;
	short int attnum; // attachment number of this post
	unsigned int row_len;
	unsigned int current_position; // current position of the file
	unsigned int total_attachlen;  // total attachment length in single attachment reading

	newline = (char*) malloc(100000);
	fp = fopen(filepath,"r");
	if(!fp)
		return -1;
	/* getline()是gcc扩展，ssize_t getline(char **lineptr, size_t *n, FILE *stream);
	 其中ssize_t是int，size_t是long unsigned int
	 line不用malloc，函数会自动realloc */
	ubbcontrolptr = 0;
	color = 0; underline = 0; bgcolor = 0; bold = 0; italic = 0;
	remove_telnet_head = 0;
	islastline = 0;
	current_position = 0;
	attnum = 0;
	while((read = getline(&line, &len, fp))>0)
	{
		// transfer ansi to UBB or attachment
		// 提取字符串长度
//		printf("UP current_position=%d\n", current_position);
		row_len = strlen(line);
//		printf("row_len=%d,\n line: %s", row_len, line);  // for test
		if(islastline == 1)  // if wrongly recognize last line
		{
			islastline = 0;
			strcat(ubbresult, newline);
		}
		newline[0] = '\0';
		if(remove_telnet_head == 0)
		{
			// remove telnet file head
			if(strstr(line, "发信站: ")!= NULL)
			{
				remove_telnet_head = 1;
				sprintf(newline, "本文通过一路BBS站telnet客户端发布\n");
				strcat(ubbresult, newline);
			}
			continue;
		}

		current_position = current_position + row_len;
		// 判断附件
		if (row_len > 10 && !strncmp(line, "begin 644 ", 10)) {
			// 处理老式图片附件，应该是不存在的，如果存在就报错
			sprintf(errormsg, "old attach found! in %s\n", filepath);
			writesyslog(ERRORLOG, errormsg);
			continue;
		}
		if (row_len > 18 && !strncmp(line, "beginbinaryattach ", 18)) {
			// 新图片附件或其他类型附件
			// beginbinaryattach 后面应该放着文件名，用一个字符串函数取出来即可
			// 下一行开始是附件
			printf("attnum=%d\n", attnum);
			total_attachlen = separateattachment(filepath, current_position, (attinfo+attnum));
			(attinfo+attnum)->ano = attnum;
			attnum++;
			if(attnum>50)
			{
				sprintf(errormsg, "attachnum > 50 %s\n", filepath);
				writesyslog(ERRORLOG, errormsg);
				exit(1);
			}
			current_position = current_position + total_attachlen;
			fseek(fp, total_attachlen, SEEK_CUR);
//			fseek(fp, current_position, SEEK_SET);
		}

		for(i = 0; i< read; i++)
		{
			ch = *(line+i);
			/* 从目前的情况看来，discuzx并不支持类似[nobbc]之类的标签，也不支持复杂标签，可能是因为排版上的问题
			 这样就造成所有用方括号括入的文字会被强制转换，暂时没有办法排除。只能让用户使用[code][/code]规避
			 */
			if(ch == '\033')  //telnet下使用的ANSI码由\ 033开始，都是八进制表示
			{
				// 这里要注意的，一个是telnet下有黑白反向的问题，另一个是闭环的问题，telnet下变色一般不闭环
				if (*(line + i + 1) != '[') {  //不是ANSI控制符的话不理会
					sprintf(newline, "%s%c", newline, ch);
					continue;
				}
				for (m = i + 2; (m < read) && (m < i + 24); m++)
					if (strchr("0123456789;", *(line+m)) == 0)
						break;
				strsncpy(ansibuf, line + i + 2, m - (i + 2) + 1);
				if (*(line + m) != 'm') { //再看一下，是不是ANSI控制符
					sprintf(newline, "%s%c", newline, ch);
					continue;
				}
//				printf("ansibuf = %s\n", ansibuf);
				if(sizeof(ansibuf) == 0)
				{
					ansinum = 0;
				}
				else
				{
					if(strstr(ansibuf, ";"))
					{
						tmp = strtok(ansibuf, ";");
						if(tmp != NULL)
						{
							tmp = strtok(NULL, ";");
							ansinum = atoi(tmp);
						}
						else
							ansinum = 0;
					}
					else
					{
						ansinum = atoi(ansibuf);
//						printf("ansinum = %d\n", ansinum);
					}
				}
//				printf("ansinum=%d\n", ansinum);
				i = m; //调整指针位置，越过整个ANSI控制符
				if(ansinum == 101)
				{
					if(bold == 0)
					{
						sprintf(ubbbuf, "%s", "[b]");
						bold = 1;
						ubbcontrolstack[ubbcontrolptr]='o'; //bold
						ubbcontrolptr++;
					}
					else
					{
						sprintf(ubbbuf, "%s", ""); // ignore
					}
				}
				else if(ansinum == 102)
				{
					if(bold == 1)
					{
						sprintf(ubbbuf, "%s", "[/b]");
						if(ubbcontrolstack[ubbcontrolptr - 1] == 'o')
						{
							ubbcontrolptr--;
							bold = 0;
						}
					}
					else
					{
						sprintf(ubbbuf, "%s", ""); // ignore
					}
				}
				else if(ansinum == 111)
				{
					if(italic == 0)
					{
						sprintf(ubbbuf, "%s", "[i]");
						italic = 1;
						ubbcontrolstack[ubbcontrolptr]='i'; //italic
						ubbcontrolptr++;
					}
					else
					{
						sprintf(ubbbuf, "%s", ""); // ignore
					}
				}
				else if(ansinum == 112)
				{
					if(italic == 1)
					{
						sprintf(ubbbuf, "%s", "[/i]");
						if(ubbcontrolstack[ubbcontrolptr - 1] == 'i')
						{
							ubbcontrolptr--;
							italic = 0;
						}
					}
					else
					{
						sprintf(ubbbuf, "%s", ""); // ignore
					}
				}
				else if(ansinum == 4)
				{
					if (underline == 0)
					{
						underline = 1;
						sprintf(ubbbuf, "%s", "[u]");
						ubbcontrolstack[ubbcontrolptr]= 'u'; //underline
						ubbcontrolptr++;
					}
					else
					{
						sprintf(ubbbuf, "%s", ""); // ignore
					}
				}
				else if(ansinum >= 30 && ansinum <= 37)
				{
					if (color == 0)
					{
						color = 1;
						sprintf(ubbbuf, "%s", ubb2[ansinum-30].ubb);
						ubbcontrolstack[ubbcontrolptr]= 'c'; //color
						ubbcontrolptr++;
					}
					else
					{
						sprintf(ubbbuf, "[/color]%s", ubb2[ansinum-30].ubb);
					}
				}
				else if(ansinum >=40 && ansinum <= 47)
				{
					if (bgcolor == 0)
					{
						bgcolor = 1;
						sprintf(ubbbuf, "%s", ""); // discuz不支持背景色，可以忽略，但因为要右闭环匹配，所以得记
						ubbcontrolstack[ubbcontrolptr]= 'b'; //bgcolor
						ubbcontrolptr++;
					}
					else
					{
						sprintf(ubbbuf, "%s", ""); // ignore
					}
				}
				else if(ansinum == 0)
				{
					if (ubbcontrolptr <= 0)
					{
						// 无左括弧情况，重置
						sprintf(ubbbuf, "%s", ""); // ignore
						ubbcontrolptr = 0;
						underline = 0;
						color = 0;
						bgcolor = 0;
					}
					else
					{
						if(ubbcontrolstack[ubbcontrolptr - 1] == 'b')  //bgcolor
						{
							sprintf(ubbbuf, "%s", "");
							ubbcontrolptr--;
							bgcolor = 0;
						}
						else if(ubbcontrolstack[ubbcontrolptr - 1] == 'c')  //color
						{
							sprintf(ubbbuf, "%s", "[/color]");
							ubbcontrolptr--;
							color = 0;
						}
						else if(ubbcontrolstack[ubbcontrolptr - 1] == 'u') //underline
						{
							sprintf(ubbbuf, "%s", "[/u]");
							ubbcontrolptr--;
							underline = 0;
						}
						else if(ubbcontrolstack[ubbcontrolptr - 1] == 'o')  //bold
						{
							sprintf(ubbbuf, "%s", "[/b]");
							ubbcontrolptr--;
							bold = 0;
						}
						else if(ubbcontrolstack[ubbcontrolptr - 1] == 'i')  //italic
						{
							sprintf(ubbbuf, "%s", "[/i]");
							ubbcontrolptr--;
							italic = 0;
						}
						else
						{
							sprintf(ubbbuf, "%s", "");
							ubbcontrolptr--;
						}
					}
				}
				else
				{
					sprintf(ubbbuf, "%s", ""); // ignore
				}
				strcat(newline, ubbbuf);
			}
			else
			{
				sprintf(newline, "%s%c", newline, ch);
			}
		}
//		printf("newline: %s\n", newline);  // for test
//		printf("newline length = %d\n", strlen(newline));  //for test
		// delete the last line
		if(strstr(newline , "※ 来源:．"))
			islastline = 1;
		else if(strstr(newline, "beginbinaryattach"))
			; // do nothing
		else
			strcat(ubbresult, newline);
	}
	if(strstr(newline, "※ 来源:．")&&(!strstr(newline, "匿名")))
	{
//		printf("newline: %s\n", newline);  // for test
		tmp = strstr(newline, "[FROM: ");
		ipplace = 0;
		useip[0]='\0';
		while(*tmp != ']' && *tmp!='\0')
		{
			if(ipplace == 1)
			{
				if(*tmp!= ' ')
					sprintf(useip, "%s%c",useip,*tmp);
			}
			if(*tmp == ':')
				ipplace = 1;
			tmp++;
		}
	}
	else
	{
		strcpy(useip, "0.0.0.0");
	}
	(attinfo+attnum)->ano = -1;
//	printf("%s\n", ubbresult);  // for testing
	printf("useip: %s\n", useip);
	fclose(fp);
	if(line)
		free(line);
	if(newline)
		free(newline);
	return 0;
}

