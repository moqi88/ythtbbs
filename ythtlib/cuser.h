#ifndef _CUSER_H
#define _CUSER_H
#include <arpa/inet.h>

#define CUSER_PORT  40913
#define CUSER_KEY   0x20040913

struct cuser_proto_req {
	uint32_t key;		//magic key
	unsigned char id[13];
	unsigned char pw[32];
} __attribute__ ((packed));

struct cuser_proto_ret {
	uint32_t key;		//magic key
	uint32_t status;
} __attribute__ ((packed));

struct cuser_ctx {
	struct sockaddr_in host;
} __attribute__ ((packed));

#define CUSER_ERR_BROKEN 1;
#define CUSER_ERR_CONNECT 2;

#define CUSER_NO_YTHT 1
#define CUSER_YTHT_BAD 2
#define CUSER_YTHT_OK 3

struct cuser_ctx *cuser_init(char *host);
void cuser_fini(struct cuser_ctx *ctx);
int cuser_check_user(struct cuser_ctx *ctx, char *id, char *pw);
#endif
