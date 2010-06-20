#include "bbslib.h"

int bbstestssl_main()
{
	printf("Content-type: text/plain\r\n\r\n");
#ifdef HTTPS_DOMAIN
	printf(
	"try {\n"
	"\tdocument.l.action=\"https://" HTTPS_DOMAIN "/" SMAGIC "/bbslogin\";\n"
	"\tdocument.ssl.src=\"lock.gif\";\n"
	"}\n"
	"catch (err) {\n"
	"}\n"
	);
#endif
}
