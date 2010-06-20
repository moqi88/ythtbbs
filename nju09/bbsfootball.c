#include "bbsfootball.h"

int bbsfb_admin_addteam() {
	struct graphheader *ptr;
	struct team save;
	char posstr[10], buf[10];
	int i, num;

	strncpy(posstr, getparm("posstr"), sizeof(posstr));
	ptr = bbsfb_graph_search(posstr);
	num = atoi(getparm("tnum"));

	if (NULL == ptr || strlen(posstr) >= 9) {
		redirect("bbsfb?A=9");
		return 0;
	}
	
	if ((ptr->main->team1 && ptr->main->team2) || ptr->main->team) {
		bbsfb_uninit_all();
		http_fatal("不能在当前位置添加球队。");
		return 0;
	}
	
	bzero(&save, FB_TSIZE);
	strncpy(save.name, getparm("tname"), sizeof(save.name));
	strncpy(save.univ, getparm("tuniv"), sizeof(save.univ));
	for (i = 0; i < 12; i++) {
		snprintf(buf, sizeof(buf), "tmember%d", i + 1);
		strncpy(save.member[i], getparm(buf), sizeof(save.member[0]));
	}
	strncpy(save.posstr, posstr, sizeof(save.posstr));
	num = bbsfb_team_add(&save, num);
	if (num && ptr->team1) 
		ptr->team2 = num;
	else if (num)
		ptr->team1 = num;
	bbsfb_graph_save();
	return 1;
}

int bbsfb_admin_addgraph() {
	char posstr[10], name[20];
       
	strncpy(posstr, getparm("posstr"), sizeof(posstr));
	strncpy(name, getparm("graphname"), sizeof(name));
	bbsfb_graph_add(posstr, name);
	bbsfb_graph_uninit();
	return 1;
}

int bbsfb_admin_delteam() {
	int num;

	num = atoi(getparm("tnum"));
	bbsfb_team_del(num);
	return 1;
}

int bbsfb_admin_delgraph() {
	char posstr[10];

	strncpy(posstr, getparm("posstr"), sizeof(posstr));
	bbsfb_graph_del(posstr);
	return 1;
}

int bbsfb_admin_renamegraph() {
	char posstr[10];

	strncpy(posstr, getparm("posstr"), sizeof(posstr));
	bbsfb_graph_rename(posstr);
	return 1;
}

int bbsfb_admin_auto() {
	char posstr[10];
	int result, total;
	
	strncpy(posstr, getparm("posstr"), sizeof(posstr));
	result = atoi(getparm("result"));
	total = atoi(getparm("total"));
	bbsfb_graph_auto(result, total, posstr);
	return 1;
}

int bbsfb_admin_refresh_graph() {
	FILE *fp;
	char buf[STRLEN];
	struct graphheader *ptr;

	ptr = &graphroot;
	if (!ptr)
		return 0;
	sprintf(buf, MY_BBS_HOME "/wwwtmp/fb_graph%c", sec);
	if ((fp = fopen(buf, "w")) == NULL)
		return 0;
	bbsfb_graph_show(fp, ptr->main->posstr, 0);
	fclose(fp);
	return 1;
}

int bbsfb_admin_import_teams() {
	char *content;
	int start;

	content = getparm("content");
	start = atoi(getparm("start"));
	bbsfb_team_import(content, start);
	return 1;
}

int bbsfb_admin_report() {
	bbsfb_report_caculate();
	http_quit();
	return 1;
}

int bbsfb_admin_addreport() {
	bbsfb_report_add();
	return 1;
}

int bbsfb_admin_index() {
	printf("<script>\nadmin_print_title('%c');</script>", sec);
	printf("<div id=\"bbsfb_graph_main\"></div>");
	printf("<div id=\"bbsfb_admin_main\"></div>");
	bbsfb_graph_show(stdout, NULL, 1);
	return 1;
}

int bbsfb_admin(void) {
	int type = atoi(getparm("T"));
	char url[STRLEN];

	if (!USERPERM(currentuser, PERM_SYSOP) && 
			!file_has_word(FB_ADMINDATA, currentuser->userid))
		http_fatal("Sorry, admin actions are not allowed to you.");
	bbsfb_init_all();
	switch(type) {
		default:
		case 0:
			bbsfb_admin_index();
			break;
		case 1:
			bbsfb_admin_addgraph();
			break;
		case 2:
			bbsfb_admin_delgraph();
			break;
		case 3:
			bbsfb_admin_renamegraph();
			break;
		case 4:
			bbsfb_admin_addteam();
			break;
		case 5:
			bbsfb_admin_delteam();
			break;
		case 6:
			bbsfb_admin_auto();
			break;
		case 7:
			bbsfb_admin_refresh_graph();
			break;
		case 8:
			bbsfb_admin_import_teams();
			break;
		case 9:
			bbsfb_admin_report();
			break;
		case 10:
			bbsfb_admin_addreport();
			break;
	}

	bbsfb_uninit_all();
	if (type) { 
		sprintf(url, "bbsfb?A=9&G=%c", sec);
		redirect(url);
	}
	return 1;
}


int bbsfb_index(void) {
	printf("<script>bbsfb_index();</script>\n");
	printf("<div id=\"bbsfb_graph_main\"></div>");
	showfile("wwwtmp/fb_title");
	http_quit();
	return 1;
}

int bbsfb_show(void) {
	char buf[STRLEN];

	strncpy(buf, getparm("G"), sizeof(buf));
	if (!*buf)
		return 0;
	printf("<div id=\"bbsfb_graph_main\"></div>");
	printf("<div id=\"bbsfb_graph__right_up\"></div>");
	snprintf(buf, sizeof(buf), "wwwtmp/fb_graph%c", *buf);
	showfile(buf);
	http_quit();
	return 1;
}


int bbsfootball_main(void) {
	int action;

	action = atoi(getparm("A"));
	sec = *(getparm("G"));
	html_header(1);
	printf("<link href=\"/football.css\" "
			"type=\"text/css\" rel=\"stylesheet\">"
			"<script src=" CSSPATH "football.js>"
			"</script>\n</head>\n<body>");

	switch (action) {
		default:
		case 0:
			bbsfb_index();
			break;
		case 1:
			bbsfb_show();
			break;
		case 9:
			bbsfb_admin();
			break;
	}
	http_quit();
	return 1;
}
