#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "bbs.h"

#define SP_DIR	"wwwtmp/special"

//#define DEBUG_MODE

typedef struct AnalysisResult {
	int ns;
	int sid[3];
	int sim[3];
} AnalysisResult; 

typedef struct special {
	int sid;
	char model[80] ;
	char title[80];
} Special;

typedef struct Server {
	char ip[20];
	int port, count;
	int nouse;
} Server;

typedef struct SpecialArticle {
	char title[80];
	char owner[40];
	char hlink[256];
	int filetime;
	int type, size, sim;
	int bidx;
	int attached, re;
} SPA;
	

#define MAX_QLEN 128
#define MAX_SP_NUM 32
#define MAX_SERVER_NUM 8

char queue[MAX_QLEN][512];
int qlen;
Special sp[MAX_SP_NUM];
Server server[MAX_SERVER_NUM]; // must in [1, 127]
int num_server;
time_t server_last_update, special_last_update;

static void disable_server(int ns);
static char * get_from_line(char **line, char **ptr,int c);

int
initAILogMSQ()
{
	int msqid;
	struct msqid_ds buf;
	msqid = msgget(getBBSKey(AID_MSQ), IPC_CREAT | 0664);
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
	static char buf[512];
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

static int 
get_article_info(FILE *fp, struct fileheader *fh, int type, const char *fname) {

	if(type == 1) {
		if(!strncmp(fh->title, "Re: ", 4))
			fh->accessed |= FH_REPLIED;	//借用
	} else if(type == 2) {
		return 0;
	} else if(type == 3) {
		return 0;
	}
	return 0;
}

static int utf82gb_file(const char *fname) {
	int fd;
	char *buf_in, *buf_out, tmpf[256];
	size_t size;
	struct stat st;

	fd = open(fname, O_RDONLY);
	if(fd < 0)
		return -1;
	
	fstat(fd, &st);
	size = st.st_size;
	if(size <= 0) {
		close(fd);
		return -1;
	}
	buf_in = malloc(size+1);
	if(buf_in == NULL) {
		close(fd);
		return -1;
	}
	read(fd, buf_in, size);
	close(fd);
	buf_in[size] = 0;

	buf_out = malloc(size<<4);
	if(buf_out == NULL) {
		free(buf_in);
		return -1;
	}
	sprintf(tmpf, "%s.tmp", fname);
	fd = open(tmpf, O_WRONLY | O_CREAT);
	if(fd < 0) {
		free(buf_out);
		free(buf_in);
		return -1;
	}
	utf82gb(buf_out, size<<4, buf_in);
	write(fd, buf_out, strlen(buf_out));
	close(fd);
	rename(tmpf, fname);
	free(buf_out);
	free(buf_in);
	return 0;
}

/*
static int get_article_type(char *fname) {
	if(!strncasecmp(fname, "boards/", 7))
		return 1;
	if(!strncasecmp(fname, "blog/", 5))
		return 2;
	if(!strncasecmp(fname, "0Announce/", 10))
		return 3;
	return 0;
}
*/	

static int filter_pure_content(char *tmpfile,  struct fileheader *fh, size_t *size, int type) {
	FILE *fp_in, *fp_out;
	char newfn[256], buf[1024];
	int i;

	*size = 0;

	if(type == 2) // Blog的文章是utf-8编码, 先转换一下
		utf82gb_file(tmpfile);
	if(filter_attach(tmpfile)) // 过滤附件
		fh->accessed |=  FH_ATTACHED;
	
	fp_in = fopen(tmpfile, "r");
	if(fp_in == NULL)
		return -1;

	sprintf(newfn, "%s.tmp", tmpfile);
	fp_out = fopen(newfn, "w");
	if(fp_out == NULL) {
		fclose(fp_in);
		return -1;
	}
	get_article_info(fp_in, fh, type, tmpfile);
	for(i=0; fgets(buf, sizeof(buf), fp_in); i++) {
		if(*buf == '\n' || *buf == '\r')
			continue;
		if(i < 3) {
			if(!strncmp("发信人: ", buf, 8) ||
			   !strncmp("标  题: ", buf, 8) ||
			   !strncmp("发信站: ", buf, 8))
				continue;
		}			
		if(!strcmp(buf, "--\n")) //过滤签名档, 并认为内容结束
			break;
		if(*buf == ':' || !strncmp(buf, "【 在 ", 6)) //过滤引文
			continue; 
		fputs(buf, fp_out);
	}
	fclose(fp_in);
	fclose(fp_out);
	*size = file_size(newfn);
	if(*size < 64) {
		unlink(newfn);
		return -1;
	}
	return rename(newfn, tmpfile);
}

static int send_file_sock(int sock, char *buffer, size_t size, int fflag) {
	int ie, left, count, sb, len;

	send(sock, &size, sizeof(size), 0);	//文件大小
	count = 0;
	left = size;
	ie = 0;
	while(count < size && ie < 3) {
		if(left > 4096)
			len = 4096;
		else
			len = left;

		sb = send(sock, buffer+count, len, 0);
		if(sb <= 0) {
			ie++;
			usleep(100);
			continue;
		}
		count+= sb;
		left -= sb;
	}
	return (0-ie);
}

static int write_as(AnalysisResult *as, char *fname, struct fileheader *fh, size_t size, int type) {
	int i;
	char sfile[256], buf[1024];

	for(i=0; i <as->ns; i++) {
		if(as->sid[i] <= 0 || as->sim[i] < 3)
			continue;
		sprintf(sfile, SP_DIR "/%s.txt", sp[(as->sid[i]-1)].title);
		sprintf(buf, "%d\t%s\t%s\t%s\t%d\t%d\t%d\t%d\n", type, fh->title, 
			fname, fh->owner, as->sim[0], 
			(fh->accessed | FH_ATTACHED) ? 1 : 0, 
			(fh->accessed | FH_REPLIED) ? 1 : 0, size);
		f_append(sfile, buf);
	}
	return 0;
}


//子进程函数: 连接超时信号
static void sig_connect(int signo) {
	return ;
}

//子进程函数: 发送AI请求
static int require_ai(int ns) {
	struct sockaddr_in sin;
	int fd, sock, retv, size, type, fflag = 1;
	char *msg, *fpath, *ptr, *buffer;
	int i, fno;
	char tmpfile[256];
	AnalysisResult as;
	struct fileheader fh;

	//printf("try to coonect..\n");
	bzero((char *) &sin, sizeof (sin));
        sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(server[ns].ip);
	sin.sin_port = htons(server[ns].port);
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0)
		return MAX_SERVER_NUM;
	signal(SIGALRM, sig_connect);
	alarm(10);
	if (connect(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0) {
		alarm(0);
		close(sock);
		return ns;
	}
	alarm(0);
	//printf("connected...\n");
	for(i=0, fno = 0; i < qlen; i++) {
		// type path owner title
		msg = queue[i];
		get_from_line(&msg, &ptr, '\t');
		type = atoi(ptr);
		if(!type)
			continue;
		get_from_line(&msg, &fpath, '\t');
		if(fpath == NULL)
			continue;
		bzero(&fh, sizeof(fh));
		get_from_line(&msg, &ptr, '\t');
		strsncpy(fh.owner, ptr, sizeof(fh.owner));
		get_from_line(&msg, &ptr, '\t');
		strsncpy(fh.title, ptr, sizeof(fh.title));
		
		sprintf(tmpfile, "bbstmpfs/tmp/ai.%ld.%d", time(NULL), fno++);
		if(copyfile(fpath, tmpfile) == -1)
			continue;

		//printf("filter file...\n");
		if(filter_pure_content(tmpfile, &fh, &size, type)) 
			goto NEXT_UL;
		//printf("ready to send file...\n");
		fd = open(tmpfile, O_RDONLY);
		if(fd < 0)
			goto NEXT_UL;
		buffer = malloc(size);
		if(buffer == NULL) 
			goto NEXT_FD;		
		read(fd, buffer, size);

		//printf("do send..\n");
		send(sock, &fflag, sizeof(fflag), 0);		//文件类型
		send_file_sock(sock, buffer, size, fflag);	//发送文件

		retv = recv(sock, &as, sizeof(as), 0);	//分析结果
		if(retv == sizeof(as))
			write_as(&as, fpath, &fh, size, type);
		//sprintf(buf, "ai recved @ %ld  ns=%d\n", time(NULL), as.ns);
		//f_append(MY_BBS_HOME "/deverrlog", buf);
		//printf("result recved...\n");
		free(buffer);
	NEXT_FD:
		close(fd);
	NEXT_UL:
		unlink(tmpfile);
	}
	//printf("end session\n");
	fflag = -1;
	send(sock, &fflag, sizeof(fflag), 0);	// end this connection
	close(sock);
	return MAX_SERVER_NUM;
}

//子进程函数: 广播专题定义文件
static int send_special() {
	struct sockaddr_in sin;
	int i, fd, sock, size, rt;
	int ns = MAX_SERVER_NUM, fflag = 0, endflag = -1;
	char *buffer;
	struct stat st;

	if(num_server == 0)
		return ns;

	//Read file to buffer
	fd = open("0Announce/special", O_RDONLY);
	if(fd < 0)
		return ns;
	fstat(fd, &st);
	size = st.st_size;
	if(size <= 0) {
		close(fd);
		return ns;
	}
		
	buffer = malloc(size);
	if(buffer == NULL) {
		close(fd);
		return ns;
	}
	read(fd, buffer, size);
	close(fd);
	
	signal(SIGALRM, sig_connect);

	bzero((char *) &sin, sizeof (sin));
        sin.sin_family = AF_INET;
	for(i=0; i < num_server; i++) {
		sin.sin_addr.s_addr = inet_addr(server[i].ip);
		sin.sin_port = htons(server[i].port);
	
		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(sock < 0)
			continue;
		alarm(10); // 设置超时信号
		if (connect(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0) {
			alarm(0);
			ns = i;
			close(sock);
			continue;
		}
		alarm(0); //Cancel alarm
		
		send(sock, &fflag, sizeof(fflag), 0);		//文件类型
		send_file_sock(sock, buffer, size, fflag);      //发送文件
		recv(sock, &rt, sizeof(rt), 0);			//同步
		send(sock, &endflag, sizeof(endflag), 0);	//结束标志
		close(sock);
	}
	free(buffer);
	return ns;
}

// -1 means error and special set to 0
// 0 means no change and special kept
// 1 means special modified and reload ok
static int load_special() {
	FILE *fp;
	char line[1024], *ptr;
	int i, ft, in=0;
	
	ft = file_size(MY_BBS_HOME "/0Announce/special");
	if(ft <= special_last_update)
		return 0;

	special_last_update = ft;

	bzero(sp, sizeof(sp));
	fp = fopen(MY_BBS_HOME "/0Announce/special", "r");
	if(fp == NULL)
		return -1;

	for(i=0; fgets(line, sizeof(line), fp) && i < MAX_SP_NUM; ) {
		if(*line == '#')
			continue;
		ptr = strpbrk(line, "\r\n");
		if(ptr)
			*ptr = 0;

		if(in && *line == '\0') {
			in = 0;
			i++;
		} else if(!in && !strncasecmp("title=", line, 6)) {
			sp[i].sid = i+1;
			snprintf(sp[i].title, sizeof(sp[i].title)-1, "%s", line+6);
			in = 1;
		} else if(in && !strncasecmp("model=", line, 6)) {
			snprintf(sp[i].model, sizeof(sp[i].model)-1, "%s", line+6);
		}
	}	
	fclose(fp);
	return 1;
}

static char * get_from_line(char **line, char **ptr,int c) {
	char *end;

	if(line == NULL)
		return NULL;
	if(*line == NULL)
		return NULL;
	*ptr = *line;
	end = strchr(*ptr, c);
	if(!end) {
		return *ptr;
	}
	*end = 0;
	*line = end+1;
	return *ptr;
}


static int  parse_txtfile_line(char *line, SPA *spa) {
	int type;
	char *ptr;
	char *path, *bname, *fname;

	bzero(spa, sizeof(SPA));

	ptr = get_from_line(&line, &ptr, '\t');
	type = atoi(ptr);
	if(!type) //未知的文章类型
		return -1;
	
	ptr = get_from_line(&line, &ptr, '\t');
	strsncpy(spa->title, ptr, sizeof(spa->title)); //文章标题
	//printf("title:%s\t", title);
	
	ptr = get_from_line(&line, &ptr, '\t');
	path = ptr; // 文章路径
	//printf("path:%s\t", path);

	ptr = get_from_line(&line, &ptr, '\t');
	strsncpy(spa->owner, ptr, sizeof(spa->owner)); // 用户名
	//printf("%s\t", userid);
	
	if(type == 1) {
		// boards/sysop/M.1122334455.A
		// con?B=sysop&F=M.1122334455.A
		get_from_line(&path, &ptr,  '/');
		get_from_line(&path, &ptr, '/');
		bname = ptr; //版名
		get_from_line(&path, &ptr, '/');
		fname = ptr; //文件名
		if(!fname)
			return -1;
		snprintf(spa->hlink, sizeof(spa->hlink), "con?B=%s&F=%s", bname, fname);
#ifdef ENABLE_BLOG
	} else if(type == 2) {
		// blog/E/赤名莉香/1122334455
		// blogread?U=%s&T=1122334455
		fname = strrchr(path, '/');
		if(!fname)
			return -1;
		snprintf(spa->hlink, sizeof(spa->hlink), "blogread?U=%s&T=%s", spa->owner, ++fname);
#endif
	}
	if(type == 3) {
		// 0Announce/gourps/GROUP_0/sysop/M1122334455/M1122334456... \t userid
		// bbsanc?path=/groups/...&item=M1122334456
		get_from_line(&path, &ptr, '/');
		fname = strrchr(path, '/');
		if(!fname)
			return -1;
		*fname = 0;
		snprintf(spa->hlink, sizeof(spa->hlink), "bbsanc?path=/%s&item=/%s", path, ++fname);
		return 0;
	}
	return 0;
}

static int update_special_html() {
	FILE *fp_in, *fp_out;
	int i, in;
	char txtname[256], tmpname[256], htmname[256], line[1024];
	char *ptr;
	SPA spa;

	// --------- 专题列表 -------------
	fp_in = fopen("0Announce/special", "r");
	if(fp_in == NULL)
		return -1;
	fp_out = fopen(SP_DIR "/special.html.tmp", "w");
	if(fp_out == NULL) {
		fclose(fp_in);
		return -1;
	}
//	fprintf(fp_out, "<table border=0><tr>\n");
	in = 0;
	for(i=0; fgets(line, sizeof(line), fp_in); ) {
		if(*line == '#')
			continue;
		ptr = strpbrk(line, "\r\n");
		if(ptr)
			*ptr = 0;
		
		if(in && *line == '\0') {
			in = 0;
		} else if(!in && !strncasecmp("title=", line, 6)) {
			fprintf(fp_out, "<li><a href=special?s=%s>%s</a></li>\n",
				line+6, line+6);
			i++;
		}
	}
//	fprintf(fp_out, "</tr></table>");
	fclose(fp_in);
	fclose(fp_out);
	rename(SP_DIR "/special.html.tmp", SP_DIR "/special.html");

	// ------------ 每一个专题的页面 ---------------
	for(i=0; i < MAX_SP_NUM && sp[i].sid; i++) {
		sprintf(txtname, SP_DIR "/%s.txt", sp[i].title);
		fp_in = fopen(txtname, "r");
		if(fp_in == NULL)
			continue;
		sprintf(tmpname, SP_DIR "/%s.htm.bak", sp[i].title);
		fp_out = fopen(tmpname, "w");
		if(fp_out == NULL) {
			fclose(fp_in);
			continue;
		}
		fprintf(fp_out, "<h2><center>%s</center></h2>\n", sp[i].title);
	
		while(fgets(line, sizeof(line), fp_in)) {
			ptr = strpbrk(line, "\r\n");
			if(ptr)
				*ptr = 0;
			if(parse_txtfile_line(line, &spa) != 0) {
				//printf("\nparse error!\n");
				continue;
			}
			fprintf(fp_out, "<li><a href=%s>%s</a>&nbsp;&nbsp;"
				"<a href=qry?U=%s>%s</li>\n",
				spa.hlink, spa.title, spa.owner, spa.owner);
		}
		fclose(fp_in);
		fclose(fp_out);
		sprintf(htmname, SP_DIR "/%s.htm", sp[i].title);
		rename(tmpname, htmname);
	}
	return 0;
}			

static int load_server() {
	FILE *fp;
	char line[256], *start, *end;
	int ft;

	ft = file_time("AI_SERVER");
	if(ft <= server_last_update) {
		if(random() % (num_server+1))
			return 0;
	} else
		server_last_update = ft;

	num_server = 0;
	fp = fopen("AI_SERVER", "r");	// 包含删除文件就禁用server的意思
	if(fp == NULL)
		return -1;
	
	while((num_server<MAX_SERVER_NUM) && fgets(line, sizeof(line), fp)) {
		if(*line == '#')
			continue;
		start = line;
		end = strpbrk(start, "\r\n");
		if(end)
			*end = 0;
		end = strchr(start, ':');
		if(!end) {
			server[num_server].port = 9999;
		} else {
			*end = 0;
			server[num_server].port = atoi(end+1);
		}	
		strsncpy(server[num_server].ip, start, sizeof(server[num_server].ip));
		server[num_server].count = 0;
		//printf("%s %d\n", server[num_server].ip, server[num_server].port);  
		num_server++;
	}
	fclose(fp);
	return 1;
}

static void disable_server(int ns) {
	int i;

	if(ns < 0 || ns >= MAX_SERVER_NUM)
		return;
	if(++(server[ns].count) < 3)
		return;

	for(i=ns; i+1 < num_server; i++) {
		memcpy(&(server[i]), &(server[i+1]), sizeof(Server));
	}
	if(num_server > 0) {
		bzero(&(server[num_server-1]), sizeof(Server));
		num_server--;
	}
}

static void sig_hup(int signo) {
	return;
}

// 父进程处理子进程的返回. 子进程返回失效的server
static void sig_chld(int signo) {
	pid_t pid;
	int stat, ns;

	while((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
		if(WIFEXITED(stat)) {
			ns = WEXITSTATUS(stat);
			if(ns != MAX_SERVER_NUM)
				disable_server(ns);
		}
	}		
	return;
}

static void sig_term(int signo) {
	if(num_server && qlen)
		require_ai(0);
	exit(0);
}
	


int
main()
{
	char *str;
	int i = 0, fd = -1, msqid = -1;
	int sp_retv, sv_retv;
	
	srandom(time(NULL));
	chdir(MY_BBS_HOME);
	mkdir(SP_DIR, 0770);
	umask(027);

	msqid = initAILogMSQ();
	if (msqid < 0)
		return -1;
#if !defined(DEBUG_MODE)
	if (fork())
		return 0;
	setsid();
	if (fork())
		return 0;
	close(0);
	close(1);
	close(2);
#endif
	signal(SIGHUP, sig_hup);
	signal(SIGTERM, sig_term);
	signal(SIGCHLD, sig_chld);
	fd = open(MY_BBS_HOME "/reclog/AId.lock", O_CREAT | O_RDONLY,
		  0660);
	if (flock(fd, LOCK_EX | LOCK_NB) < 0)
		return -1;
#ifdef DEBUG_MODE
	printf("msq started...\n");
#endif
	while (1) {
		if(i % 128 == 0) {
			sv_retv = load_server();
			sp_retv = load_special();
			if(fork() == 0) {
				close(fd);
				update_special_html();
				if(sp_retv == 1)
					return send_special();
				else
					return MAX_SERVER_NUM;
			}
		}

		while ((str = rcvlog(msqid, 1))) {
			strsncpy(queue[qlen++], str, 512);
#ifdef DEBUG_MODE
			printf("recved@%ld |%s|\n", time(NULL), str);
			if(qlen < 1)
#else
			if(qlen < 16)
#endif
				continue;
			if(num_server == 0) {
				if(qlen > MAX_QLEN - 1)
					qlen--;//drop msg
				break;
			}
			if(fork() == 0) {
				close(fd);
				return require_ai(i%num_server);
			}
			qlen=0;
			break;
		}
		i++;
		sleep(5);
	}
}

