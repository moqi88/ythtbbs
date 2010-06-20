#include "bbslib.h"

#define SEO_PAGE_SIZE 40

int seo_show_all(int debug) {
	int i;
	struct boardmem *x;

	printf("<meta name=\"keywords\" content=\"" MY_BBS_ID " " 
			BBS_WWW_INTRO "\">\n");
	printf("<meta name=\"description\" content=\"" MY_BBS_NAME "\">\n");
	printf("<title>" MY_BBS_NAME " 网站地图</title>\n");
	printf("</head><body>\n");
	if (!debug)
		printf("<script>location.replace('/" SMAGIC 
			"/boa?secstr=&redirect=1');</script>\n");
	printf("<h1>"MY_BBS_NAME" 全部版面列表</h1>");
	printf("<table>\n");

	for (i = 0; i < MAXBOARD; i++) {
		x = &(shm_bcache->bcache[i]);
		if (x->header.filename[0] <= 32 || x->header.filename[0] > 'z')
			continue;
		if (!has_view_perm_x(currentuser, x))
			continue;
		printf("<tr><td>%s</td>"
				"<td><a href=\"%d/0.htm\">%s</a></td>"
				"<td>%s</td></tr>\n",
				x->header.filename, i, x->header.title, 
				getboardaux(i)->intro);
	}
	printf("</table>");
	return 1;
}

int seo_show_board(int bnum, int offset, int debug) {
	int i, total, count = 0;
	struct boardmem *x;
	struct mmapfile mf = { ptr:NULL };
	struct fileheader *fhdr;
	char buf[STRLEN];

	if (bnum < 0 || bnum >= MAXBOARD)
		return 0;
	
	x = &(shm_bcache->bcache[bnum]);
	if (!x || x->header.filename[0] <= 32 ||
			x->header.filename[0] > 'z' || 
			!has_view_perm_x(currentuser, x))
		return 0;
	
	printf("<meta name=\"keywords\" content=\"" MY_BBS_ID " " 
			BBS_WWW_INTRO "\">\n");
	printf("<meta name=\"description\" content=\"" MY_BBS_NAME 
			" %s 版\"> ", x->header.title);
	printf("<title>" MY_BBS_NAME " - %s </title>\n", x->header.title);
	printf("</head><body>");
	if (!debug)
		printf("<script>location.replace('/" SMAGIC 
			"/doc?B=%d&redirect=1')</script>", bnum);

	offset = offset >= 0 ? offset : 0;

	printf("当前位置：<a href='/" SMAGIC "/s/'>" 
			MY_BBS_ID "</a> &gt;&gt;""<b>%s (%s)</b>", 
			x->header.title, x->header.filename);

	printf("<h1><b>%s</b>(<b>%s</b>)"
			"</h1><br>\n", x->header.title, x->header.filename);
	if (getboardaux(bnum)->intro)
		printf("<h2>%s</h2>\n", getboardaux(bnum)->intro);
	printf("<hr>\n");

	printf("<table>");

	sprintf(buf, MY_BBS_HOME "/boards/%s/.DIR", x->header.filename);

	mmapfile(NULL, &mf);
	MMAP_TRY {
		if (mmapfile(buf, &mf) < 0) {
			MMAP_UNTRY;
			return 0;
		}
		total = mf.size / sizeof(struct fileheader);
		if (total < 0) {
			MMAP_UNTRY;
			return 0;
		}

		if ((offset + 1) * SEO_PAGE_SIZE < total) 
			printf("<a href=\"%d.htm\">上一页</a> ", 
					offset + 1);
		if (offset != 0)
			printf("<a href=\"%d.htm\">下一页</a> ",
					offset - 1);
		printf("<a href=\"/" SMAGIC "/s/\">返回版面列表</a>");


		for (i = total - 1; i >= 0; i--) {
			fhdr = (struct fileheader *) 
				(mf.ptr + i * sizeof(struct fileheader));

			//if (fhdr->thread != fhdr->filetime) 
			//	continue;
			
			count++;
			if (count < offset * SEO_PAGE_SIZE ||
					count > (offset + 1) * SEO_PAGE_SIZE)
				continue;

			printf("<tr><td><a href=\"/HT/con_%d_M.%d.A.htm\">"
					"%s</a></td>"
					"<td><a href=\"qry?U=%s\">%s</a>"
					"</td><td>%s</td></tr>\n",
					bnum,
					fhdr->filetime, fhdr->title, 
					fhdr->owner[0] == '.' ? 
					"Anonymous" : fhdr->owner, 
					fhdr->owner[0] == '.' ?
					"Anonymous" : fhdr->owner,
					Ctime((time_t) (fhdr->filetime)));
		}
	} MMAP_CATCH {
	} MMAP_END mmapfile(NULL, &mf);

	printf("</table>");

	return 1;
	
}

int seo_show_article_content(char *buf, int filetime) {
	struct mmapfile mf = { ptr:NULL };
	char filename[20];

	if (mmapfile(buf, &mf) < 0)
		return 0;
	sprintf(filename, "M.%d.A", filetime);
	MMAP_TRY{
		mem2html(stdout, mf.ptr, mf.size, filename, YEA, 0);

	} MMAP_CATCH {
	} MMAP_END mmapfile(NULL, &mf);

	printf("<br /><hr />");

	return 1;
}

int seo_show_article(int bnum, int filetime, int debug) {
	int i, total, title = 1;
	struct boardmem *x = &(shm_bcache->bcache[bnum]);
	struct mmapfile mf = { ptr:NULL };
	struct fileheader *fhdr;
	char buf[STRLEN];

	if (!x || x->header.filename[0] <= 32 ||
			x->header.filename[0] > 'z' || 
			!has_view_perm_x(currentuser, x))
		return 0;

	sprintf(buf, MY_BBS_HOME "/boards/%s/.DIR", x->header.filename);

	mmapfile(NULL, &mf);
	MMAP_TRY {
		if (mmapfile(buf, &mf) < 0) {
			MMAP_UNTRY;
			return 0;
		}
		total = mf.size / sizeof(struct fileheader);
		if (total < 0) {
			MMAP_UNTRY;
			return 0;
		}

		for (i = 0; i < total; i++) {
			fhdr = (struct fileheader *) 
				(mf.ptr + i * 
				sizeof(struct fileheader));

			if (fhdr->thread != filetime) 
				continue;

			sprintf(buf, MY_BBS_HOME "/boards/%s/M.%d.A", 
					x->header.filename, fhdr->filetime);
	
			if (title) {
				printf("<meta name=\"keywords\" "
					"content=\"" MY_BBS_ID " " 
					BBS_WWW_INTRO "\">\n");
				printf("<meta name=\"description\" "
					"content=\"" MY_BBS_NAME 
					" %s 版\"> ", x->header.title);
				printf("<title>" MY_BBS_NAME " - %s "
					"</title>\n", fhdr->title);
				printf("</head><body>");
				if (!debug)
					printf("<script>location.replace('"
					"/" SMAGIC "/con_%d_M.%d.A.htm?"
					"redirect=1');</script>", 
					bnum, fhdr->filetime);
				printf("当前位置：<a href='/" SMAGIC 
					"/s/'>" MY_BBS_ID "</a> &gt;&gt; "
					"<a href='0.htm'>%s</a> "
					"(<a href='0.htm'>%s</a>) &gt;&gt; "
					"<b>%s</b>",
					x->header.title, x->header.filename, 
					fhdr->title);
	
				printf("<h1>%s</h1><br>\n"
					"<h2>%s</h2>\n"
					"<hr />\n", 
					fhdr->title,
					getboardaux(bnum)->intro);
				title = !title;
			}

			seo_show_article_content(buf, fhdr->filetime);
		}
	} MMAP_CATCH {
	} MMAP_END mmapfile(NULL, &mf);

	if (file_exist(MY_BBS_HOME "/wwwtmp/googleads"))
		showfile(MY_BBS_HOME "/wwwtmp/googleads");

	return 1;
}

int bbsseo_main(void) {
	char *path_info, *ptr, *agent;
	int offset = 0, bnum = -1, filetime = -1, debug;
	char tmp[1024];

	path_info = getsenv("SCRIPT_URL");

	/*
	 * /SMAGIC/s/0.1/1234567890.html
	 */

	path_info = strchr(path_info + 1, '/');
	debug = atoi(getparm("debug"));


	if (NULL == path_info)
		http_fatal("Fatal Error 1.");

	if (!strncmp(path_info, "/s/", 3))
		path_info += 3;
	else
		http_fatal("Fatal Error 2.");

	if (*path_info) {
		bnum = atoi(path_info);
		ptr = strchr(path_info, '/');
		if (ptr && *(++ptr)) {
			filetime = atoi(ptr);
			if (filetime <= 1000) {
				offset = filetime;
				filetime = -1;
			}
		}
	}

	html_header(1);
	if (bnum == -1)
		seo_show_all(debug);
	else if (filetime == -1) 
		seo_show_board(bnum, offset, debug);
	else{
		agent=getsenv("HTTP_USER_AGENT");
		if(agent[0] && (strcasestr(agent,"windows")||
				strcasestr(agent,"linux")||
				strcasestr(agent,"macintosh"))){
			snprintf(tmp, sizeof(tmp),"http://%s/HT/bbsindex?b=con_%d_M.%d.A.htm",
					MY_BBS_DOMAIN, bnum, filetime);
			redirect(tmp);
			return 0;
		}
		seo_show_article(bnum, filetime, debug);
	}
	printf("</body></html>");
	http_quit();
	return 0;
}
