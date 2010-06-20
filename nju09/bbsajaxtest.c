#include "bbslib.h"

/* This file is last modified by Fishingsnow */
/* It is only a test of ajax, or some sense, ajah */

int bbsajaxtest_main(void) {
	html_header(1);
	printf("<script src=/function.js></script>");

	printf("<script src=/ajax.js></script></head></body>");

	printf("<script>checkFrame(1);</script>");

	printf("<div id=\"ajax_test\"></div>");

	printf("<input id=\"ajax_button\" type=button value=go onclick='ajax_login(1, 2)'>");

	http_quit();

	return 1;
}
