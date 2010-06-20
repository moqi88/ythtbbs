#include "ythtbbs.h"
#include <sys/mman.h>

static int _initialFavorFolder(char *userid);
#ifdef FAVOR_UPGRADE
static int _upgradeFavor1(char *userid, struct FolderSet *Favor);
static int _upgradeFavor2(char *userid, struct FolderSet *Favor);
#endif

char *
bm2str(buf, bh)
char *buf;
struct boardheader *bh;
{
	int i;
	buf[0] = 0;
	for (i = 0; i < 4; i++)
		if (bh->bm[i][0] == 0)
			break;
		else {
			if (i != 0)
				strcat(buf, " ");
			strcat(buf, bh->bm[i]);
		}
	return buf;
}

char *
sbm2str(buf, bh)
char *buf;
struct boardheader *bh;
{
	int i;
	buf[0] = 0;
	for (i = 4; i < BMNUM; i++)
		if (bh->bm[i][0] == 0)
			break;
		else {
			if (i != 0)
				strcat(buf, " ");
			strcat(buf, bh->bm[i]);
		}
	return buf;
}

int
chk_BM(struct userec *user, struct boardheader *bh, int isbig)
{
	int i;
	for (i = 0; i < 4; i++) {
		if (bh->bm[i][0] == 0)
			break;
		if (!strcmp(bh->bm[i], user->userid)
		    && bh->hiretime[i] >= user->firstlogin)
			return i + 1;
	}
	if (isbig)
		return 0;
	for (i = 4; i < BMNUM; i++) {
		if (bh->bm[i][0] == 0)
			break;
		if (!strcmp(bh->bm[i], user->userid)
		    && bh->hiretime[i] >= user->firstlogin)
			return i + 1;
	}
	return 0;
}

int
chk_BM_id(char *user, struct boardheader *bh)
{
	int i;
	for (i = 0; i < BMNUM; i++) {
		if (bh->bm[i][0] == 0) {
			if (i < 4) {
				i = 3;
				continue;
			}
			break;
		}
		if (!strcmp(bh->bm[i], user))
			return i + 1;
	}
	return 0;
}

int
bmfilesync(struct userec *user)
{
	char path[256];
	struct myparam1 mp;
	sethomefile(path, user->userid, "mboard");
	if (file_time(path) > file_time(".BOARDS"))
		return 0;
	memcpy(&(mp.user), user, sizeof (struct userec));
	mp.fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0600);	//touch a new file
	if (mp.fd == -1) {
		errlog("touch new mboard error");
		return -1;
	}
	mp.bid = 0;
	new_apply_record(".BOARDS", sizeof (struct boardheader),
			 (void *) fillmboard, &mp);
	close(mp.fd);
	return 0;
}

int
fillmboard(struct boardheader *bh, struct myparam1 *mp)
{
	struct boardmanager bm;
	int i;
	if ((i = chk_BM(&(mp->user), bh, 0))) {
		bzero(&bm, sizeof (bm));
		strncpy(bm.board, bh->filename, 24);
		bm.bmpos = i - 1;
		bm.bid = mp->bid;
		write(mp->fd, &bm, sizeof (bm));
	}
	(mp->bid)++;
	return 0;
}

static inline int
setbmhat(struct boardmem *bmem, char *userid, int online, int invisible)
{
	int bmpos;
	bmpos = chk_BM_id(userid, &bmem->header) - 1;
	if (bmpos < 0)
		return -1;
	if (online) {
		bmem->bmonline |= (1 << bmpos);
		if (invisible)
			bmem->bmcloak |= (1 << bmpos);
		else
			bmem->bmcloak &= ~(1 << bmpos);
	} else {
		bmem->bmonline &= ~(1 << bmpos);
		bmem->bmcloak &= ~(1 << bmpos);
	}
	return 0;
}

int
dosetbmstatus(struct boardmem *bcache, char *userid, int online, int invisible)
{
	int i;
	if(!bcache)
		return -1;
	for (i = 0; i < MAXBOARD; i++) {
		setbmhat(&bcache[i], userid, online, invisible);
	}
	return 0;
}

int
getlastpost(char *board, int *lastpost, int *total)
{
	struct fileheader fh;
	struct stat st;
	char filename[STRLEN * 2];
	int fd, atotal;

	snprintf(filename, sizeof (filename), MY_BBS_HOME "/boards/%s/.DIR",
		 board);
	if ((fd = open(filename, O_RDONLY)) < 0) {
		*lastpost = 0;
		*total = 0;
		return 0;
	}
	fstat(fd, &st);
	atotal = st.st_size / sizeof (fh);
	if (atotal <= 0) {
		*lastpost = 0;
		*total = 0;
		close(fd);
		return 0;
	}
	*total = atotal;
	lseek(fd, (atotal - 1) * sizeof (fh), SEEK_SET);
	if (read(fd, &fh, sizeof (fh)) > 0) {
		if (fh.edittime == 0)
			*lastpost = fh.filetime;
		else
			*lastpost = fh.edittime;
	}
	close(fd);
	return 0;
}

struct boardaux *
getboardaux(int bnum)		//bnum starts from 0, i.e., bnum for the first board is 0
{
      static struct mmapfile mf = { ptr:NULL };
	if (!mf.ptr) {
		if (mmapfile(BOARDAUX, &mf) < 0)
			return NULL;
	}
	if (bnum < 0 || bnum >= MAXBOARD)
		return NULL;
	return &((struct boardaux *) mf.ptr)[bnum];
}

int
addtopfile(int bnum, struct fileheader *fh)	//bnum starts from 0
{
	struct boardaux baux;
	int fd;
	bzero(&baux, sizeof (struct boardaux));
	if ((fd = open(BOARDAUX, O_RDWR | O_CREAT, 0660)) < 0)
		return -1;
	lseek(fd, bnum * sizeof (struct boardaux), SEEK_SET);
	read(fd, &baux, sizeof (struct boardaux));
	if (baux.ntopfile >= MAXTOPFILE)
		return -1;
	fh->accessed |= FH_ISTOP;
	baux.topfile[baux.ntopfile] = *fh;
	baux.ntopfile++;
	lseek(fd, bnum * sizeof (struct boardaux), SEEK_SET);
	write(fd, &baux, sizeof (struct boardaux));
	close(fd);
	return 0;
}

int
deltopfile(int bnum, int num)	//bnum starts from 0, num starts from 0
{
	struct boardaux baux;
	int fd, i;
	if (bnum < 0 || num < 0)
		return -1;
	bzero(&baux, sizeof (struct boardaux));
	if ((fd = open(BOARDAUX, O_RDWR | O_CREAT, 0660)) < 0)
		return -1;
	lseek(fd, bnum * sizeof (struct boardaux), SEEK_SET);
	read(fd, &baux, sizeof (struct boardaux));
	if (baux.ntopfile <= num)
		return -1;
	for (i = num; i < baux.ntopfile - 1; i++)
		baux.topfile[i] = baux.topfile[i + 1];
	baux.ntopfile--;
	lseek(fd, bnum * sizeof (struct boardaux), SEEK_SET);
	write(fd, &baux, sizeof (struct boardaux));
	close(fd);
	return 0;
}

int
updateintro(int bnum, char *filename)	//更新 WWW 版面简介，bnum starts from 0
{
	struct boardaux baux;
	int fd, i;
	char buf[200];
	if (bnum < 0 || bnum >= MAXBOARD)
		return -1;
	buf[0] = 0;
	fd = open(filename, O_RDONLY);
	if (fd >= 0) {
		i = read(fd, buf, sizeof (buf) - 1);
		buf[i] = 0;
		close(fd);
	}
	bzero(&baux, sizeof (struct boardaux));
	if ((fd = open(BOARDAUX, O_RDWR | O_CREAT, 0660)) < 0)
		return -1;
	lseek(fd, bnum * sizeof (struct boardaux), SEEK_SET);
	read(fd, &baux, sizeof (struct boardaux));
	strsncpy(baux.intro, buf, sizeof (baux.intro));
	lseek(fd, bnum * sizeof (struct boardaux), SEEK_SET);
	write(fd, &baux, sizeof (struct boardaux));
	close(fd);
	return 0;

}

int
_inCurrFolder(char *bname, struct FolderSet *Favor, int currFolder) {

	int i, bindex;

	if (NULL == Favor || currFolder > FOLDER_NUM - 1 || currFolder < 0)
		return -1;

	for (i = 0; i < Favor->folder[currFolder].boardNum && 
			i < FOLDER_BRD_NUM; i++) {
		bindex = Favor->folder[currFolder].bidx[i];
		if (!strcasecmp(bname, Favor->boards[bindex]))
			return i;
	}
	return -1;
}

int
_inFavorBoards(char *bname, struct FolderSet *Favor) {
	int i;

	if (NULL == Favor)
		return 0;
	for (i = 1; i < FAVOR_BRD_NUM + 1; i++) {
		if (!strcasecmp(bname, Favor->boards[i]))
			return i;
	}

	return 0;
}

static int
_initialFavorFolder(char *userid) {
	char filepath[256];
	int fd;
	struct FolderSet TmpFavor;

	sethomefile(filepath, userid, FAVOR_FILE);
	if ((fd = open(filepath, O_RDWR | O_CREAT, 0600)) < 0)
		return 0;
	
	flock(fd, LOCK_EX);
	bzero(&TmpFavor, sizeof(struct FolderSet));
	strcpy(TmpFavor.folder[0].name, "根目录");
	strcpy(TmpFavor.folder[0].desc, "不能修改");
#ifdef FAVOR_UPGRADE
	if (_upgradeFavor2(userid, &TmpFavor)) {
		write(fd, &TmpFavor, sizeof(struct FolderSet));
		flock(fd, LOCK_UN);
		close(fd);
		return 1;
	} else if (_upgradeFavor1(userid, &TmpFavor)) {
		write(fd, &TmpFavor, sizeof(struct FolderSet));
		flock(fd, LOCK_UN);
		close(fd);
		return 1;
	}
#endif

	/*一路 一共13个默认收藏夹
	  *	TmpFavor.folder[0].bidx[0] = 1;
	  * TmpFavor.folder[0].boardNum = 1;
	  * TmpFavor.boardNum = 1
	 */
	TmpFavor.folder[0].bidx[0] = 1;
	TmpFavor.folder[0].bidx[1] = 2;
	TmpFavor.folder[0].bidx[2] = 3;
	TmpFavor.folder[0].bidx[3] = 4;
	TmpFavor.folder[0].bidx[4] = 5;
	TmpFavor.folder[0].bidx[5] = 6;
	TmpFavor.folder[0].bidx[6] = 7;
	TmpFavor.folder[0].bidx[7] = 8;
	TmpFavor.folder[0].bidx[8] = 9;
	TmpFavor.folder[0].bidx[9] = 10;
	TmpFavor.folder[0].bidx[10] = 11;
	TmpFavor.folder[0].bidx[11] = 12;
	TmpFavor.folder[0].bidx[12] = 13;
	TmpFavor.folder[0].boardNum = 13;
	TmpFavor.boardNum = 13;
	
	strncpy(TmpFavor.boards[1], FAVOR_DEFAULT, 
			sizeof(TmpFavor.boards[1]));
	strncpy(TmpFavor.boards[2], FAVOR_YILU_DEFAULT_1, 
				sizeof(TmpFavor.boards[2]));
	strncpy(TmpFavor.boards[3], FAVOR_YILU_DEFAULT_2, 
					sizeof(TmpFavor.boards[3]));
	strncpy(TmpFavor.boards[4], FAVOR_YILU_DEFAULT_3, 
					sizeof(TmpFavor.boards[4]));
	strncpy(TmpFavor.boards[5], FAVOR_YILU_DEFAULT_4, 
					sizeof(TmpFavor.boards[5]));
	strncpy(TmpFavor.boards[6], FAVOR_YILU_DEFAULT_5, 
					sizeof(TmpFavor.boards[6]));
	strncpy(TmpFavor.boards[7], FAVOR_YILU_DEFAULT_6, 
					sizeof(TmpFavor.boards[7]));
	strncpy(TmpFavor.boards[8], FAVOR_YILU_DEFAULT_7, 
					sizeof(TmpFavor.boards[8]));
	strncpy(TmpFavor.boards[9], FAVOR_YILU_DEFAULT_8, 
					sizeof(TmpFavor.boards[9]));
	strncpy(TmpFavor.boards[10], FAVOR_YILU_DEFAULT_9, 
					sizeof(TmpFavor.boards[10]));
	strncpy(TmpFavor.boards[11], FAVOR_YILU_DEFAULT_10, 
					sizeof(TmpFavor.boards[11]));
	strncpy(TmpFavor.boards[12], FAVOR_YILU_DEFAULT_11, 
					sizeof(TmpFavor.boards[12]));
	strncpy(TmpFavor.boards[13], FAVOR_YILU_DEFAULT_12, 
					sizeof(TmpFavor.boards[13]));
	
	write(fd, &TmpFavor, sizeof(struct FolderSet));
	flock(fd, LOCK_UN);
	close(fd);
	return 1;
	/*
		//add other favor Boards for new users changed by dl
		_addFavorBoard("triangle", FavorFolder, 0);
		_addFavorBoard("joke", FavorFolder, 0);
		_addFavorBoard("Movie", FavorFolder, 0);
		_addFavorBoard("music", FavorFolder, 0);
		_addFavorBoard("familylife", FavorFolder, 0);
		_addFavorBoard("job", FavorFolder, 0);
		_addFavorBoard("stock", FavorFolder, 0);
		_addFavorBoard("AdvancedEdu", FavorFolder, 0);
		_addFavorBoard("Game", FavorFolder, 0);
		_addFavorBoard("China_news", FavorFolder, 0);
		_addFavorBoard("Test", FavorFolder, 0);
		_addFavorBoard("sysop", FavorFolder, 0);
		//end of change
*/
}

struct FolderSet *
_loadFavorFolder(char *userid) {
	char filepath[256];
	struct FolderSet *Favor;
	int fd;

	sethomefile(filepath, userid, FAVOR_FILE);

	if (!file_exist(filepath) && !_initialFavorFolder(userid))
		return NULL;

	if ((fd = open(filepath, O_RDWR, 0600)) < 0)
		return NULL;

	Favor = mmap(0, sizeof(struct FolderSet), PROT_READ | PROT_WRITE, 
			MAP_SHARED, fd, 0);
	close(fd);
	if (MAP_FAILED == Favor)
		return NULL;
	return Favor;
}

int
_unloadFavorFolder(struct FolderSet *Favor) {
	if (NULL == Favor)
		return 1;
	munmap(Favor, sizeof(struct FolderSet));
	return 1;
}

int
_addFavorBoard(char *bname, struct FolderSet *Favor, int currFolder) {

	int i, nCurrBrd, nTotalBrd, bindex;

	if (NULL == Favor || currFolder > FOLDER_NUM - 1 || currFolder < 0)
		return 1;

	if (_inCurrFolder(bname, Favor, currFolder) >= 0)
		return 2;

	nCurrBrd = Favor->folder[currFolder].boardNum;
	nTotalBrd = Favor->boardNum;

	if (nCurrBrd > FOLDER_BRD_NUM - 1)
		return 3;

	if ((bindex = _inFavorBoards(bname, Favor))) {
					// 如果已经在其他目录中
					// 直接添加一个链接
		Favor->folder[currFolder].bidx[nCurrBrd] = bindex;
		Favor->folder[currFolder].boardNum++;
		return 0;
	}

	if (nTotalBrd > FAVOR_BRD_NUM - 1)
		return 4;

	for (i = 1; i < FAVOR_BRD_NUM + 1; i++) {
		if (!(Favor->boards[i][0]))
					// 找到第一个空位
			break;
	}

	Favor->folder[currFolder].bidx[nCurrBrd] = i;
	Favor->folder[currFolder].boardNum++;
	Favor->boardNum++;
	strncpy(Favor->boards[i], bname, sizeof(Favor->boards[i]));
	return 0;
}

int 
_addFavorFolder(char *foldername, char *desc, struct FolderSet *Favor) {
	int nCurrFolder, i;
	
	if (NULL == Favor)
		return 1;

	nCurrFolder = Favor->folderNum + 1;
					// 因为 folder[0] 是根目录，
					// 但在 folderNum 中没有体现

	if (nCurrFolder > FOLDER_NUM - 1)
		return 2;

	bzero(&(Favor->folder[nCurrFolder]), sizeof(struct SubFolder));
	strncpy(Favor->folder[nCurrFolder].name, foldername, 
			sizeof(Favor->folder[0].name));
	strncpy(Favor->folder[nCurrFolder].desc, desc, 
			sizeof(Favor->folder[0].desc));
	Favor->folderNum++;

	if (Favor->boardNum < FAVOR_BRD_NUM || 
			_inFavorBoards(FAVOR_DEFAULT, Favor)) {
					// 如果能添加默认版面
		_addFavorBoard(FAVOR_DEFAULT, Favor, nCurrFolder);
		return 0;
	}

	// 否则随意添加一个版面的链接

	for (i = 1; i < Favor->boardNum && i < FAVOR_BRD_NUM + 1; i++) {
		if (!(Favor->boards[i][0]))
			continue;
		_addFavorBoard(Favor->boards[i], Favor, nCurrFolder);
		break;
	}

	return 0;
}

int
_delFavorBoard(char *bname, struct FolderSet *Favor, 
		int currFolder, int keepLastBoard) {
	int i, bindex, bpos;

	if (NULL == Favor || currFolder < 0 || currFolder > FOLDER_NUM - 1)
		return 0;

	if ((bindex = _inCurrFolder(bname, Favor, currFolder)) < 0)
					// 当前目录没有这个版面
		return 1;

	if (Favor->folder[currFolder].boardNum == 1 && keepLastBoard)
		return 1;
	
	bpos = Favor->folder[currFolder].bidx[bindex];
	for (i = bindex; i < Favor->folder[currFolder].boardNum 
			&& i < FOLDER_BRD_NUM - 1; i++) {
		Favor->folder[currFolder].bidx[i] = 
			Favor->folder[currFolder].bidx[i + 1];
	}
	Favor->folder[currFolder].bidx[i] = 0;
	Favor->folder[currFolder].boardNum--;

	// 开始处理 boards[] 相关的问题。
	for (i = 0; i < FOLDER_NUM; i++) {
		if (_inCurrFolder(bname, Favor, i) >= 0)
					// 在其他目录找到了这个版面
			return 1;
	}
	// 如果不在其他目录中
	Favor->boards[bpos][0] = 0;	// 标记为空洞
	Favor->boardNum--;
	

	return 1;
}

int
_delFavorFolder(struct FolderSet *Favor, int currFolder) {
	int i, bindex;

	if (NULL == Favor || currFolder <= 0 || currFolder > Favor->folderNum)
		return 0;

	for (i = Favor->folder[currFolder].boardNum; i > 0; i--) {
					// 倒着删，更健康。
		bindex = Favor->folder[currFolder].bidx[i - 1];
		_delFavorBoard(Favor->boards[bindex], Favor, currFolder, 0);
	}

	for (i = currFolder; i < FOLDER_NUM - 1; i++) {
		memcpy(&(Favor->folder[i]), &(Favor->folder[i + 1]), 
				sizeof(struct SubFolder));
	}
	bzero(&(Favor->folder[i]), sizeof(struct SubFolder));
	Favor->folderNum--;

	return 1;
}

int
_editFavorFolder(struct FolderSet *Favor, int currFolder, 
		char *name, char *desc) {
	
	if (!Favor || currFolder < 0 || currFolder > Favor->folderNum)
		return 1;
	if (strlen(name) == 0 && strlen(desc) == 0)
		return 2;

	if (NULL != name && strlen(name) > 0)
		strncpy(Favor->folder[currFolder].name, name, 
				sizeof(Favor->folder[0].name));
	if (NULL != desc && strlen(desc) > 0)
		strncpy(Favor->folder[currFolder].desc, desc, 
				sizeof(Favor->folder[0].desc));

	return 0;
}

int
_moveFavorFolder(struct FolderSet *Favor, int currFolder, int tagFolder) {
	struct SubFolder tmpSubFolder;
	int i;

	if (NULL == Favor)
		return 1;
	if (currFolder <= 0 || currFolder > Favor->folderNum)
		return 2;
	if (tagFolder <= 0 || tagFolder > Favor->folderNum)
		return 3;
	if (currFolder == tagFolder)
		return 4;

	memcpy(&tmpSubFolder, &(Favor->folder[currFolder]),
			sizeof(struct SubFolder));
	if (currFolder < tagFolder) {
		for (i = currFolder; i < tagFolder; i++) {
			memcpy(&(Favor->folder[i]), 
					&(Favor->folder[i + 1]), 
					sizeof(struct SubFolder));
		}
	} else {
		for (i = currFolder; i > tagFolder; i--) {
			memcpy(&(Favor->folder[i]), 
					&(Favor->folder[i - 1]), 
					sizeof(struct SubFolder));
		}
	}
	memcpy(&(Favor->folder[tagFolder]), &tmpSubFolder, 
			sizeof(struct SubFolder));
	return 0;
}

int
_fixFavorFolder(struct FolderSet *Favor) {
	int i, j, k, inSomeFolder, bindex, nTotalBrd, nFolderBrd, nTotalFolder;

	if (NULL == Favor)
		return 0;

	nTotalBrd = 0;
	for (i = 1; i < FAVOR_BRD_NUM + 1; i++) {
		if (!Favor->boards[i][0])
			continue;
		nTotalBrd++;
		inSomeFolder = 0;
		for (j = 0; j < FOLDER_NUM; j++) {
			if (_inCurrFolder(Favor->boards[i], Favor, j) >= 0) {
					// 说明在某个目录中
				inSomeFolder = 1;
				break;
			}
		}
		if (!inSomeFolder)
			Favor->boards[i][0] = 0;

		if ((bindex = _inFavorBoards(Favor->boards[i], Favor)) < i) {
					// 说明有重复
			for (j = 0; j < FOLDER_NUM; j++) {
				for (k = 0; k < FOLDER_BRD_NUM && k < 
					Favor->folder[j].boardNum; k++) {
					if (Favor->folder[j].bidx[k] == i) {
						Favor->folder[j].bidx[k] = 
							bindex;
						break;
					}
				}
			}
		}
	}
	Favor->boardNum = nTotalBrd;

	nTotalFolder = 0;
	for (i = 0; i < FOLDER_NUM; i++) {
		nFolderBrd = 0;
		if (!Favor->folder[i].name[0] && i != 0)
			break;
		for (j = 0; j < FOLDER_BRD_NUM; j++) {
			bindex = Favor->folder[i].bidx[j];
			if (!bindex || bindex > FAVOR_BRD_NUM)
				break;
			if (!Favor->boards[bindex][0]) {
				Favor->folder[i].bidx[j] = 0;
			}
			else
				nFolderBrd++;
		}
		Favor->folder[i].boardNum = nFolderBrd;
		if (nFolderBrd == 0)
			_addFavorBoard(FAVOR_DEFAULT, Favor, i);
		nTotalFolder++;
		for (j = 0; j < FOLDER_BRD_NUM; j++) {
			if (Favor->folder[i].bidx[j] != 0)
				continue;
			for (k = j + 1; k < FOLDER_BRD_NUM; k++) {
				if (Favor->folder[i].bidx[k] == 0)
					continue;
				Favor->folder[i].bidx[j] = 
					Favor->folder[i].bidx[k];
				Favor->folder[i].bidx[k] = 0;
				break;
			}
		}
	}
	Favor->folderNum = nTotalFolder - 1;

	strcpy(Favor->folder[0].name, "根目录");
	strcpy(Favor->folder[0].desc, "不能修改");
	return 1;
}

#ifdef FAVOR_UPGRADE
static int
_upgradeFavor1(char *userid, struct FolderSet *Favor) {
	char filepath[256], bname[32];
	int i;
	FILE *fp;

	sethomefile(filepath, userid, FAVOR_OLDFILE1);

	if (!file_exist(filepath))
		return 0;

	if (NULL == (fp = fopen(filepath, "r")))
		return 0;

	for (i = 0; i < FOLDER_BRD_NUM - Favor->folder[0].boardNum; i++) {
		if (!fgets(bname, sizeof(bname), fp))
			break;
		if (strchr(bname, '\n'))
			*(strchr(bname, '\n')) = 0;
		if (strchr(bname, '\r'))
			*(strchr(bname, '\r')) = 0;
		_addFavorBoard(bname, Favor, 0);
	}
	fclose(fp);
	return 1;
}

static int
_upgradeFavor2(char *userid, struct FolderSet *Favor) {
	int fd;
	char filepath[256];

	sethomefile(filepath, userid, FAVOR_OLDFILE2);

	if (!file_exist(filepath))
		return 0;

	if ((fd = open(filepath, O_RDWR, 0600)) < 0)
		return 0;
	flock(fd, LOCK_EX);
	read(fd, Favor, sizeof(struct FolderSet));
	flock(fd, LOCK_UN);
	close(fd);

	_fixFavorFolder(Favor);
	return 1;
}
#endif
