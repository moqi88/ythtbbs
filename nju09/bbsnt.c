#include "bbslib.h"
int
bbsnt_main()
{
	char board[80], dir[80], boardtitle[40], *pathparm, *ptr;
	struct fileheader *x, *x2;
	struct boardmem *x1;
	int num = 0, found = 0, start, total, thread, num2;
	struct mmapfile mf = { ptr:NULL };
	struct fileheader *other_thread[CON_MAX_OTHERTHREAD];
	int otnum = 0;

	html_header(1);
	pathparm = getsenv("SCRIPT_URL");
	if (strstr(pathparm, "..") || strstr(pathparm, "//"))
		http_fatal("错误的路径 1。");
	if (!(pathparm = strchr(pathparm + 1, '/')))
		http_fatal("错误的路径 2。");
	if (!strncmp(++pathparm, "bbsnt_", 6)) {
		ptr = pathparm + 6;
		if (!(pathparm = strchr(pathparm + 6, '_')))
			http_fatal("错误的路径 3。");
		*(pathparm++) = 0;
		parm_add("B", ptr);
		ptr = pathparm;
		if (!(pathparm = strchr(pathparm, '.')))
			http_fatal("错误的路径 4。");
		*(pathparm) = 0;
		parm_add("th", ptr);
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
//	printf("<body onload=\"con_check_width();\">\n");
//修改框架 增加左侧
	printf("<body topmargin=0 leftMargin=1 MARGINWIDTH=1 MARGINHEIGHT=0 onload=\"con_check_width();\">\n" WWWLEFT_DIV);
	printf("<script>var defaultsmagic = '%s';</script>", SMAGIC);
	if ((x1 = getboard(board)) == NULL)
		http_fatal("错误的讨论区");
	strsncpy(boardtitle, 
		void1((unsigned char *) titlestr(x1->header.title)), 40);
	printf("<script>con_show_head('" MY_BBS_NAME "', '%s', '%c', '%s', "
		"%d, %d);</script>", 
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
		printf("<div id=\"con_main\">\n"
				"<div id=\"con_main_left\">");
		ismozilla = testmozilla();
		x = (struct fileheader *) (mf.ptr +
					   start * sizeof (struct fileheader));
		for (num = start; num < total; num++, x++) {
			if (x->thread != thread) {
				continue;
			}
			show_file(x1, x, num, 3);
			found = 1;
			break;
		}
		if (found) {
			char *lasttitle = "";
	/*		printf("<div id=\"con_rel_title\">"
				"<a href=\"bbstcon?B=%d&start=%d&th=%d\" "
				"name=\"查看所有跟贴\">所有根贴全部展开</a>"
				"</div>\n", getbnumx(x1), num, thread);*/
			printf("<div id=\"con_doclist_nt_nav\">"
				"同主题文章索引</div>"
				"<div id=\"con_doclist_nt\"></div><br><br>"
				"<script>\n"
				"var tzdiff = (new Date())."
				"getTimezoneOffset()*60 + 8*3600;\n"
				"var today = new Date((%d + tzdiff)*1000);\n"
				"var board = %d;\n"
				"var ftime = 0;\n"
				"con_doclist_head('con_doclist_nt');\n", 
				(int)time(NULL), getbnumx(x1));
			x2 = x;
			num2 = num;
			num = max(num - 5, 0);
			x = (struct fileheader *) (mf.ptr + 
					num * sizeof (struct fileheader));
			for (; num < total; num++, x++) {
				if (x->thread != thread)
					continue;
				printf("con_doclist_item(%d,'%s','%s','%s',"
					"%d,", num + 1,
				       flag_str2(x->accessed, !brc_un_read(x)),
				       fh2owner(x), fh2fname(x), feditmark(x));
				if (strcmp(x->title, lasttitle)) {
					printf("'%s',", scriptstr(x->title));
					lasttitle = x->title;
				} else
					printf("'',");
				printf("%d,%d,%d);\n", bytenum(x->sizebyte),
				       x->staravg50 / 50, x->hasvoted);
			}
			if (otnum < CON_MAX_OTHERTHREAD - 1)
				other_thread[otnum] = NULL;
			printf("con_doclist_end('con_doclist_nt');\n"
				"</script>\n"
				"<div id=\"con_doclist_nav\">\n"
				"<div id=\"con_doclist_nav_padding\">\n"
				"</div></div>\n"
				"<div id=\"con_doclist_ajax\">\n"
				"<div id=\"con_doclist_ajax_padding\">\n"
				"</div></div>\n"
				"<div class=\"clear\"></div>"
				"<div id=\"con_doclist_content\"></div>"
				"<script>con_docnav_init(%d, %d, -1, -1, 2);"
				"con_doclist_head('con_doclist_content');"
				"con_doclist_end('con_doclist_content');"
				"var ftime = %d;\n"
				"</script>", 
				x2->filetime, CON_MAX_OTHERTHREAD, x2->thread);
			printf("<script>con_doclist_update(%d, %d, %d, 1);"
				"</script>", num2 - 5, x2->filetime, 
				CON_MAX_OTHERTHREAD);
			printf("</div>"); 	//for con_main_left
			con_print_main_right(x1, x2);
			printf("</div><div class=\"clear\"></div>"
				"<br><br><br>");	
				//for con_main
		}
	}
	MMAP_CATCH {
		found = -1;
	}
	MMAP_END mmapfile(NULL, &mf);
	if (found == -1) {
		printf("文件列表更新例外, 请"
		       "<a href=# onclick='javascript:{location=location;return false;}'>刷新</a>"
		       "<script>setTimeout('location.reload()', 1000);</script>");
		http_quit();
		return 0;
	}
	if (found == 0)
		http_fatal("错误的文件名");
	processMath();
	printf("<script>con_switch_right_bycookie();</script>");
	showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
//	printf("</body>\n");
//修改框架 增加底部
	printf(WWWFOOT_DIV "</body></html>\n");

	brc_update(currentuser->userid);
	http_quit();
	return 0;
}
