#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "bbs.h"
#include "ythtlib.h"
#include "ythtbbs.h"

void
print_file(char *dir, int mask, int result)
{
	char buf[256];
	char *ptr;
	struct mmapfile mf = { ptr:NULL };
	struct fileheader *fh;
	strsncpy(buf, dir, sizeof (buf));
	ptr = strrchr(buf, '/');
	if (!ptr)
		return;
	*ptr = 0;
	if (mmapfile(dir, &mf))
		return;
	printf("%s\n", dir);
	fh = (struct fileheader *) mf.ptr;
	while (fh < (struct fileheader *) (mf.ptr + mf.size)) {
		if ((fh->accessed & mask) == result)
			printf("%s/%s\n", buf, fh2fname(fh));
		else
			printf("HIDE %s/%s\n", buf, fh2fname(fh));
		fh++;
	}
	mmapfile(NULL, &mf);
}

void
process_bknum(char *board)
{
	char buf[256];
	struct mmapfile mf = { ptr:NULL };
	struct bknheader *bh;
	snprintf(buf, sizeof (buf), ".backnumbers/%s/.DIR", board);
	if (mmapfile(buf, &mf))
		return;
	printf("%s\n", buf);
	bh = (struct bknheader *) mf.ptr;
	while (bh < (struct bknheader *) (mf.ptr + mf.size)) {
		snprintf(buf, sizeof (buf), ".backnumbers/%s/%s/.DIR", board,
			 bknh2bknname(bh));
		print_file(buf, FH_HIDE, 0);
		bh++;
	}
	mmapfile(NULL, &mf);
}

int
main(int argc, char *argv[])
{
	char buf[256];

	if (argc < 2) {
		printf("which board?\n");
		exit(1);
	}

	snprintf(buf, sizeof (buf), "%s/.DIR", argv[1]);
	print_file(buf, 0, 0);
	snprintf(buf, sizeof (buf), "%s/.DIGEST", argv[1]);
	print_file(buf, FH_ISDIGEST, FH_ISDIGEST);
	process_bknum(argv[1]);
	return 0;
}
