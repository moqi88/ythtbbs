#include "bbslib.h"

int
bbsattach_main()
{
	char *ptr, *path_info;
	struct cgi_applet *a;
	char *ref;
	path_info = getsenv("SCRIPT_URL");
	path_info = strchr(path_info + 1, '/');
	ref = getsenv("HTTP_REFERER");
	if(ref[0] && !strcasestr(ref, MY_BBS_ID) && !strcasecmp(MY_BBS_ID, "YJRG")){
		printf("Status: 403\r\n\r\nSorry, we don't support for "
				"cross site link because of poor bandwidth.");
		return -1;
	}
	if (NULL == path_info)
		http_fatal("错误的文件名");
	if (!strncmp(path_info, "/attach/", 8))
		path_info += 8;
	else
		http_fatal("错误的文件名");
	ptr = strchr(path_info, '/');
	if (NULL == ptr)
		http_fatal("错误的文件名");
	*ptr = 0;
	a = get_cgi_applet(path_info);
	if (NULL == a)
		http_fatal("错误的文件名");
	return (*(a->main)) ();
}
