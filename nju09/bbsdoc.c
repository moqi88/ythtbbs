#include "bbslib.h"
char *size_str(int size);

void
printdocform(char *cginame, int bnum)
{
	printf("<script>docform('%s','%d');</script>", cginame, bnum);
}

void
noreadperm(char *board, char *cginame)
{
	printf("该讨论区可能是一个俱乐部版面，请向版主提出加入申请<p>");
	printf("<p><a href=javascript:history.go(-1)>快速返回</a>");
	http_quit();
}

//把lepton的这个代码写成一个函数
void
nosuchboard(char *board, char *cginame)
{
	int i, j = 0;
	char buf[128];
	int closed = 0;
	printf("没有这个讨论区啊，可能的选择:<p>");
	printf("<table width=300>");
	board = strtrim(board);
	for (i = 0; i < MAXBOARD && i < shm_bcache->number; i++) {
		if (!strcasestr(shm_bcache->bcache[i].header.filename, board) &&
		    !(strcasestr(shm_bcache->bcache[i].header.title, board)))
			continue;
		if (!has_view_perm_x(currentuser, &(shm_bcache->bcache[i])))
			continue;
		closed = (shm_bcache->bcache[i].header.flag & CLOSECLUB_FLAG);
		printf("<tr><td>");
		printf("<a href=%s?B=%d>%s (%s)</a>",
		       cginame, getbnumx(&shm_bcache->bcache[i]),
		       void1(titlestr(shm_bcache->bcache[i].header.title)),
		       shm_bcache->bcache[i].header.filename);
		printf("</td></tr>");
		j++;
		if (j == 1)
			sprintf(buf, "%s?B=%d", cginame,
				getbnumx(&shm_bcache->bcache[i]));
	}
	printf("</table>");
	if (!j)
		printf
		    ("喔？我真的帮你找了，你那个讨论区一定输入错了, 没有叫做 \"%s\" 的讨论区啊",
		     board);
	if (j == 1 && !closed)
		redirect(buf);
	printf("<p><a href=javascript:history.go(-1)>快速返回</a>");
	http_quit();
}

void
printhrwhite()
{
	printf("<TABLE width=100%% cellspacing=1 cellpadding=0>"
	       "<TR><TD><IMG height=1 src=\"/small.gif\""
	       " width=1></TD></TR></table>");
}

void
printboardhot(struct boardmem *x)
{
	char path[STRLEN];
	sprintf(path, "boards/%s/TOPN", x->header.filename);
	if (showfile(path) >= 0)
		printhr();
	else
		printhrwhite();
}

void
printrelatedboard(char *board)
{
	int i;
	struct boardaux *boardaux = getboardaux(getbnum(board));
	if (!boardaux || !boardaux->nrelate)
		return;
	printf("<div align=center>其他版面");
	for (i = 0; i < boardaux->nrelate; i++) {
		printf(" <a href=doc?B=%d>&lt;%s&gt;</a>",
		       getbnum(boardaux->relate[i].filename),
		       void1(titlestr(boardaux->relate[i].title)));
	}
	printf("</div>");
}

void
printboardtop(struct boardmem *x, int num, char *infostr)
{
	char *board = x->header.filename;
	//char *c1 = "whi", *c2 = "blu";

	char bmbuf[IDLEN * 4 + 4];
	char sbmbuf[IDLEN * 12 + 12];
	printf("<table width=100%%><tr><td align=left class=f1>");
	printf("<a href=boa?secstr=%s class=blu>%s</a> - ",
	       x->header.sec1, nohtml(getsectree(x->header.sec1)->title));
	printf("<a href=home?B=%d class=blu>%s版</a>", getbnumx(x), board);
	printf("</td><td align=center class=f3>%s</td>",
	       void1(titlestr(x->header.title)));
	printf("<td align=right class=f1>版主[%s]",
	       userid_str(bm2str(bmbuf, &(x->header))));
	if (x->ban)
		printf("<a href=bbsshowfile?F=banAlert class=red>%s!!!</a>",
		       (x->ban == 1) ? "" : "<b>");
	if (strlen(sbm2str(sbmbuf, &(x->header)))) {
		printf("<br>小版主[%s]", userid_str(sbmbuf));
	}
	printf("</td></tr>");
	printf("</table>");
	updatewwwindex(x);
	//For javascript function tabs(...), check html/function.js
	printf("<script>tabs('%d',%d,%d,", getbnumx(x), num, x->wwwindex);
	if ( !politics(board) && x->wwwbkn)
		printf("1,");
	else
		printf("0,");
	printf("'%s','%s',%d,%d);</script>", anno_path_of(board),
	       infostr ? infostr : "", x->inboard,
	       (x->header.flag & VOTE_FLAG) ? 1 : 0);
}

int
getdocstart(int total, int lines)
{
	char *ptr;
	int start;
	ptr = getparm("S");
	if (!ptr[0])
		ptr = getparm("start");
//      if (sscanf(ptr, "%d", &start) != 1)
//              start = 0;
	start = atoi(ptr);
	if (start == 0 || start > total - lines + 1)
		start = total - lines + 1;
	if (start <= 0)
		start = 1;
	return start;
}

void
bbsdoc_helper(char *cgistr, int start, int total, int lines)
{
	int manop = atoi(getparm("manop"));

	if (start > 1)
		printf("<a href=%s&S=%d&manop=%d>上一页</a> ", cgistr, start - lines, manop);
	else
		printf("上一页 ");
	if (start < total - lines + 1)
		printf("<a href=%s&S=%d&manop=%d>下一页</a> ", cgistr, start + lines, manop);
	else
		printf("下一页 ");
	printf
	    ("<a href=# onclick='javascript:{location=location;return false;}' class=blu>刷新</a> ");
}

#if 0
static int
istoday(time_t t)
{
	struct tm *tm;
	int year, yday;
	tm = localtime(&now_t);
	if (NULL == tm)
		return 0;
	year = tm->tm_year;
	yday = tm->tm_yday;
	tm = localtime(&t);
	if (NULL == tm)
		return 0;
	if (year == tm->tm_year && yday == tm->tm_yday)
		return 1;
	return 0;
}
#endif

void print_doc_head(int mode, int has_bm_perm);

/*int
bbsdoc_main()
{
	char board[80], dir[80], genbuf[STRLEN], title[STRLEN], filename[80];
	struct boardmem *x1;
	struct fileheader *x, x2;
	struct mmapfile mf = { ptr:NULL };
	int j, i = 0, start, total, fd = -1, sizebyte;
	struct boardaux *boardaux;
	int has_bm_perm, manager, mode;
	
	changemode(READING);
	getparmboard(board, sizeof (board));
	x1 = getboard2(board);
	boardaux = getboardaux(getbnum(board));
	has_bm_perm = has_BM_perm(currentuser, x1);
	manager = atoi(getparm("m"));

	if (!x1 || !boardaux) {
		html_header(1);
		nosuchboard(board, "doc");
	}
	if (!has_read_perm_x(currentuser, x1)) {
		html_header(1);
		noreadperm(board, "doc");
	}
	updateinboard(x1);
	sprintf(dir, "boards/%s/.DIR", board);
//      if(cache_header(file_time(dir),10))
//              return 0; 
	html_header(1);
	check_msg();
	printf("<script src=" BBSJS "></script>\n");
	total = x1->total;
	start = getdocstart(total + boardaux->ntopfile, w_info->t_lines);
	brc_initial(currentuser->userid, board);
	printf("<body topmargin=0 onload=\"check_manop(this.location.search, %d, %d);\">", start, w_info->t_lines);
	sprintf(genbuf, "文章%d篇", total);
	printboardtop(x1, 3, genbuf);
	printboardhot(x1);
	printf("<table cellSpacing=0 cellPadding=2 width=98%% style='table-layout: fixed;'>\n");
	print_doc_head(0, has_bm_perm);
	MMAP_TRY {
		mmapfile(dir, &mf);
		total = mf.size / sizeof (struct fileheader);
		if (total <= 0) {
			printf("<tr><td colspan=7>");
			print_caution_box(1, "本讨论区目前没有文章");
			printf("></td></tr>");
		}
		printf("<script>docItemInit('%d',%d,%d);", getbnumx(x1), start,
		       (int) now_t);
		x = (struct fileheader *) mf.ptr;
		for (i = 0; i < w_info->t_lines; i++) {
			j = start + i - 1;
			if (j < 0 || j >= total)
				break;
			while (x[j].sizebyte == 0) {
				char filename[80];
				sprintf(filename, "boards/%s/%s", board,
					fh2fname(&(x[j])));
				sizebyte = numbyte(eff_size(filename));
				fd = open(dir, O_RDWR);
				if (fd < 0)
					break;
				flock(fd, LOCK_EX);
				lseek(fd,
				      j * sizeof (struct fileheader), SEEK_SET);
				if (read(fd, &x2, sizeof (x2)) == sizeof (x2)
				    && x[j].filetime == x2.filetime) {
					x2.sizebyte = sizebyte;
					lseek(fd, -1 * sizeof (x2), SEEK_CUR);
					write(fd, &x2, sizeof (x2));
				}
				flock(fd, LOCK_UN);
				close(fd);
				fd = -1;
				break;
			}
			printf("docItem('%s',%d,'%s','%s',%d,",
					has_bm_perm?
						flag_str_bm(x[j].accessed, !brc_un_read(&(x[j]))):
						flag_str2(x[j].accessed, !brc_un_read(&(x[j]))),
					x[j].accessed & FH_NOREPLY ? 1 : 0, 
					fh2owner(&(x[j])), 
					fh2fname(&(x[j])),
					feditmark(&x[j]));
			printf("'%s',%d,%d,%d,%d,%d,%d);\n", 
					scriptstr(x[j].title), 
					bytenum(x[j].sizebyte),
					x[j].staravg50 / 50, 
					x[j].hasvoted, 
					has_bm_perm, getbnumx(x1), 
					start);
		}
	}
	MMAP_CATCH {
		if (fd != -1)
			close(fd);
	}
	MMAP_END mmapfile(NULL, &mf);
	printf("</script>\n");
	if (i < w_info->t_lines) {
		//显示置顶文章
		for (j = 0; j < boardaux->ntopfile; j++) {
			struct fileheader *tfh = &boardaux->topfile[j];
			printf
			    ("<tr><td align=center><b>[提示]</b></td><td></td><td>%s</td>"
			     "<td align=center>%6.6s</td>",
			     userid_str(fh2owner(tfh)),
			     (Ctime(tfh->filetime) + 4));
			strncpy(title, tfh->title, 60);
			title[60] = 0;
			printf
			    ("<td><a href=bbstopcon?B=%d&F=%s>%s%s</a>%s</td></tr>",
			     getbnumx(x1), fh2fname(tfh), strncmp(title, "Re: ",
								  4) ? "○ " :
			     "", void1(titlestr(title)),
			     size_str(bytenum(tfh->sizebyte)));
		}
	}

	printf("</table>");
	printhr();
	printf("<script>showdocnav(0, %d, %d, %d, %d, %d, %d);</script>", 
			getbnumx(x1), 
			get_num_psttmpls(board),
			has_bm_perm, 
			start, 
			w_info->t_lines, 
			total + boardaux->ntopfile);
	
	docinfostr(x1, 0, has_bm_perm);
	printf(" <a href=tshirt class=red>订购糊涂站衫</a> \n");
	printf(" <a href=pst?B=%d class=red>我要发表文章！</a> \n",
	       getbnumx(x1));
	if (get_num_psttmpls(board) > 0)
		printf("<a href=psttmpl?B=%d&action=view>模板发文</a>\n", getbnumx(x1));
	else if (has_bm_perm)
		printf("<a href=psttmpl?B=%d&action=view>创建模板</a>\n", getbnumx(x1));
	printf("<a href=bfind?B=%d>版内查询</a> ", getbnumx(x1));
	printf("<a href=clear?B=%d&S=%d>清除未读</a> ", getbnumx(x1), start);
	sprintf(genbuf, "doc?B=%d", getbnumx(x1));
	bbsdoc_helper(genbuf, start, total + boardaux->ntopfile,
		      w_info->t_lines);
	printdocform("doc", getbnumx(x1));
	printrelatedboard(board);

	if (manager && has_bm_perm) {
		mode = atoi(getparm("mode"));
		strncpy(filename, getparm("fname"), sizeof(filename));
		do_manager(mode, board, filename);
		printf("<script>location.replace('bbsdoc?B=%d&S=%d&manop=1');</script>", 
				getbnumx(x1), start);
	}
		
	printf("</body>");
	http_quit();
	return 0;
}*/

char *
size_str(int size)
{
	static char buf[256];
	if (size < 1000) {
		sprintf(buf, "(<font class=tea>%d字</font>)", size);
	} else {
		sprintf(buf, "(<font class=red>%d.%d千字</font>)", size / 1000,
			(size / 100) % 10);
	}
	return buf;
}

char *
short_Ctime(time_t t)
{
	char *ptr = Ctime(t) + 4;
	ptr[12] = 0;
	//对20小时内得进行缩短
//      if (now_t - t < 20 * 3600)
	ptr += 7;
	return ptr;
}

void
docinfostr(struct boardmem *brd, int mode, int haspermm)
{
	printf("<div align=center>");
	if (mode == 0)
		printf("[一般模式 <a href=tdoc?B=%d>主题模式</a>",
		       getbnumx(brd));
	else
		printf("[<a href=doc?B=%d>一般模式</a> 主题模式",
		       getbnumx(brd));
	if (haspermm) {
		if (mode == 2)
			printf(" 管理模式");
		else
			//printf(" <a href=mdoc?B=%d>管理模式</a>",
			//       getbnumx(brd));
			printf(" <a href=\"#\" onclick=switch_manop();>管理模式</a>]");
	}
	printf("]");
}

/* 打印文章表头 */
/* mode = 0 为一般模式*/
/* mode = 1 为主题模式*/
void
print_doc_head(int mode, int has_bm_perm)
{
	if (has_bm_perm)
		printf("<tr align=center class=docbgcolor><td width=7%% align=center>%s序号</td>"
				"<td width=5%% align=center>状态</td>"
				"<td width=12%%>作者</td>"
				"<td width=7%%>时间</td>"
				"<td width=46%%>标题</td>"
				"<td width=5%%>%s</td>\n"
				"<td width=18%% id=manop style='display: none;'>管理操作</td></tr>\n",
				mode == 0 ? "" : "原",
				mode == 0 ? "星级" : "回贴/推荐");
	else
		printf("<tr align=center class=docbgcolor><td width=7%%>%s序号</td>"
				"<td width=7%%>状态</td>"
				"<td width=12%%>作者</td>"
				"<td width=7%%>时间</td>"
				"<td width=62%%>标题</td>"
				"<td width=5%%>%s</td></tr>\n", 
				mode == 0 ? "" : "原", 
				mode == 0 ? "星级" : "回贴/推荐");
	return;
}

//以下为Fishingsnow于2005年9月13日新增，
//本注释以上内容未来将会被下面的函数逐步替换


int doc_init_0(struct fileheader *board_data, struct fileheader *doc_data,
		int start, int total, int pagesize, 
		int *has_prev_page, int *has_next_page, char *boardname)
//按照一般模式存入版面数据
{
	int i, j, sum = 0, sizebyte, fd;
	char filename[80], dir_path[80];
	struct fileheader tmp;

	*has_prev_page = 0;
	*has_next_page = 0;

	for (i = 0; i < pagesize; i++) {
		j = i + start - 1;
		if (j < 0 || j >= total)
			break;
		memcpy(&(doc_data[i]), &(board_data[j]), 
				sizeof (struct fileheader));
		sprintf(dir_path, "boards/%s/.DIR", boardname);
		while (doc_data[i].sizebyte == 0) {
			sprintf(filename, "boards/%s/%s", boardname, 
					fh2fname(&(doc_data[i])));
			sizebyte = numbyte(eff_size(filename));
			fd = open(dir_path, O_RDWR);
			if (fd < 0)
				break;
			flock(fd, LOCK_EX);
			lseek(fd, j * sizeof(struct fileheader), SEEK_SET);
			if (read(fd, &tmp, sizeof (tmp)) == sizeof(tmp) 
					&& doc_data[i].filetime == 
					tmp.filetime) {
				tmp.sizebyte = sizebyte;
				lseek(fd, -1 * sizeof (tmp), SEEK_CUR);
				write(fd, &tmp, sizeof (tmp));
			}
			flock(fd, LOCK_UN);
			close(fd);
			fd = -1;
			break;
		}
		sum++;
	}
	if (start != 1)
		*has_prev_page = 1;
	if (start + sum < total)
		*has_next_page = 1;
	return sum;
}

int doc_init_1(struct fileheader *board_data, struct fileheader *doc_data, 
		int *doc_info, int start, int total, 
		int direction, int pagesize)
//按照主题模式存入版面数据
{
	int i, sum = 0;

	for (i = start; i >= 0 && i < total; i += direction) {
		if (board_data[i].thread != board_data[i].filetime)
			//非主题贴ignore
			continue;
		memcpy(&(doc_data[sum]), &(board_data[i]), 
				sizeof(struct fileheader));
		doc_info[sum] = i;
		sum++;
		if (sum >= pagesize)
			break;
	}

	if (i < 0)
		//说明已经到了第一篇文章，但是此时可能pagesize不满
		//在文章足够的情况下，从第一篇文章开始正着调用一次
		//这个时候文章是正序的，但是direction是倒序的
		//所以让sum变成负的
		return -doc_init_1(board_data, doc_data, doc_info, 0, total, 
				1, pagesize);
		return sum;

	return sum;
}

int doc_test_1(struct fileheader *board_data, int *doc_info, int sum, int total, 
		int direction, int *has_prev_page, int *has_next_page)
//监测主题模式是否有前后页
{
	int i, pos;
	
	//当前屏幕上最靠前的文章
	pos = min(doc_info[0], doc_info[sum - 1]);
	for (i = 0; i < pos; i++)
		if (board_data[i].thread == board_data[i].filetime) {
			*has_prev_page = 1;
			break;
		}

	//当前屏幕上最靠后的文章
	pos = max(doc_info[0], doc_info[sum - 1]);
	for (i = pos + 1; i < total; i++)
		if (board_data[i].thread == board_data[i].filetime) {
			*has_next_page = 1;
			break;
		}

	return sum;
}

static int
doc_stat_1(struct fileheader *board_data, int from, int total, 
		int *click, int *reply, int *unread)
//统计版面第from文章的主题有多少回贴，多少未读
{
	int i, filetime, thread;

	*click = board_data[from].staravg50 * 
		board_data[from].hasvoted / 50;
	*reply = 0;
	*unread = 0;
	thread = board_data[from].thread;
	filetime = board_data[from].filetime;
	
	for (i = from + 1; i < total; i++) {
		if (board_data[i].thread == thread) {
			(*reply)++;
			*click += board_data[i].staravg50 * 
				board_data[i].hasvoted / 50;
			if (filetime != board_data[i].filetime &&
					brc_un_read(&(board_data[i])))
				(*unread)++;
		}
	}

	return 1;
}

int 
doc_print_title(struct boardmem *x, int highlight) {
//显示版面名称和选单
	char *board = x->header.filename;
	char bmbuf[IDLEN * 4 + 4];
	char bmbuf2[IDLEN * 12 + 12];
	int bnum = getbnumx(x);
  char bmhelp[200];
  
	bm2str(bmbuf, &(x->header));
	sbm2str(bmbuf2, &(x->header));
  if (!isguest && loginok) {
    if (!has_BM_perm(currentuser,x)) {
      sprintf(bmhelp,"<a href=\"psttmpl?B=3&action=post&num=1\" class=\"red\">我要申请版主！</a>");
      } else {
      sprintf(bmhelp,"<a href=\"bbsanc?path=/groups/GROUP_M/BBSHelp&item=/M.1229495526.A\" class=red>版主操作帮助</a>");
      }
  } else {
      sprintf(bmhelp," ");
  }
	printf("<div><script>checkFrame(%d);</script></div>\n", 
			atoi(getparm("redirect")));
	printf("<script>\ndoc_show_title('%s','%s',%d,'%s',", 
			x->header.sec1, void1((unsigned char *) titlestr(x->header.title)), 
			bnum, userid_str(bmbuf));
	printf("'%s','%s','%s',%d,'%s');\n", 
			nohtml(getsectree(x->header.sec1)->title), board,
			userid_str(bmbuf2), 
			x->ban,bmhelp);
	updatewwwindex(x);
	printf("doc_show_tabs(%d,%d,%d,%d,'%s',%d,%d,%d,%d)\n</script>\n", 
			bnum, highlight, x->wwwindex,
			!politics(board) && x->wwwbkn ? 1 : 0, 
			anno_path_of(board), 
			x->total, 
			x->inboard, 
			(x->header.flag & VOTE_FLAG) ? 1 : 0, 
			has_BM_perm(currentuser, x));
	return 1;
}

int 
doc_print_top_article(struct boardaux *aux, int pagesize, 
		int has_next_page, int sum, int bnum)
//显示置顶文章
{
	struct fileheader *fh;
	int i;
	char title[60];

	fh = (struct fileheader *)malloc(sizeof(struct fileheader));
	if (fh == NULL)
		return 0;
	if (sum < pagesize || !has_next_page)
		for (i = 0; i < aux->ntopfile; i++) {
			memcpy(fh, &(aux->topfile[i]), sizeof(struct fileheader));
			strncpy(title, fh->title, 60);
			printf("doc_show_top_item('%s','%6.6s',%d,"
					"'%s','%s',%d);\n",
					userid_str(fh2owner(fh)), 
					Ctime(fh->filetime) + 4, 
					bnum, fh2fname(fh), 
					void1((unsigned char *)titlestr(title)), 
					bytenum(fh->sizebyte));
			}
	return 1;
}

int doc_print_related(int bnum, int mode) {
//显示相关版面
	int i;
	struct boardaux *aux = getboardaux(bnum);

	if (!aux || !aux->nrelate)
		return 0;
	
	printf("<div align=center class=doc_related>");
	for (i = 0; i < aux->nrelate; i++) {
		printf("[<a href=doc?B=%d&M=%d>%s</a>] ", 
			getbnum(aux->relate[i].filename), mode,
			void1((unsigned char *) 
				titlestr(aux->relate[i].title)));
	}
	printf("</div>");
	return 1;
}
		
int doc_print_hot(char *boardname) {
	char path[STRLEN];
	sprintf(path, "boards/%s/TOPN", boardname);
	showfile(path);
	return 1;
}

int doc_nosuchboard(char *boardname) {
	int i, j = 0;
	char buf[STRLEN * 11];
	char buf2[STRLEN];
	int closed = 0;
	
	sprintf(buf, "没有这个讨论区啊，可能的选择：<br /><br />");
	for (i = 0; i < MAXBOARD && i < shm_bcache->number; i++) {
		if(!strcasestr(shm_bcache->bcache[i].header.filename, 
				boardname) &&
			!(strcasestr(shm_bcache->bcache[i].header.title, 
				boardname))) {
				continue;
			}
		if (!has_view_perm_x(currentuser, &(shm_bcache->bcache[i])))
			continue;
		closed = (shm_bcache->bcache[i].header.flag & CLOSECLUB_FLAG);
		sprintf(buf, "%s<a href=doc?B=%d>%s[%s]</a><br />", buf,
			getbnumx(&shm_bcache->bcache[i]), 
			void1((unsigned char *)
				titlestr(shm_bcache->bcache[i].header.title)), 
				shm_bcache->bcache[i].header.filename);
		j++;
		if (j == 1)
			sprintf(buf2, "doc?B=%d", getbnumx(&shm_bcache->bcache[i]));
	}

	if (!j)
		sprintf(buf, "%s啊，没有叫 %s 的讨论区，而且竟然一个可能的版面都没有找到！", 
				buf, boardname);
	print_caution_box(1, buf);
	if (j == 1 && !closed)
		redirect(buf2);
	http_quit();
	return 0;
	
}

int
ajax_bbsdoc_main_st() {
	int start, ftime, offset, direction, total, sum;
	char boardname[80], dirpath[STRLEN];
	struct boardmem *x;
	struct fileheader *board_data = NULL;
	struct mmapfile mf = {ptr: NULL};
	static struct fileheader *doc_data = NULL;
	static int *doc_info;
	int i, j, reply, unread, click;

	getparmboard(boardname, sizeof(boardname));
	x = getboard(boardname);
	if (!x) {
		printf("无法找到版面。");
		http_quit();
	}

	sprintf(dirpath, "boards/%s/.DIR", boardname);
	start = atoi(getparm("s"));
	direction = atoi(getparm("d"));
	offset = atoi(getparm("o"));
	ftime = atoi(getparm("f"));
	offset = offset <= 1 ? CON_MAX_OTHERTHREAD : offset;

	if (NULL == doc_data) {
		doc_data = malloc(offset * sizeof(struct fileheader));
		if (NULL == doc_data) {
			printf("内部错误01。");
			http_quit();
		}
	}

	if (NULL == doc_info) {
		doc_info = malloc(offset * sizeof(struct fileheader));
		if (NULL == doc_info) {
			printf("内部错误02。");
			http_quit();
		}
	}

	brc_initial(currentuser->userid, boardname);

	mmapfile(NULL, &mf);
	MMAP_TRY {
		if (mmapfile(dirpath, &mf) < 0) {
			MMAP_UNTRY;
			printf("内部错误03。");
			http_quit();
		}

		board_data = (struct fileheader *) mf.ptr;
		total = mf.size / sizeof (struct fileheader);
		switch (direction) {
			case 1:
			case -1:
				break;
			case 2:
				direction = -1;
				start = total -1;
				break;
			case -2:
				direction = 1;
				start = 0;
			default:
				direction = 1;
				break;
		}
		
		if (start > total - 1 || start == -2) {
			start = total - 1;
			direction = -1;
		} else if (start < 0) {
			start = 0;
			direction = 1;
		}
		sum = doc_init_1(board_data, doc_data, doc_info, 
				start, total, direction, offset);
		if (sum == 0 || (sum < offset && sum > 0))
			sum = -doc_init_1(board_data, doc_data, doc_info, 
				total - 1, total, -1, offset);
		if (sum < 0) {
			direction = -direction;
			sum = -sum;
		}
		printf("con_docnav_init(%d, %d, -1, -1, 1);con_doclist_head("
			"'con_doclist_content');", 
				ftime, offset);
		for (i = 0; i < sum; i++) {
			j = direction == 1 ? i : sum - i - 1;
			doc_stat_1(board_data, doc_info[j], total, 
					&click, &reply, &unread);
			printf("con_doclist_item(%d, '%s', '%s', '%s', %d, "
				"'%s', %d, %d, %d);", 
				doc_info[j] + 1,
				flag_str2(doc_data[j].accessed, 
					!brc_un_read(&(doc_data[j]))), 
				fh2owner(&(doc_data[j])), 
				fh2fname(&(doc_data[j])), 
				feditmark(&(doc_data[j])), 
				scriptstr(doc_data[j].title), 
				bytenum(doc_data[j].sizebyte), 
				reply, unread);
		}
		printf("con_doclist_end('con_doclist_content');");
	} MMAP_CATCH {
	} MMAP_END mmapfile(NULL, &mf);
	
	http_quit();
	return 1;
}

int
ajax_bbsdoc_main() {
	int bnum, start, ftime, offset, direction, total, mode;
	int newstart, i;
	char boardname[80], dirpath[STRLEN];
	struct fileheader *fh;
	struct boardmem *x;
	struct mmapfile mf = {ptr: NULL};

	printf("Content-type: text/html; charset=%s\r\n\r\n", CHARSET);

	mode = atoi(getparm("m"));
	if (mode == 1) {
		ajax_bbsdoc_main_st();
		return 0;
	}
	getparmboard(boardname, sizeof(boardname));
	x = getboard(boardname);
	if (!x)
		ajax_fatal("版面不存在或无权访问");
	brc_initial(currentuser->userid, boardname);
	
	bnum = getbnumx(x);
	start = atoi(getparm("s"));
	ftime = atoi(getparm("f"));
	direction = atoi(getparm("d"));
	offset = atoi(getparm("o"));
	offset = offset <= 1 ? offset : 10;

	sprintf(dirpath, "boards/%s/.DIR", boardname);

	mmapfile(NULL, &mf);
	MMAP_TRY {
		if (mmapfile(dirpath, &mf) < 0) {
			MMAP_UNTRY;
			printf("读取版面文章列表发生意外");
			http_quit();
		}

		total = mf.size / sizeof(struct fileheader);

		switch (direction) {
			case -2:
				start = 0;
				break;
			case 2:
				start = total - 1;
				break;
			case -1:
				break;
			default:
				direction = 1;
		}

		if (start > total - 1 || start < 0)
			start = Search_Bin((struct fileheader *) mf.ptr, 
					ftime, 0, total - 1);
		
		start = start < 0 ? -start : start;

		newstart = start + direction * offset - 1;
		if (newstart + offset > total - 1)
			newstart = total - offset;
		if (newstart < 0)
			newstart = 0;
		printf("con_docnav_init(%d, %d, %d, %d);con_doclist_head("
			"'con_doclist_content');", 
				ftime, offset, 
				(newstart + offset - 1)/ offset + 1, 
				(total - 1) / offset + 1);
		fh = (struct fileheader *) (mf.ptr + newstart * 
			sizeof(struct fileheader));
		for (i = 0; i < offset && newstart + i < total; i++, fh++) {
			printf("con_doclist_item(%d, '%s', '%s', '%s', %d, "
				"'%s', %d, %d, %d);", 
				newstart + i + 1, 
				flag_str2(fh->accessed, !brc_un_read(fh)), 
				fh2owner(fh), fh2fname(fh), feditmark(fh), 
				scriptstr(fh->title), bytenum(fh->sizebyte), 
				fh->staravg50 / 50, fh->hasvoted);
		}
		printf("con_doclist_end('con_doclist_content');");
	} MMAP_CATCH {
	} MMAP_END mmapfile(NULL, &mf);
	
	http_quit();
	return 1;
}

int
bbsdoc_main() {
	char boardname[80], dirpath[STRLEN];
	struct boardmem *x;
	struct fileheader *board_data = NULL;
				//整个版面的文章
	struct mmapfile mf = {ptr: NULL};
	static struct fileheader *doc_data = NULL;
				//当前页面的文章
	static int *doc_info;	//同主题方式当前页文章的位置数组
	struct boardaux *aux;	//置底文章
	int has_bm_perm;	//是否当前斑竹
	int man_action;		//是否进行管理操作
	int has_prev_page = 0;	//是否有前一页
	int has_next_page = 0;	//是否有后一页
	int click, reply, unread;
				//点击数、回贴数、未读回贴数
	int bnum;		//版面编号
	int start, start2;	//开始的文章号，原始输入的文章号
	int total;		//版面的全部文章总数
	int direction, direction2;
				//查找文章的方向，原始输入的方向
	int mode;		//阅读版面的方式：0 一般 1 主题
	int pagesize;		//页面条目多少
	int sum = 0;		//当前页的文章数（没有足够多文章时）
	int has_tmpl;		//是否有模板
	int manop;		//是否显示管理菜单
	int i, j;
	char ajax[10];

	strncpy(ajax, getparm("ajax"), 10);

	if (!strcmp(ajax, "con"))
		ajax_bbsdoc_main();
	
	changemode(READING);
	html_header(1);
	printf("<script src='" BBSDOCJS "'></script>\n");
	
	//开始获取数据并初始化版面
	getparmboard(boardname, sizeof (boardname));
	x = getboard(boardname);
	bnum = getbnumx(x);
	aux = getboardaux(bnum);

	printf("<title>%s %s 一路BBS版面</title>\n</head>\n", titlestr(x->header.title), boardname);


	if (!x || !aux) {
		html_header(1);
		doc_nosuchboard(boardname);
	}
	updateinboard(x);
	sprintf(dirpath, "boards/%s/.DIR", boardname);
	has_bm_perm = has_BM_perm(currentuser, x);
	man_action = atoi(getparm("A"));
	start = atoi(getparm("S"));
	start2 = start;
	direction = atoi(getparm("D"));
	direction2 = direction;
	mode = getparm("M") == "" ? 	//未指定阅读方式则根据个人参数选择
		w_info->def_mode : atoi(getparm("M"));
	pagesize = w_info -> t_lines;
	has_tmpl = get_num_psttmpls(boardname);
	manop = atoi(getparm("manop"));

	if (NULL == doc_data) {
		doc_data = malloc(DOC_MAX_LINES * sizeof (struct fileheader));
		if (NULL == doc_data)
			http_fatal("分配内存空间出错，请尝试刷新。");
	}
	if (NULL == doc_info) {
		doc_info = malloc(DOC_MAX_LINES * sizeof (int));
		if (NULL == doc_info)
			http_fatal("分配内存空间出错，请尝试刷新。");
	}

//	printf("<body>\n");
//  修改框架，加上页面左侧
	printf("<body topmargin=0 leftMargin=1 MARGINWIDTH=1 MARGINHEIGHT=0>" WWWLEFT_DIV);
	//check_msg();
	brc_initial(currentuser->userid, boardname);

	doc_print_title(x, 3);

	doc_print_hot(boardname);

	mmapfile(NULL, &mf);
	MMAP_TRY {
		if (mmapfile(dirpath, &mf) < 0) {
			MMAP_UNTRY;
			http_fatal("无法读取版面文章列表，请联系系统维护。");
		}

		board_data = (struct fileheader *) mf.ptr;
					//载入整个版面的fileheader
		total = mf.size / sizeof (struct fileheader);
		
		switch(mode) {
		case 0:
			if (start > total + aux->ntopfile - pagesize + 1 || 
					start == 0)
			//如果start过大或者没有指定则从最后一页开始
				start = total + aux->ntopfile - pagesize + 1;
			if (start < 0)
				//如果start过小则从第一页开始
				start = 1;
			sum = doc_init_0(board_data, doc_data, start, total, 
					pagesize, &has_prev_page, &has_next_page, 
					boardname);

			printf("<script>doc_show_head(0, %d, %d, %d, %d);\n", 
					has_bm_perm, bnum, start, (int) now_t);
			for (i = 0; i < sum; i++) {
				printf("doc_show_item(0, '%s', %d, '%s', "
					"'%s', %d, '%s', %d, %d, %d, "
					"%d, %d, %d, 0, 0, %d, 0, %d);\n", 
					has_bm_perm ? 
						flag_str_bm(doc_data[i].accessed, 
						!brc_un_read(&(doc_data[i]))) :
						flag_str2(doc_data[i].accessed, 
							!brc_un_read(&(doc_data[i]))), 
					doc_data[i].accessed & FH_NOREPLY ? 1 : 0, 
					fh2owner(&(doc_data[i])), 
					fh2fname(&(doc_data[i])), 
					feditmark(&(doc_data[i])),
					scriptstr((unsigned char *)doc_data[i].title), 
					bytenum(doc_data[i].sizebyte), 
					doc_data[i].staravg50 * doc_data[i].hasvoted / 50,
					doc_data[i].hasvoted,
					has_bm_perm, bnum, start, start + i, 
					doc_data[i].thread);
			}
			
			if (!doc_print_top_article(aux, pagesize, has_next_page, sum, bnum)) {
				MMAP_UNTRY;
				printf("</scirpt></table>\n");
				http_fatal("读取版面置顶文章失败。");
			}

			printf("doc_show_end();\n"
				"doc_show_nav(0, %d, %d, %d, %d, 0, "
				"%d, %d, %d, '" SMAGIC "');\n", 
				bnum, has_tmpl, has_bm_perm, start, pagesize, 
				has_prev_page, has_next_page);
			break;
		case 1:
			direction = !direction ? -1 : direction;
			//当未指定方向时默认为逆向，从而配合未指定start的情况
			if (start == 0) {
			//如果没有给定start参数
			//那么显示最后一屏
				start = total - 1;
				direction = -1;
			} else if (start < 0) {
				start = 0;
				direction = 1;
			} else{
			//这里假设start<total的时候是文章的编号
			//但是如果事实上start的原意是文章的编号，而不是一
			//个thread的话，就是错误的了。
			//好在合理的文章编号和合理的filetime应该无交集
				if (start < total && start != 1)
					start = Search_Bin((struct fileheader *) 
						mf.ptr, board_data[start - 1].filetime, 
						0, start);
				else
					start = Search_Bin((struct fileheader *) 
						mf.ptr, start, 0 ,total - 1);
				//上面以start为key进行检索
				//找到了返回其在.DIR中的编号
				//否则返回比start大的最小filetime的位置
				//但是此时返回值是负的
				start = start < 0 ?
					-(start + 1) : start + direction;
				//上面根据所搜的结果给出开始的位置
			}
			sum = doc_init_1(board_data, doc_data, doc_info, 
					start, total, direction, 
					start == total - 1 ? 
						pagesize - aux->ntopfile : 
						pagesize);
			if (sum == 0 || (sum + aux->ntopfile < pagesize && sum > 0))
				//没有找到结果或最后一页帖子不够
				//则从最后一页开始凑齐
				sum = -doc_init_1(board_data, doc_data, doc_info, 
						total - 1, total, -1, 
						pagesize - aux->ntopfile);
			if (sum < 0) {
				direction = -direction;
				sum = -sum;
			}
			
			doc_test_1(board_data, doc_info, sum, total, direction, 
					&has_prev_page, &has_next_page);
			printf("<script>doc_show_head(1, %d, %d, %d, %d);\n", 
					has_bm_perm, bnum, start, (int) now_t);

			for (i = 0; i < sum; i++) {
				j = direction == 1 ? i : sum - i - 1;
				doc_stat_1(board_data, doc_info[j], total, 
						&click, &reply, &unread);
				printf("doc_show_item(1, '%s', %d, '%s', "
					"'%s', %d, '%s', 0, %d, 0, "
					"%d, %d, %d, %d, %d, %d, %d, %d);\n", 
					has_bm_perm ? 
						flag_str_bm(doc_data[j].accessed, 
							!brc_un_read(&(doc_data[j]))) :
						flag_str2(doc_data[j].accessed, 
							!brc_un_read(&(doc_data[j]))), 
					doc_data[j].accessed & FH_NOREPLY ? 1 : 0, 
					fh2owner(&(doc_data[j])), 
					fh2fname(&(doc_data[j])), 
					feditmark(&(doc_data[j])),
					scriptstr((unsigned char *)doc_data[j].title), 
					click, has_bm_perm, bnum,
					start2,
					reply, unread, doc_info[j] + 1, direction2, 
					doc_data[j].thread);
			}
			
			if (!doc_print_top_article(aux, pagesize, has_next_page, sum, bnum)) {
				MMAP_UNTRY;
				printf("</scirpt></table>\n");
				http_fatal("读取版面置顶文章失败。");
			}
			
			printf("doc_show_end();\n"
				"doc_show_nav(1, %d, %d, %d, %d, "
				"%d, %d, %d, %d, '" SMAGIC "');\n", 
				bnum, has_tmpl, has_bm_perm, 
				direction == 1 ? 
					doc_data[0].filetime : 
					doc_data[sum - 1].filetime, 
				direction == 1 ?
					doc_data[sum - 1].filetime :
					doc_data[0].filetime, 
				pagesize,
				has_prev_page, has_next_page);
			
			break;
		default:
			MMAP_UNTRY;
			http_fatal("目前尚不支持的访问模式。");
			break;
		}

		if (manop)
			printf("open_manop(%d,%d,%d,%d);\n", sum, 
					has_prev_page, has_next_page, mode);
		printf("</script>\n");
		doc_print_related(bnum, mode);
		showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
//		printf("</body></html>");
//		修改框架，加上页面底部
		printf(WWWFOOT_DIV "</body></html>");

	} MMAP_CATCH {
	} MMAP_END mmapfile(NULL, &mf);

	return 1;
}
