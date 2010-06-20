#include "ythtbbs.h"

static int tmpl_checknum(char *str, int maxnum);

static int
tmpl_checknum(char *str, int maxnum)
{
	char *p = str;
	int n;
	while (*p) {
		if (*p < '0' || *p > '9')
			return -1;
		p++;
	}
	n = atoi(str);
	if (n < 1 || n > maxnum)
		return -1;
	return n;
}

/*
 * 使用callback的原因：
 * 由于进行了替换，目标字符串的长度就不可预知了
 * 因此正文部分直接写入文件
 * 标题可以输出到内存
 */
int
tmpl_replacetxt(char *buf, char **alist, int answer_num,
		int (*outputfun) (char *, void *), void *out)
{
	char *p, *pbase;
	int i;
	pbase = buf;
	while (1) {
		if ((p = strstr(pbase, "[$")) == NULL) {
			(*outputfun) (pbase, out);
			return 1;
		}
		*p = 0;
		(*outputfun) (pbase, out);
		*p = '[';
		pbase = p;
		if ((p = strchr(pbase, ']')) == NULL) {
			(*outputfun) ("[$", out);
			pbase += 2;
			continue;
		}
		*p = 0;
		if ((i = tmpl_checknum(pbase + 2, answer_num)) == -1) {
			(*outputfun) (pbase, out);
			*p = ']';
			pbase = p;
			continue;
		}
		*p = ']';
		pbase = p + 1;
		(*outputfun) (alist[i - 1], out);
	}
}

int
tmpl_set_filename(char *o_buf, int i_size, char *i_board, int i_num,
		  char i_suffix)
{
	snprintf(o_buf, i_size, "boards/%s/T.%d.%c", i_board, i_num, i_suffix);
	return 1;
}

int
get_num_psttmpls(char *board)
{
	char dir[PATH_MAX];
	struct stat st;
	snprintf(dir, sizeof (dir), "boards/%s/.TMPL", board);
	if (stat(dir, &st) == -1)
		return 0;
	return st.st_size / sizeof (struct posttmplheader);
}

int
tmpl_cmp_psttmpls(struct posttmplheader *phdr, int *pfiletime)
{
	if(phdr->filetime == *pfiletime) {
		return 1;
	}
	return 0;
}
