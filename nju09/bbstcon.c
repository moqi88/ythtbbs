#include "bbslib.h"

int ismozilla;

int
bbstcon_main()
{
	char board[80], dir[80], boardtitle[80], *path, *ptr;
	struct fileheader *x = NULL;
	struct boardmem *x1;
	int num = 0, firstnum = 0, found = 0, start, total;
	int thread, filetime0 = 0;
	struct mmapfile mf = { ptr:NULL };

	html_header(1);
	path = getsenv("SCRIPT_URL");
	if (strstr(path, "..") || strstr(path, "//"))
		http_fatal("错误的路径 1。");
	if (!(path = strchr(path + 1, '/')))
		http_fatal("错误的路径 2。");
	if (!strncmp(++path, "tcon_", 5)) {
		path += 5;
		if (!(ptr = strchr(path, '_')))
			http_fatal("错误的路径 3。");
		*ptr = 0;
		parm_add("B", path);
		path = ptr + 1;
		if (!(ptr = strchr(path, '.')))
			http_fatal("错误的路径 4。");
		*ptr = 0;
		parm_add("th", path);
	}
	getparmboard(board, sizeof (board));
	thread = atoi(getparm("th"));
	//check_msg();
	printf("<script>var baseurl='/HT/';</script>");
			//temporarily added by Fishingsnow
			//should be update to a solid one.
	printf("<script src=\"" BBSCONJS "\"></script>\n"
		"<script src=\"" BBSAJAXJS "\"></script>\n"
		"<script src=\"" BBSCOOKIEJS "\"></script>"
			"</head>\n");
//	printf("<body "	"onload=\"con_check_width();\">\n");
//修改框架 增加左侧和底部
	printf("<body topmargin=0 leftMargin=1 MARGINWIDTH=1 MARGINHEIGHT=0 onload=\"con_check_width();\">\n" WWWLEFT_DIV);

	printf("<script>var defaultsmagic = '%s';</script>", SMAGIC);
	if ((x1 = getboard(board)) == NULL)
		http_fatal("错误的讨论区");
	strsncpy(boardtitle, 
		void1((unsigned char*) titlestr(x1->header.title)), 40);
	printf("<script>con_show_head('" MY_BBS_NAME "', "
			"'%s', '%c', '%s', %d, %d);</script>\n",
			nohtml(getsectree(x1->header.sec1)->title),
			x1->header.sec1[0], boardtitle, getbnumx(x1), 
			atoi(getparm("redirect")));
#ifdef ENABLE_MYSQL
	printf("<script>var enmysql = 1;</script>");
#else
	printf("<script>var enmysql = 0;</script>");
#endif
	start = atoi(getparm("start"));
	brc_initial(currentuser->userid, board);
	sprintf(dir, "boards/%s/.DIR", board);
	MMAP_TRY {
		if (mmapfile(dir, &mf) == -1) {
			MMAP_UNTRY;
			http_fatal("目录错误");
		}
		total = mf.size / sizeof (struct fileheader);
		if (start <= 0 || start > total) {
			start =
			    Search_Bin((struct fileheader *) mf.ptr, thread, 0,
				       total - 1);
			if (start < 0)
				start = -(start + 1);
		} else {
			start--;
		}
		ismozilla = testmozilla();
		printf("<div id=\"con_main\">\n<div id=\"con_main_left\">");
		for (num = start; num < total; num++) {
			x = (struct fileheader *) (mf.ptr + 
				num * sizeof (struct fileheader));
			if (x->thread != thread) {
				continue;
			}
			show_file(x1, x, num, 2);
			if (!found) {
				found = 1;
				firstnum = num - 1;
				filetime0 = x->filetime;
			}
		}
		printf(	"<div id=\"con_doclist_nav\">\n"
			"<div id=\"con_doclist_nav_padding\">\n"
			"</div></div>\n"
			"<div id=\"con_doclist_ajax\">\n"
			"<div id=\"con_doclist_ajax_padding\">\n"
			"</div></div>\n"
			"<div class=\"clear\"></div>"
			"<div id=\"con_doclist_content\"></div>"
			"<script>"
			"var tzdiff = (new Date())."
			"getTimezoneOffset()*60 + 8*3600;\n"
			"var today = new Date((%d + tzdiff)*1000);\n"
			"var board = %d;\n"
			"var ftime = %d;\n"
			"con_docnav_init(%d, %d, -1, -1, 2);"
			"con_doclist_head('con_doclist_content');"
			"con_doclist_end('con_doclist_content');"
			"</script>", 
			(int)time(NULL), getbnumx(x1), thread, 
			filetime0, CON_MAX_OTHERTHREAD);
		printf("<script>con_doclist_update(%d, %d, %d, 1);"
			"</script>", start - 5, x->thread, 
			CON_MAX_OTHERTHREAD);
		printf("</div>");
		con_print_main_right(x1, x);
		printf("</div><div class=\"clear\"></div><br><br>");
	}
	MMAP_CATCH {
		found = 0;
	}
	MMAP_END mmapfile(NULL, &mf);
	if (found == 0)
		http_fatal("错误的文件名");
	processMath();
	printf("<script>con_switch_right_bycookie();</script>");
	showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
//	printf("</body>\n");
	printf(WWWFOOT_DIV "</body></html>\n");

	brc_update(currentuser->userid);
	http_quit();
	return 0;
}

int
fshow_file(FILE * output, char *board, struct fileheader *x)
{
	char path[80];
	char interurl[256];
	if ((x->accessed & FH_MATH)) {
		usedMath = 1;
		usingMath = 1;
		withinMath = 0;
	} else {
		usingMath = 0;
	}
	binarylinkfile(fh2fname(x));
	sprintf(path, "boards/%s/%s", board, fh2fname(x));
	if (!w_info->doc_mode && !hideboard(board)
	    && (via_proxy
		|| (wwwcache->text_accel_addr.s_addr && wwwcache->text_accel_port))) {
		sprintf(path, "boards/%d/%s", getbnum(board), fh2fname(x));
		if (via_proxy)
			snprintf(interurl, sizeof (interurl),
				 "/" SMAGIC "/%s+%d", path,
				 x->edittime ? (x->edittime - x->filetime) : 0);
		else
			snprintf(interurl, sizeof (interurl),
//				 "http://proxy.%s:%d/" SMAGIC "/%s+%d",
				 "http://%s:%d/" SMAGIC "/%s+%d",
				 MY_BBS_DOMAIN,
				 wwwcache->text_accel_port, path,
				 x->edittime ? (x->edittime - x->filetime) : 0);
		fprintf(output, "<script>docStr=''</script>");
		fprintf(output, "<script src=\"%s\"></script>", interurl);
		ttid(1);
		fprintf(output, "<div id=\"tt%d\" class=\"con_content_%d\">"
				"</div>", ttid(0), contentbg);
		fprintf(output,
			"<script>document.getElementById('tt%d')."
			"innerHTML=docStr;</script>", ttid(0));
		return 0;
	}
	ttid(1);
	fprintf(output, "<div id=\"tt%d\" class=\"con_content_%d\">"
			"</div>", ttid(0), contentbg);
// script will harm SEO
#if 0
	fputs("<script>var docStr = \"", output);
#endif
	fputs("\n", output);
	fshowcon(output, path, 2);
#if 0
	fprintf(output, "\";\ndocument.getElementById('tt%d')"
			".innerHTML=docStr;</script>", ttid(0));
#endif
	fprintf(output, "\n", ttid(0));
	return 0;
}

int
show_file(struct boardmem *brd, struct fileheader *x, int num, int st)
{
	int outgoing;
	char tmptitle[STRLEN];
	int myreply_mode=0;
	char buf[256];

	strncpy(tmptitle, scriptstr(x->title), sizeof(tmptitle));
	
	if (NULL == x)
		return 0;

	outgoing = (x->accessed & FH_INND) || strchr(x->owner, '.');
	if (readuservalue(currentuser->userid, "myreply_mode", buf, sizeof (buf)) >=
	    0)
		myreply_mode = atoi(buf);
	printf("<div id=\"con_%d_box\" class=\"con_box_%d\">", 
			x->filetime, contentbg);
	printf("<script>con_show_connav(%d, %d,'M.%d.A', %d, %d, %d, %d, %d, "
		"%d, %d, -1, -1, '', '', 0, 0, %d, %d, %d, %d, 1);"
		"</script>",
		num, myreply_mode,x->filetime, x->thread, getbnumx(brd), 
		(!strncmp(currentuser->userid, x->owner, IDLEN + 1) || 
		 	has_BM_perm(currentuser, brd)) ? 1: 0, 
		x->staravg50, x->hasvoted, st, x->filetime == x->thread,
		brd->header.flag & INNBBSD_FLAG, 
		x->accessed & FH_NOREPLY, outgoing, 
		brd->header.flag & ANONY_FLAG);
	printf("<script>con_new_title(%d, '%s');</script>\n", 
			x->filetime, tmptitle);

	printf("<div id=\"con_%d_content_box\" class=\"con_content_box_%d\">"
		"<div id=\"con_%d_content_msg\" class=\"con_content_msg_%d\">"
		"</div>", 
			x->filetime, contentbg, 
			x->filetime, contentbg);
	if(w_info->mypic_mode == 0)
		printmypicbox(fh2owner(x));
	fshow_file(stdout, brd->header.filename, x);
	// show article link
	printf("<div id=\"con_%d_content_link\" class=\"con_content_link_%d\""
			"><div class=\"con_content_link_padding_%d\">", 
			x->filetime, contentbg, contentbg);
	bbscon_show_article_link(getbnumx(brd), fh2fname(x));
	printf("</div></div></div>");
	// show article link end
	printf("<script>con_show_connav(%d, %d,'M.%d.A', %d, %d, %d, %d, %d, "
		"%d, %d, -1, -1, '', '', 0, 0, %d, %d, %d, %d, 0);"
		"con_hide_load(%d, 1);"
		"</script>\n",
		num, myreply_mode,x->filetime, x->thread, getbnumx(brd), 
		(!strncmp(currentuser->userid, x->owner, IDLEN + 1) || 
		 	has_BM_perm(currentuser, brd)) ? 1: 0, 
		x->staravg50, x->hasvoted, st, x->filetime == x->thread, 
		brd->header.flag & INNBBSD_FLAG,
		x->accessed & FH_NOREPLY, outgoing, 
		brd->header.flag & ANONY_FLAG, x->filetime);
	printf("<div id=\"con_%d_reply\" class=\"con_reply_%d\" "
		"style=\"display: none;\">\n"
		"</div>\n",
		x->filetime, contentbg);
	printf("</div><div class=\"clear\">&nbsp;</div>");
	brc_add_read(x);
	changeContentbg();
	return 0;
}

int bbstopic_main()
{
	char board[80], dir[80], boardtitle[80], *path, *ptr;
	struct fileheader *x = NULL; struct fileheader *firstarticle = NULL;
	struct boardmem *x1;
	int num = 0, firstnum = 0, found = 0, start, total;
	int thread, filetime0 = 0;
	struct mmapfile mf = { ptr:NULL };

	path = getsenv("SCRIPT_URL");
	if (strstr(path, "..") || strstr(path, "//"))
		http_fatal("错误的路径 1。");
	if (!(path = strchr(path + 1, '/')))
		http_fatal("错误的路径 2。");
	if (!strncmp(++path, "topic/", 6)) {
		path += 6;
		if (!(ptr = strchr(path, '/')))
			http_fatal("错误的路径 3。");   //path和ptr之间应该是版面的编号。
		*ptr = 0;
		parm_add("B", path);  //取出版面
		path = ptr + 1;
		if (!(ptr = strchr(path, '.')))
			http_fatal("错误的路径 4。");
		*ptr = 0;
		parm_add("th", path);  //取出文件系列
	}
	getparmboard(board, sizeof (board));
	thread = atoi(getparm("th"));

	html_header(5);
	if ((x1 = getboard(board)) == NULL)
		http_fatal("错误的讨论区");
	strsncpy(boardtitle, 
		void1((unsigned char*) titlestr(x1->header.title)), 40);
	start = atoi(getparm("start"));
	brc_initial(currentuser->userid, board);
	sprintf(dir, "boards/%s/.DIR", board);
	MMAP_TRY {
		if (mmapfile(dir, &mf) == -1) {
			MMAP_UNTRY;
			http_fatal("目录错误");
		}
		total = mf.size / sizeof (struct fileheader);
		if (start <= 0 || start > total) {
			start =
			    Search_Bin((struct fileheader *) mf.ptr, thread, 0,
				       total - 1);
			if (start < 0)
				start = -(start + 1);
		} else {
			start--;
		}
		ismozilla = testmozilla();

		firstarticle = (struct fileheader *) (mf.ptr + start * sizeof (struct fileheader));
		if (firstarticle->thread != thread) {
			MMAP_UNTRY
			http_fatal("找不到这个系列");
		}
		printf("<script src=\"" BBSCONJS "\" type=\"text/javascript\"></script>\n"
			"<script src=\"" BBSAJAXJS "\" type=\"text/javascript\"></script>\n"
			"<script src=\"" BBSCOOKIEJS "\" type=\"text/javascript\"></script>\n");
		printf("<script>var baseurl='/HT/';</script>");
			//temporarily added by Fishingsnow
			//should be update to a solid one.
		printf("<title>%s</title>", scriptstr(firstarticle->title));
		printf("</head>\n");
		printf("<body onload=\"con_check_width();\">\n" WWWLEFT_DIV);

		printf("<script>var defaultsmagic = '%s';</script>", SMAGIC);
		
		printf("<script>con_show_head('" MY_BBS_NAME "', "
				"'%s', '%c', '%s', %d, %d);</script>\n",
				nohtml(getsectree(x1->header.sec1)->title),
				x1->header.sec1[0], boardtitle, getbnumx(x1), 
				atoi(getparm("redirect")));
	#ifdef ENABLE_MYSQL
		printf("<script>var enmysql = 1;</script>");
	#else
		printf("<script>var enmysql = 0;</script>");
	#endif
		printf("<div id=\"con_main\">\n<div id=\"con_main_left\">");
		for (num = start; num < total; num++) {
			x = (struct fileheader *) (mf.ptr + 
				num * sizeof (struct fileheader));
			if (x->thread != thread) {
				continue;
			}
			show_file(x1, x, num, 2);
			if (!found) {
				found = 1;
				firstnum = num - 1;
				filetime0 = x->filetime;
			}
		}
		printf(	"<div id=\"con_doclist_nav\">\n"
			"<div id=\"con_doclist_nav_padding\">\n"
			"</div></div>\n"
			"<div id=\"con_doclist_ajax\">\n"
			"<div id=\"con_doclist_ajax_padding\">\n"
			"</div></div>\n"
			"<div class=\"clear\"></div>"
			"<div id=\"con_doclist_content\"></div>"
			"<script>"
			"var tzdiff = (new Date())."
			"getTimezoneOffset()*60 + 8*3600;\n"
			"var today = new Date((%d + tzdiff)*1000);\n"
			"var board = %d;\n"
			"var ftime = %d;\n"
			"con_docnav_init(%d, %d, -1, -1, 2);"
			"con_doclist_head('con_doclist_content');"
			"con_doclist_end('con_doclist_content');"
			"</script>", 
			(int)time(NULL), getbnumx(x1), thread, 
			filetime0, CON_MAX_OTHERTHREAD);
		printf("<script>con_doclist_update(%d, %d, %d, 1);"
			"</script>", start - 5, x->thread, 
			CON_MAX_OTHERTHREAD);
		printf("</div>");
		con_print_main_right(x1, x);
		printf("</div><div class=\"clear\"></div><br><br>");
	}
	MMAP_CATCH {
		found = 0;
	}
	MMAP_END mmapfile(NULL, &mf);
	if (found == 0)
		http_fatal("错误的文件名");
	processMath();
	printf("<script>con_switch_right_bycookie();</script>");
	showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
	printf(WWWFOOT_DIV "</body></html>\n");

	brc_update(currentuser->userid);
	http_quit();
	return 0;
}