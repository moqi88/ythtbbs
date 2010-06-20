#include "ythtbbs.h"
#include <sys/mman.h>   // for mmap

int
initData(int type, char *filepath)
{
	int fd;
	MC_Env mce;
	mcUserInfo ui;
	struct BoardStock bs;
	void *ptr = NULL;
	size_t filesize = 0;

	if (type == 0) {
		filesize = sizeof (MC_Env);
		bzero(&mce, filesize);
		mce.version = MONEYCENTER_VERSION;
		mce.transferRate = 100;
		mce.depositRate = 100;
		mce.loanRate = 200;
		ptr = &mce;
		mce.Treasury = 100000000; //¹ú¿â
		mkdir(DIR_MC, 0770);
	} else if (type == 1) {
		filesize = sizeof (mcUserInfo);
		bzero(&ui, filesize);
		ui.version = MONEYCENTER_VERSION;
		ptr = &ui;
		ui.con = random()%10 + 1;
		ui.GetLetter = 1;
		//ui.credit = 50000;
		//mcEnv->Treasury -= 50000;
		addtofile(DIR_MC "mc_user", filepath);
	} else {
		filesize = sizeof (struct BoardStock);
		bzero(&bs, filesize);
		ptr = &bs;
		mkdir(DIR_STOCK, 0770);
	}
	if ((fd = open(filepath, O_CREAT | O_EXCL | O_WRONLY, 0660)) == -1)
		return -1;
	write(fd, ptr, filesize);
	close(fd);
	return 0;
}

void *loadData(char *filepath, size_t filesize)
{
	int fd;
	void *buffer;

	if ((fd = open(filepath, O_RDWR, 0660)) == -1)
		return (void*)-1;
	buffer = mmap(0, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	return buffer;
}

void
unloadData(void *buffer, size_t filesize)
{
	if (buffer != NULL)
		munmap(buffer, filesize);
}


