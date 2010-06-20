#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <time.h>
#include "ythtbbs.h"

char *
fh2fname(struct fileheader *fh)
{
	static char s[16];
	sprintf(s, "M.%d.A", fh->filetime);
	if (fh->accessed & FH_ISDIGEST)
		s[0] = 'G';
	return s;
}

char *
bknh2bknname(struct bknheader *bknh)
{
	static char s[16];
	sprintf(s, "B.%d", bknh->filetime);
	return s;
}

char *
fh2owner(struct fileheader *fh)
{
	if (!fh->owner[0])
		return "Anonymous";
	else
		return fh->owner;
}

char *
fh2realauthor(struct fileheader *fh)
{
	if (!fh->owner[0])
		return fh->owner + 1;
	else
		return fh->owner;
}

int
fh2modifytime(struct fileheader *fh)
{
	if (fh->edittime)
		return fh->edittime;
	else
		return fh->filetime;
}

void
fh_setowner(struct fileheader *fh, char *owner, int anony)
{
	char *ptr;
	bzero(fh->owner, sizeof (fh->owner));
	if (anony) {
		fh->owner[0] = 0;
		strsncpy(fh->owner + 1, owner, sizeof (fh->owner) - 1);
		return;
	}
	if (!*owner) {
		*fh->owner = 0;
		return;
	}
	strsncpy(fh->owner, owner, sizeof (fh->owner) - 1);
	if (!strchr(owner, '@') && !strchr(owner, '.')) {
		ptr = strchr(fh->owner, ' ');
		if (ptr)
			*ptr = 0;
		return;
	}
	ptr = fh->owner + 1;
	while (*ptr) {
		if (*ptr == '@' || *ptr == '.' || *ptr == ' ') {
			*ptr = '.';
			*(ptr + 1) = 0;
			return;
		}
		ptr++;
	}
	*--ptr = '.';
	return;
}

// mode = 1 指func需要使用 fileinfo 中的参数，比如title啥的
// mode = 0 则不需要
int
change_dir(char *direct, struct fileheader *fileinfo,
	   void (*func(struct fileheader *, struct fileheader *, void *)), int ent, int digestmode, int mode, void *cb_arg)
{
	int i, newent;
	int fd;
	int size = sizeof (struct fileheader);
	struct fileheader xfh, newfileinfo;
	if (digestmode != 0)
		return -1;
	if ((fd = open(direct, O_RDWR, 0660)) == -1)
		return -2;
	if (flock(fd, LOCK_EX) == -1) {
		errlog("reclock error %d", errno);
		close(fd);
		return -4;
	}

	if (mode)
		memcpy(&newfileinfo, fileinfo, size);
	newent = ent;
	for (i = newent; i > 0; i--) {
		if (lseek(fd, size * (i - 1), SEEK_SET) == -1) {
			i = 0;
			break;
		}
		if (read(fd, &xfh, size) != size) {
			i = 0;
			break;
		}
		if (fileinfo->filetime == xfh.filetime) {
			newent = i;
			break;
		}
	}
	if (!i) {
		flock(fd, LOCK_UN);
		close(fd);
		return -3;
	}
	if ((*func) (&xfh, &newfileinfo, cb_arg) < 0) {
		flock(fd, LOCK_UN);
		close(fd);
		return -1;
	}
	if (lseek(fd, size * (newent - 1), SEEK_SET) == -1) {
		errlog("subrec seek err %d", errno);
		flock(fd, LOCK_UN);
		close(fd);
		return -5;
	}
	if (write(fd, &xfh, size) != size) {
		errlog("subrec write err %d", errno);
		flock(fd, LOCK_UN);
		close(fd);
		return -6;
	}
	flock(fd, LOCK_UN);
	close(fd);
	memcpy(fileinfo, &xfh, sizeof(struct fileheader));
	return 0;
}

#define SWITCH_FLAG(x, y) if(x & y) {x &= ~y;} else { x |= y;}

int
DIR_do_mark(struct fileheader *fileinfo, struct fileheader *newfileinfo, void *cbarg)
{
	SWITCH_FLAG(fileinfo->accessed, FH_MARKED);
	return 0;
}

int
DIR_do_setbmuse(struct fileheader *fileinfo, struct fileheader *newfileinfo, 
		void *cbard) {
	SWITCH_FLAG(fileinfo->accessed, FH_BMUSE);
	return 0;
}

int
DIR_do_digest(struct fileheader *fileinfo, struct fileheader *newfileinfo, void *cbarg)
{
	SWITCH_FLAG(fileinfo->accessed, FH_DIGEST);
	return 0;
}

int
DIR_do_underline(struct fileheader *fileinfo, struct fileheader *newfileinfo, void *cbarg)
{
	SWITCH_FLAG(fileinfo->accessed, FH_NOREPLY);
	return 0;
}

int
DIR_do_allcanre(struct fileheader *fileinfo, struct fileheader *newfileinfo, void *cbarg)
{
	SWITCH_FLAG(fileinfo->accessed, FH_ALLREPLY);
	return 0;
}

int
DIR_do_attach(struct fileheader *fileinfo, struct fileheader *newfileinfo, void *cbarg)
{
	fileinfo->accessed |= FH_ATTACHED;
	return 0;
}

int
DIR_clear_dangerous(struct fileheader *fileinfo, struct fileheader *newfileinfo, void *cbarg)
{
	fileinfo->accessed &= ~FH_DANGEROUS;
	return 0;
}

int
DIR_do_dangerous(struct fileheader *fileinfo, struct fileheader *newfileinfo, void *cbarg)
{
	fileinfo->accessed |= FH_DANGEROUS;
	return 0;
}

int
DIR_do_markdel(struct fileheader *fileinfo, struct fileheader *newfileinfo, void *cbarg)
{
	SWITCH_FLAG(fileinfo->accessed, FH_DEL);
	return 0;
}

int
DIR_do_edit(struct fileheader *fileinfo, struct fileheader *newfileinfo, void *cbarg)
{
	fileinfo->sizebyte = newfileinfo->sizebyte;
	fileinfo->edittime = newfileinfo->edittime;
	return 0;
}

int
DIR_do_changetitle(struct fileheader *fileinfo, struct fileheader *newfileinfo, void *cbarg)
{
	strncpy(fileinfo->title, newfileinfo->title, 60);
	fileinfo->title[59] = 0;
	return 0;
}

int
DIR_do_evaluate(struct fileheader *fileinfo, struct fileheader *newfileinfo, void *cbarg)
{
	fileinfo->staravg50 = newfileinfo->staravg50;
	fileinfo->hasvoted = newfileinfo->hasvoted;
	return 0;
}

int
DIR_do_spec(struct fileheader *fileinfo, struct fileheader *newfileinfo, void *cbarg)
{
	SWITCH_FLAG(fileinfo->accessed, FH_SPEC);
	return 0;
}

int
DIR_do_import(struct fileheader *fileinfo, struct fileheader *newfileinfo, void *cbarg)
{
	if (newfileinfo->accessed & FH_ANNOUNCE)
		fileinfo->accessed |= FH_ANNOUNCE;
	return 0;
}

int
DIR_do_suremarkdel(struct fileheader *fileinfo, struct fileheader *newfileinfo, void *cbarg)
{
	fileinfo->accessed |= FH_DEL;
	return 0;
}

int
DIR_do_sureunmarkdel(struct fileheader *fileinfo,
		     struct fileheader *newfileinfo, void *cbarg)
{
	fileinfo->accessed &= ~FH_DEL;
	return 0;
}

int
outgo_post(struct fileheader *fh, char *board, char *id, char *name)
{
	FILE *foo;
	if (fh->accessed & FH_INND)
		if ((foo = fopen("bbstmpfs/innd/out.bntp", "a"))) {
			fprintf(foo, "%s\t%s\t%s\t%s\t%s\n", board,
				fh2fname(fh), id, name, fh->title);
			fclose(foo);
		}
	return 0;
}

/* modifying by ylsdd 
 * unlink action is taked within cancelpost if in mail
 * else this item is added to the file '.DELETED' under
 * the board's directory, the filename is not changed.
 * Unlike the fb code which moves the file to the deleted
 * board.
 *             */
void
cancelpost(char *board, char *userid, struct fileheader *fh, int owned)
{
	struct fileheader postfile;
	FILE *fin;
	int digestmode, len;
	char buf[256], from[STRLEN], *ptr;
	time_t now_t;
	postfile = *fh;
	postfile.accessed &= ~(FH_MARKED | FH_SPEC | FH_DIGEST);
	postfile.deltime = time(&now_t) / (3600 * 24) % 100;
	sprintf(buf, "%-32.32s - %s", fh->title, userid);
	strsncpy(postfile.title, buf, sizeof (postfile.title));
	digestmode = (owned) ? 5 : 4;
	if (5 == digestmode)
		sprintf(buf, MY_BBS_HOME "/boards/%s/.JUNK", board);
	else
		sprintf(buf, MY_BBS_HOME "/boards/%s/.DELETED", board);
	append_record(buf, &postfile, sizeof (postfile));
	if (strrchr(fh->owner, '.'))
		return;
	if ((fh->accessed & FH_INND) && fh->filetime > now_t - 14 * 86400) {
		sprintf(buf, MY_BBS_HOME "/boards/%s/%s", board, fh2fname(fh));
		from[0] = '\0';
		if ((fin = fopen(buf, "r")) != NULL) {
			while (fgets(buf, sizeof (buf), fin) != NULL) {
				len = strlen(buf) - 1;
				buf[len] = '\0';
				if (len <= 8)
					break;
				if (strncmp(buf, "发信人: ", 8))
					continue;
				if ((ptr = strrchr(buf, ')')) != NULL) {
					*ptr = '\0';
					if ((ptr = strrchr(buf, '('))
					    != NULL) {
						strcpy(from, ptr + 1);
						break;
					}
				}
			}
			fclose(fin);
		}
		sprintf(buf, "%s\t%s\t%s\t%s\t%s\n",
			board, fh2fname(fh), fh->owner, from, fh->title);
		if ((fin = fopen("bbstmpfs/innd/cancel.bntp", "a")) != NULL) {
			fputs(buf, fin);
			fclose(fin);
		}
	}
}

int
cmp_title(char *title, struct fileheader *fh1)
{
	char *p1;
	if (!strncasecmp(fh1->title, "Re:", 3))
		p1 = fh1->title + 4;
	else
		p1 = fh1->title;
	return (!strncmp(p1, title, 45));
}

int
fh_find_thread(struct fileheader *fh, char *board)
{
	char direct[255];
	char *p;
	int i;
	int start;
	struct mmapfile mf = { ptr:NULL };
	struct fileheader *buf1;
	char *title = fh->title;
	int size = sizeof (struct fileheader);
	if (fh->thread != 0)
		return 0;
	fh->thread = fh->filetime;
	sprintf(direct, MY_BBS_HOME "/boards/%s/.DIR", board);
	if (mmapfile(direct, &mf) < 0)
		return -1;
	if (!strncasecmp(title, "Re:", 3))
		p = title + 4;
	else
		p = title;
	start = mf.size / size;
	for (i = start, buf1 =
	     (struct fileheader *) (mf.ptr + size * (start - 1));
	     i > start - 100 && i > 0;
	     i--, buf1 = (struct fileheader *) (mf.ptr + size * (i - 1)))
		//仅在最近100篇内尝试搜索同主题
		if (cmp_title(p, buf1)) {
			if (buf1->thread != 0) {
				fh->thread = buf1->thread;
				break;
			}
		}
	mmapfile(NULL, &mf);
	return 0;
}

int
Search_Bin(struct fileheader *ptr0, int key, int start, int end)
{
	// 在有序表中折半查找其关键字等于key的数据元素。
	// 若查找到，返回索引
	// 否则为大于key的最小数据元素索引m，返回(-m-1) 
	int low, high, mid;
	struct fileheader *totest, *ptr;
	ptr = (struct fileheader *) ptr0;
	low = start;
	high = end;
	while (low <= high) {
		mid = (low + high) / 2;
		totest = ptr + mid;
		if (key == totest->filetime)
			return mid;
		else if (key < totest->filetime)
			high = mid - 1;
		else
			low = mid + 1;
	}
	return -(low + 1);
}

int
add_edit_mark(char *fname, char *userid, time_t now_t, char *fromhost)
{
	FILE *fp;
	if ((fp = fopen(fname, "a")) == NULL)
		return 0;
	fprintf(fp,
		"\n\033[1;36m※ 修改:．%s 于 %15.15s 修改本文．[FROM: %-.20s]\033[m\n",
		userid, ctime(&now_t) + 4, fromhost);
	fclose(fp);
	return 0;
}

int
delete_dir_callback(char *dirname, int ent, int filetime, int (*callback)(struct fileheader *, void *), void *cb_arg)
{
	int fd;
	struct stat st;
	int ret = 0, pos;
	struct fileheader *ptr = NULL;

	if ((fd = open(dirname, O_RDWR)) == -1)
		return -1;
	flock(fd, LOCK_EX);
	fstat(fd, &st);
	MMAP_TRY {
		ptr =
		    mmap(0, st.st_size, PROT_READ | PROT_WRITE,
			 MAP_FILE | MAP_SHARED, fd, 0);
		ret = 0;
		pos = st.st_size / sizeof(struct fileheader);
		if( pos > ent)
			pos = ent;
		if ((ptr+(pos-1))->filetime != filetime) {
			pos = Search_Bin(ptr, filetime, 0, pos - 1) + 1;
			if (pos <= 0) {
				for (pos = ent; pos > 0; pos--)
					if ((ptr + (pos - 1))->filetime == filetime)
						break;
				if (pos == 0)
					ret = -2;
			}
		}
		if (ret == 0) {
			if (callback == NULL || callback(ptr + (pos - 1), cb_arg) >= 0) {
				memmove(ptr + (pos - 1), ptr + pos, st.st_size - sizeof(struct fileheader) * pos);
			}
		}
	}
	MMAP_CATCH {
	}
	MMAP_END munmap(ptr, st.st_size);
	if(!ret)
		ftruncate(fd, st.st_size - sizeof(struct fileheader));
	flock(fd, LOCK_UN);
	close(fd);
	return ret;
}

int
append_dir_callback(char *filename, struct fileheader *record, int thread, int (*callback)(struct fileheader *, void *), void *cb_arg)
{
	int fd;
	int t;
	char filepath[256];
	char *ptr;
	time_t now = time(NULL);
	int ret;

	if (strlen(filename) >= 256)
		return -1;
	strcpy(filepath, filename);
	ptr = strrchr(filepath, '/');
	if (ptr == NULL)
		return -1;
	*ptr = 0;

	if ((fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0660)) == -1) {
		return -1;
	}
	flock(fd, LOCK_EX);

	t = trycreatefile(filepath, "M.%d.A", now, 100);
	if (t < 0) {
		flock(fd, LOCK_UN);
		close(fd);
		return -1;
	}
	record->filetime = t;
	if (thread == 0)
		record->thread = t;
	else
		record->thread = thread;
	if (callback != NULL) {
		ret = (*callback)(record, cb_arg);
		if (ret < 0) {
			flock(fd, LOCK_UN);
			close(fd);
			return -1;
		}
	}
	write(fd, record, sizeof(struct fileheader));
	flock(fd, LOCK_UN);
	close(fd);
	return t;
}
