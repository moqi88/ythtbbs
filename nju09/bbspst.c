#include "bbslib.h"

int
printquote(FILE *fp, char *filename, char *board, char *userid, int fullquote)
{
	int lines = 0, i;
	char buf[512];
	struct mmapfile mf = { ptr:NULL };
	struct memline ml;
	char *ptr, tmpuserid[IDLEN + 1];
	underline = 0;
	highlight = 0;
	lastcolor = 37;
	useubb = 1;
	if (mmapfile(filename, &mf) < 0)
		return -1;
	MMAP_TRY {
		if (*userid) {
			fprintf(fp, "\n\n【 在 %s 的大作中提到: 】\n", userid);
		} else {
			if (NULL != (ptr = memchr(mf.ptr, ' ', mf.size))) {
				strncpy(tmpuserid, ++ptr, sizeof(tmpuserid));
				if (NULL != (ptr = strchr(tmpuserid, ' ')))
					*ptr = 0;
					fprintf(fp, "\n\n【 在 %s 的"
						"大作中提到: 】\n", 
						tmpuserid);
			}
		}
		memlineinit(&ml, mf.ptr, mf.size);
		for (i = 0; i < 6; i++) {
			if (!memlinenext(&ml))
				break;
			if (ml.text[0] == '\r' || ml.text[0] == '\n')
				break;
		}
		while (memlinenext(&ml)) {
			if (ml.text[0] == '\n' || ml.text[0] == '\r')
				continue;
			if (ml.len > 4 && (!strncmp(ml.text, ": 【", 4)
					   || !strncmp(ml.text, ": : ", 4)))
				continue;
			if (ml.len == 3 && !strncmp(ml.text, "--\n", 3))
				break;
			if (ml.len > 10 && !strncmp(ml.text, "begin 644 ", 10))
				break;
			if (ml.len > 18
			    && !strncmp(ml.text, "beginbinaryattach ", 18))
				break;
			//用ml.len而不是ml.len+1来丢弃最后一个\n，而长行的最后反正是要丢弃的，没问题
			strsncpy(buf, ml.text, min(sizeof (buf), ml.len));
			if (!fullquote && strlen(buf) > min(7 - lines, 6) * 50)
				buf[min(7 - lines, 6) * 50] = 0;
			void1(buf);
			fprintf(fp, ": %s\n", nohtml_textarea(buf));
			lines += 1 + strlen(buf) / 50;
			if (!fullquote && lines >= 7) {
				fprintf(fp, ": (以下引言省略...)\n");
				break;
			}
		}
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);
	return 0;
}

void
printselsignature()
{
	int i, sigln, numofsig;
	char path[200];
	sprintf(path, "home/%c/%s/signatures",
		mytoupper(currentuser->userid[0]), currentuser->userid);
	sigln = countln(path);
	numofsig = (sigln + MAXSIGLINES - 1) / MAXSIGLINES;
	printf("签名档 <select name=\"signature\">\n");
	if (u_info->signature == 0 || u_info->signature > 11)
		printf("<option value=\"0\" selected>不用签名档</option>\n");
	else
		printf("<option value=\"0\">不用签名档</option>\n");
	for (i = 1; i <= numofsig && i <= 11; i++) {
		if (u_info->signature == i)
			printf
			    ("<option value=\"%d\" selected>第 %d 个</option>\n",
			     i, i);
		else
			printf("<option value=\"%d\">第 %d 个</option>\n", i,
			       i);
	}
	if (u_info->signature == -1)
		printf
		    ("<option value=\"-%d\" selected>随机签名档</option>\n",
		     numofsig);
	else
		printf("<option value=\"-%d\">随机签名档</option>\n", numofsig);
	printf("</select>\n");
	printf(" [<a target=_blank href=bbssig>查看签名档</a>]");
}

void
printnore()
{
	printf("<input type=checkbox name=nore>禁止回复");
}

void
printuploadattach()
{
//	printf("<script type='text/javascript' src='http://union.bokecc.com/ccplugin.bo?userID=13953&type=yjrg'></script>");
	printf(" [<a href=bbsupload target=uploadyjrg>添加/删除附件</a>]");
#if 0
	int i = u_info - &(shm_utmp->uinfo[0]);
	unsigned char buf[4];
	myitoa(i, buf, 3);
#ifndef USEBIG5
	printf
	    (" [<a href=/cgi-bin/upload/%s%s target=uploadyjrg>添加/删除附件</a>]",
	     buf, u_info->sessionid);
#else
	printf
	    (" [<a href=/cgi-bin/big5upload/%s%s target=uploadyjrg>添加/删除附件</a>]",
	     buf, u_info->sessionid);
#endif
#endif
}

void
printusemath(int checked)
{
	printf
	    ("<br><a href=home/boards/BBSHelp/html/itex/itexintro.html target=_blank>"
	     "使用Tex数学公式</a><input type=checkbox name=usemath%s>\n",
	     checked ? " checked" : "");
}

int
bbspst_main()
{
	int local_article, num, fullquote = 0, guestre = 0, thread = -1;
	char *ptr, userid[80], buf[512], path[512], file[512], board[512],
	    title[80] = "";
	struct fileheader *dirinfo = NULL;
	struct boardmem *x;
	struct mmapfile mf = { ptr:NULL };
	if (!strcmp(getparm("ajax"), "getquote")) {
	}
	html_header(1);
	printf("<script src=" CSSPATH "richtext.js></script>");
	//check_msg();
	//printf("</head><body topmargin=0 leftMargin=1 MARGINWIDTH=1 MARGINHEIGHT=0>" WWWLEFT_DIV);
	if (!loginok) {
		printf("匆匆过客不能发表文章，请先登录!<br>");
		printf
		    ("如果您确实已经登录却看到这个提示，请尝试了解登录时的 IP验证范围 选项<br>");
		printf("<script>openlog();</script>");
		http_quit();
	}
	local_article = atoi(getparm("la"));
	getparmboard(board, sizeof (board));
	if (*(ptr = getparm("M"))) {
		int mode = atoi(ptr) ? 1 : 0;
		if (mode != w_info->edit_mode) {
			w_info->edit_mode = mode;
			sprintf(buf, "%d", mode);
			saveuservalue(currentuser->userid, "edit_mode", buf);
		}
	}
	strsncpy(file, getparm("F"), 20);
	if (!file[0])
		strsncpy(file, getparm("file"), 20);
	fullquote = atoi(getparm("fullquote"));
	if (file[0] != 'M' && file[0])
		http_fatal("错误的文件名");
	if (!(x = getboard(board)))
		http_fatal("错误的讨论区或者您无权在此讨论区发表文章");
	if (njuinn_board(board) && !innd_board(board))
		local_article = 1;
	if (!has_post_perm(currentuser, x) && !isguest)
		http_fatal("错误的讨论区或者您无权在此讨论区发表文章");
	if (noadm4political(board))
		http_fatal("对不起,因为没有版面管理人员在线,本版暂时封闭.");
	if (x->ban == 2)
		http_fatal("对不起, 因为版面文章超限, 本版暂时关闭.");

	if (file[0]) {
		num = atoi(getparm("num"));
		sprintf(path, "boards/%s/.DIR", board);
		MMAP_TRY {
			if (mmapfile(path, &mf) == -1) {
				MMAP_UNTRY;
				http_fatal("错误的讨论区");
			}
			dirinfo = findbarticle(&mf, file, &num, 1);
		}
		MMAP_CATCH {
			dirinfo = NULL;
		}
		MMAP_END mmapfile(NULL, &mf);
		if (dirinfo) {
			thread = dirinfo->thread;
			if (dirinfo->accessed & FH_ALLREPLY)
				guestre = 1;
			strsncpy(userid, fh2owner(dirinfo), 20);
			if (strncmp(dirinfo->title, "Re: ", 4))
				snprintf(title, 60, "Re: %s", dirinfo->title);
			else
				strsncpy(title, dirinfo->title, 60);
		} else
			http_fatal("错误的文件名");
		if (dirinfo->accessed & FH_NOREPLY)
			http_fatal("本文被设为不可Re模式");

	}
	if (isguest && !guestre) {
		printf("匆匆过客不能发表文章，请先登录!<br><br>");
		printf("<script>openlog();</script>");
		http_quit();
	}
	changemode(POSTING);
	printf("<script>\nfunction submitForm(obj) {\n"
			"if (document.getElementById('form1title').value.length == 0) {\n"
			"alert('文章要有一个标题。');\n"
			"return false;}\n"
			"obj.disabled = true;\n"
			"obj.value = '文章提交中';\n"
			"updateRTE('rte1');\n"
		       "document.form1.text.value=html2ansi(document.form1.rte1.value);\n"
		       "document.form1.submit();"
		       "return false;}\n"
		       "initRTE('/images/', '/', '');\n"
		       "</script>\n");
	//printf("<body onLoad='enableDesignMode(\"rte1\", ansi2html(document.form1.text.value), false);'>\n");
	//printf("</head><body topmargin=0 leftMargin=1 MARGINWIDTH=1 MARGINHEIGHT=0>" WWWLEFT_DIV);
	// 只能注释掉左侧，否则IE下richtext会出问题，没有发文框，这可能是js也可能是iframe的问题，原因不明。
	printf("</head><body>");
	printf("<div class=swidth style=\"height:99%%\">");
	printf("<table height=100%% width=100%% border=0><tr height=20><td>");	//大表
	printf("%s -- 发表文章于 %s 讨论区\n", BBSNAME, board);
	strsncpy(buf, getsenv("QUERY_STRING"), sizeof (buf));
	if ((ptr = strstr(buf, "&M=")))
		*ptr = 0;
	if (w_info->edit_mode == 0) {
		printf
		    ("<script>if(isRichText) document.write(' [<a href=bbspst?%s&M=1 class=red>切换到所见即所得编辑器</a>');</script>]",
		     buf);
	} else {
		printf(" [<a href=bbspst?%s&M=0>切换到简单编辑器</a>]", buf);
		printf
		    ("<script>if(isRichText!=true||!testReplace()) location.replace('bbspst?%s&M=0')</script>",
		     buf);
	}
	if (x->header.flag & IS1984_FLAG)
		printf
		    ("<br><font color=red>请注意，本文发表后需通过审查</font>");
	printf("</td></tr><tr><td valign=top>");	//大表
	printf("<table border=1 width=100%% height=100%%>");	//中表
	if (file[0])
		snprintf(buf, sizeof (buf), "&ref=%s&rid=%d", file, num);
	//form 放在 table 跟 tr 之间，否则显示不好
	printf("<form name=form1 method=post action=bbssnd?B=%d&th=%d%s>\n",
	       getbnumx(x), thread, file[0] ? buf : "");
	printf("<tr height=40><td>");	//中表
	printf
	    ("使用标题: <input id=form1title type=text name=title size=50 maxlength=100 value='%s'>",
	     void1(nohtml(title)));
	if (innd_board(board) || njuinn_board(board))
		printf
		    (" <font class=red>转信</font><input type=checkbox name=outgoing %s>\n",
		     local_article ? "" : "checked");
	if (anony_board(board))
		printf("匿名<input type=checkbox name=anony %s>\n",
		       strcmp(board, DEFAULTANONYMOUS) ? "" : "checked");

	printf("<table width=100%%><tr><td>");
	printselsignature();
	printnore();
	printusemath(0);
	printf("</td><td align=right>");
	printuploadattach();
	printf("</td></tr></table>");

	if (!w_info->edit_mode) {
		printubb("form1", "text");
		printf("<br>");
	}
	if (file[0]) {
		printf("引文模式: %s ", fullquote ? "完全" : "精简");
		printf
		    ("[<a target=_self href=bbspst?inframe=1&B=%d&file=%s&num=%d&la=%d",
		     getbnumx(x), file, num, local_article);
		printf("&fullquote=%d>切换为%s模式</a> (将丢弃所更改内容)]<br>",
		       !fullquote, (!fullquote) ? "完全" : "精简");
	}
	printf("</td></tr><tr><td>");	//中表
	if (w_info->edit_mode) {
		printf
		    ("<div style='position:absolute;visibility:hidden;display:none'>"
	     "<textarea name=text onKeyDown='bbspst_ctrlenter(event);'>");
	} else
		printf
	    ("<br>\n<textarea "
		     "name=text rows=20 cols=76 wrap=virtual class=f2 "
		     "onKeyDown='bbspst_ctrlenter(event);'>\n\n");
	if (file[0]) {
		sprintf(path, "boards/%s/%s", board, file);
		printquote(stdout, path, board, userid, fullquote);
	}
	printf("</textarea>");
	if (w_info->edit_mode)
		printf("</div>");
	if (w_info->edit_mode) {
		printf
		    ("<script>writeRichText('rte1', ansi2html(document.form1.text.value),400,400,true,false);</script>");
	}
	printf("</td></tr><tr height=30><td class=post align=center>");	//中表
	if (w_info->edit_mode)
		printf
	    ("<input type=button value=发表 "
	     "id='bbspst_button_submit' onclick=\"submitForm(this);\">");
	else
		printf
		    ("<table width=100%%><tr><td align=center>"
		     "<input id='bbspst_button_submit' "
		     "type=submit value=发表 onclick=\"this.value='文章提交中，请稍候...';"
		     "this.disabled=true;form1.submit();\"> </td><td align=center>"
		     "<input type=reset value=清除 onclick='return confirm(\"确定要全部清除吗?\")'>"
		     "</td></tr></table>");
	printf("</td></tr></form></table>"	//中表
	       "</td></tr></table></div>");	//大表
	showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
//	printf("</body>");
//修改框架，页面底部
//	printf(WWWFOOT_DIV "</body></html>\n");
	printf("</body></html>\n");
	http_quit();
	return 0;
}

