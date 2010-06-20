#include <stdio.h>
#include "bbs.h"
#include "cuser.h"
#include "cuserd.h"

struct passrec {
	char id[12];
	int salt;
	char pw[16];
};

struct mmapfile pass_mf = { ptr:NULL };

/*Add by SmallPig*/
int
seek_in_file(filename, seekstr)
char filename[STRLEN], seekstr[STRLEN];
{
	FILE *fp;
	char buf[STRLEN];
	char *namep;

	if ((fp = fopen(filename, "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		namep = (char *) strtok(buf, ": \n\r\t");
		if (namep != NULL && strcasecmp(namep, seekstr) == 0) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

static int
check_user(char *id, char *pw)
{
	struct passrec *base = (struct passrec *) pass_mf.ptr;
	char id2[IDLEN + 1];
	int low, high;
	int t, c;

	id2[IDLEN] = 0;
	low = 0;
	high = pass_mf.size / sizeof (struct passrec);
	if (seek_in_file("/home/bbs/etc/ytht_pass", id))
		return CUSER_YTHT_OK;
	while (low <= high) {
		t = (low + high) / 2;
		memcpy(id2, base[t].id, IDLEN);
		c = strcasecmp(id, id2);
		if (!c) {
			if (checkpasswd(base[t].pw, base[t].salt, pw))
				return CUSER_YTHT_OK;
			else
				return CUSER_YTHT_BAD;
		} else if (c > 0) {
			low = t + 1;
		} else
			high = t - 1;
	}
	return CUSER_NO_YTHT;
}

static int
cuser_do_child(int connfd)
{
	int res;
	struct cuser_proto_req head_req;
	struct cuser_proto_ret head_ret;

	res = recv(connfd, &head_req, sizeof (head_req), 0);
	if (res != sizeof (head_req))
		return -1;

	head_ret.status = check_user(head_req.id, head_req.pw);
	head_ret.key = htonl(CUSER_KEY);
	send(connfd, &head_ret, sizeof (head_ret), 0);
	return 0;
}

static int
cuser_service(void)
{
	int listenfd, connfd;
	int val;
	struct linger ld;
	struct sockaddr_in cliaddr, servaddr;
	int caddr_len;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	if (listenfd < 0) {
		syslog(1, "cuserd: socket error");
		exit(-1);
	}

	val = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *) &val,
		   sizeof (val));
	ld.l_onoff = ld.l_linger = 0;
	setsockopt(listenfd, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof (ld));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(CUSER_PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof (servaddr)) <
	    0) {
		syslog(1, "cuserd: bind error");
		exit(-1);
	}

	listen(listenfd, 32);

	while (1) {
		connfd =
		    accept(listenfd, (struct sockaddr *) &cliaddr, &caddr_len);
		if (connfd < 0)
			continue;
		cuser_do_child(connfd);
		close(connfd);
	}
	return 0;
}

int
main(int argc, char **argv)
{
	if (daemon(1, 0))
		return -1;
	if (mmapfile("PASS-N", &pass_mf) < 0)
		return -2;
	cuser_service();
	return 0;
}
