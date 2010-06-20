#include "bbslib.h"

int
bbsmypic_main()
{
	char userid[sizeof (currentuser->userid)], filename[256];
	struct userec *x;
	struct mmapfile mf = { ptr:NULL };
	size_t size=0;
	
	strsncpy(userid, getparm2("U", "userid"), sizeof (userid));
	if (getuser(userid, &x) <= 0) {
		html_header(1);
		printf("啊，没这个人啊？！");
		http_quit();
	}
	sethomefile(filename, x->userid, "mypic");
	if (cache_header(file_time(filename), 3600*24*100)) {
		return 0;
	}
	MMAP_TRY {
		if (mmapfile(filename, &mf)) {
			strncpy(filename, HTMPATH "/defaultmypic.gif", sizeof(filename));
			if (mmapfile(filename, &mf)) {
				MMAP_UNTRY;
				http_fatal("错误的文件名");
			}
		}
		size = mf.size;
		printf("Content-type: %s\r\n", get_mime_type("mypic.gif"));
		printf("Content-Length: %d\r\n\r\n", mf.size);
		fwrite(mf.ptr, 1, mf.size, stdout);
	}
	MMAP_CATCH {
	}
	MMAP_END {
		mmapfile(NULL, &mf);
	}
	if(size<=0) {	//处理0字节错误的情况
		struct userec tmp;
		memcpy(&tmp, x, sizeof(tmp));
		tmp.mypic=0;
		updateuserec(&tmp, 0);
		unlink(filename);
	}
	return 0;
}

void
_printmypic(char *userid, int no_cache)
{
	char buf[80];
	int t;
	if(no_cache){
		sethomefile(buf, userid, "mypic");
		t = file_time(buf);
		sprintf(buf, "&t=%d", t);
	} else
		buf[0]=0;

	printf(	"<a href=\"/" SMAGIC "/bbsqry?U=%s\"><img src=\"/" SMAGIC "/mypic?U=%s%s&n=mypic.gif\""
		" onLoad='script:(this.width/this.height)>0.75?"
		"(this.width=(this.width>120?120:this.width)):"
		"(this.height=(this.height>160?160:this.height));' "
		"alt=%s的头像（在登录后左侧菜单中的个人工具箱内，可以增添或修改您的头像） border=0></a>", userid,userid,buf,userid);
}

void
printmypic_nocache(char *userid)
{
	_printmypic(userid, 1);
}

void
printmypic(char *userid)
{
	_printmypic(userid, 0);
}

int
printmypicbox(char *userid)
{
	int nobr = 0;
	struct userec *x;
	
	if (getuser(userid, &x) <= 0)
		return -1;
	printf("<div class=mypic>");
	printf ("<div align=center><a href=\"/"SMAGIC"/qry?U=%s\"><b>%s</b></a><br>",
	     x->userid, x->userid);
	if (x->mypic)
		printmypic(x->userid);
	else
		printf("<img src=\"/defaultmypic.gif\" alt=\"在登录后左侧菜单中的个人工具箱内，可以增添或修改您的头像\">");
	printf("</div><font class=f1>");
	printf("文章：<font color=blue>%d</font><br>"
			"生命：<font color=purple>%d</font>",
			 x->numposts, countlife(x));
	printf("<br>经验：<font color=fuchsia>%d+%d</font>", 
		 (x->exp_group=='U') ? 0 : countexp(x,0), 
		 (x->exp_group=='U')?0:countexp(x,1));
	printf("<br>等级：<font color=olive>%s</font>", 
		 cuserexp(x->exp_group, countexp(x,2)) );
#ifdef ENABLE_BLOG
	if(x->hasblog) {
		printf("<br><font color=blue><a href=\"blog?U=%s\">"
			"[Blog]</a></font> ", x->userid);
		nobr = 1;
	}
#endif
	if(USERPERM(x, PERM_SPECIAL8)) {
		printf("%s<font color=brown>"
			"<a href=\"bbs0an?path=/groups/GROUP_0/"
			"Personal_Corpus/%c/%s\">[文集]</a>", 
			nobr ? "&nbsp;&nbsp;" :"<br>",
			toupper(x->userid[0]), x->userid);
	}
	printf("</font></div>");
	return 0;
}
