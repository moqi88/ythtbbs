#include <sys/types.h>
#include <sys/socket.h>

#include "ythtlib.h"

struct cuser_ctx *
cuser_init(char *host)
{
	int ret = 0;
	struct cuser_ctx *ctx;

	ctx = malloc(sizeof (struct cuser_ctx));
	if (ctx == NULL)
		return NULL;
	memset(ctx, 0, sizeof (struct cuser_ctx));
	ctx->host.sin_port = htons(CUSER_PORT);
	ctx->host.sin_family = AF_INET;
	ret = inet_aton(host, &(ctx->host.sin_addr));
	if (ret == 0) {
		free(ctx);
		return NULL;
	}
	return ctx;
}

void
cuser_fini(struct cuser_ctx *ctx)
{
	if (ctx != NULL)
		free(ctx);
	return;
}

static int
cuser_proto(struct cuser_ctx *ctx, struct cuser_proto_req *cuser_req)
{
	int fd, ret;
	struct cuser_proto_ret cuser_ret;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(fd, (struct sockaddr *) &(ctx->host),
		    sizeof (ctx->host)) < 0) {
		close(fd);
		return -CUSER_ERR_CONNECT;
	}
	ret = send(fd, cuser_req, sizeof (struct cuser_proto_req), 0);
	if (ret < sizeof (*cuser_req)) {
		close(fd);
		return -CUSER_ERR_BROKEN;
	}
	ret = recv(fd, &cuser_ret, sizeof (cuser_ret), 0);
	if (ret != sizeof (cuser_ret)) {
		close(fd);
		return -CUSER_ERR_BROKEN;
	}
	if (ntohl(cuser_ret.key) != CUSER_KEY) {
		close(fd);
		return -CUSER_ERR_BROKEN;
	}
	close(fd);
	return cuser_ret.status;
}

int
cuser_check_user(struct cuser_ctx *ctx, char *id, char *pw)
{
	struct cuser_proto_req cuser_req;
	return CUSER_NO_YTHT;
	cuser_req.key = htonl(CUSER_KEY);
	strsncpy(cuser_req.id, id, 13);
	strsncpy(cuser_req.pw, pw, 32);
	return cuser_proto(ctx, &cuser_req);
}
