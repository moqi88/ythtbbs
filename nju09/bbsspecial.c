#include "bbslib.h"


int showspecialall(const struct sectree *sec) {
	char path[256];
	
	if(sec->parent)
        	return 0;
	
	sprintf(path, "wwwtmp/special/special.html");
	return showfile(path);
}


int bbsspecial_main() {
	char buf[256], *stitle;

	changemode(READING);
        html_header(1);

	stitle =  getparm2("s", "special");
	if(strpbrk(stitle, "./\\?*"))
		http_fatal("没有这个专题");
	snprintf(buf, sizeof(buf), "wwwtmp/special/%s.htm", stitle);
	if(!file_exist(buf))
		http_fatal("没有找到文件");
		
	showfile(buf);
	return 0;
}
