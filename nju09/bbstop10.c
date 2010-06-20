#include "bbslib.h"

int
bbstop10_main()
{
	struct mmapfile mf = { ptr:NULL };
	char *sec, *ptr, path[256];

	sec = getparm("sec");
	if(strlen(sec) == 0)
		sprintf(path, "wwwtmp/ctopten");
	else if(isalnum(sec[0])) {
		if(isalpha(sec[0]))
			sec[0] = toupper(sec[0]);
		ptr = strchr(sectree.seccodes, sec[0]);
		if(ptr == NULL)
			http_fatal("不存在的分区");
		
		sprintf(path, "wwwtmp/%c_topten", sec[0]);
	} else
		http_fatal("不存在的分区");
	
	html_header(1);
	//check_msg();
	printf("<body topmargin=0 leftMargin=1 MARGINWIDTH=1 MARGINHEIGHT=0>" WWWLEFT_DIV);
	MMAP_TRY {
		if (mmapfile(path, &mf) < 0) {
			MMAP_UNTRY;
			http_fatal("文件错误");
		}
		fwrite(mf.ptr, mf.size, 1, stdout);
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);

	printf(WWWFOOT_DIV "</body></html>\n");
	http_quit();
	return 0;
}
