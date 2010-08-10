#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iconv.h>    // convert gb2312 to utf8
#include "bbs.h"
#include "discuzsql.h"  // like usesql.h define the name,password and database of discuz
#include "discuzmodule.h"

int
main(int argc, char *argv[])
{
	char log[256];
	writesyslog(ERRORLOG, "error test\n");
	sprintf(log, "%s", "printf adminlog\n");
	writesyslog(ADMINLOG, log);
	sprintf(log, "%s", "printf testlog\n");
	writesyslog(TESTLOG, log);
	sprintf(log, "%s", "printf testloglog\n");
	writesyslog(TESTLOGLOG, log);
	sprintf(log, "%s", "printf illegal log \n");
	writesyslog(5, log);
	return 0;
}
