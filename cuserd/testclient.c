#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "cuser.h"
int
main(int argc, char *argv[])
{
	struct cuser_ctx *ctx;
	int ret;

	if (argc < 3)
		return -1;
	ctx = cuser_init("127.0.0.1");
	ret = cuser_check_user(ctx, argv[1], argv[2]);
	if (ret < 0) {
		printf("err code is %d\n", ret);
		return 0;
	}
	printf("%d\n", ret);
	cuser_fini(ctx);
	return 0;
}
