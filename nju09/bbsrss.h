#include "bbslib.h"
#include <iconv.h>

#define RSS_MAX_DIGEST_ITEM	20
#define RSS_MAX_BOARD_ITEM	20

static int istitle = 0;
static void 
rss_hsprintf(char *s, char *s0)
{
	static const char specchar[256] = {['\\'] = 1,['$'] = 1,['"'] =
		    1,['&'] = 1,['<'] = 1,['>'] = 1,[' '] = 1,['\n'] =
		    1,['\r'] = 1,['\033'] = 1
	};
	int c, m, i, len;
	static int dbchar = 0;
	int lastdb;
	char buf[256];
	static struct bbsfont bf;
	static int infont;
	if (s == NULL) {
		bzero(&bf, sizeof (bf));
		bf.color = 7;
		infont = 0;
		withinMath = 0;
		usingMath = 0;
		usedMath = 0;
		contentbg = 0;
		return;
	}
	len = 0;
	for (i = 0; (c = s0[i]); i++) {
		lastdb = dbchar;
		if (dbchar)
			dbchar = 0;
		else if (c & 0x80)
			dbchar = 1;

		if (!specchar[(unsigned char) c]) {
			s[len++] = c;
			continue;
		}
		if (lastdb && len) {
			len--;
		}
		switch (c) {
		case '\\':
			if (usingMath && !withinMath && s0[i + 1] == '[') {
				strnncpy2(s, &len, "<div class=math>", 16);
				i++;
				withinMath = 2;
			} else if (usingMath && withinMath == 2
				   && s0[i + 1] == ']') {
				strnncpy2(s, &len, "</div>", 6);
				i++;
				withinMath = 0;
			} else if (usingMath && s0[i + 1] == '$') {
				s[len++] = '$';
				i++;
			} else if (quote_quote)
				strnncpy2(s, &len, "\\\\", 2);
			else
				s[len++] = c;
			break;
		case '$':
			if (usingMath && !withinMath) {
				strnncpy2(s, &len, "<span class=math>", 17);
				withinMath = 1;
			} else if (usingMath && withinMath == 1) {
				strnncpy2(s, &len, "</span>", 7);
				withinMath = 0;
			} else
				s[len++] = c;
			break;
		case '"':
			if (quote_quote)
				strnncpy2(s, &len, "\\\"", 2);
			else
				s[len++] = c;
			break;
		case '&':
			strnncpy2(s, &len, "&amp;", 5);
			break;
		case '<':
			strnncpy2(s, &len, "&lt;", 4);
			break;
		case '>':
			strnncpy2(s, &len, "&gt;", 4);
			break;
		case ' ':
			if (!i || s0[i - 1] != ' ') {
				s[len++] = c;
			} else {
				strnncpy2(s, &len, "&amp;nbsp;", 10);
			}
			break;
		case '\r':
			break;
		case '\n':
			if (withinMath) {
				s[len++] = ' ';
				break;
			}
			if (quote_quote)
				strnncpy2(s, &len, " \\\n<br>", 7);
			else if (!istitle)
				strnncpy2(s, &len, "\n<br>", 5);
			break;
		case '\033':
			if (s0[i + 1] == '\n') {
				i++;
				continue;
			}
			if (s0[i + 1] == '\r' && s0[i + 2] == '\n') {
				i += 2;
				continue;
			}
			if (s0[i + 1] == '<') {
				for (m = 2; s0[i + m] && s0[i + m] != '>';
				     m++) ;
				if (s0[i + m])
					i += m;
				continue;
			}
			if (s0[i + 1] != '[')
				continue;
			if (s0[i + 2] == '<') {
				int savelen = 0;
				savelen = len;
				for (m = i + 2; s0[m]; m++) {
					if (quote_quote
					    && (s0[m] == '\"'
						|| s0[m] == '\\')) {
						s[len++] = '\\';
						s[len++] = s0[m];
					} else if (quote_quote
						   && s0[m - 1] == '<'
						   && s0[m] == '/'
						   && tolower(s0[m + 1]) ==
						   's') {
						s[len++] = '\\';
						s[len++] = '\n';
						s[len++] = '/';
					} else
						s[len++] = s0[m];
					if (strchr(">\r\n", s0[m]))
						break;
				}
				if (s0[m] != '>') {
					i = i + 2;
					len = savelen;
					continue;
				}
				if (infont) {
					strnncpy2(s, &len, "</font>", 7);
					infont = 0;
				}
				m++;
				i = m - 1;
				if (fontstr(&bf, buf)) {
					infont = 1;
					strnncpy2(s, &len, buf, strlen(buf));
				}
				break;
			}
			for (m = i + 2; s0[m] && m < i + 24; m++)
				if (strchr("0123456789;", s0[m]) == 0)
					break;
			if (s0[m] != 'm') {
				i = m;
				continue;
			}
			ansifont(s0 + i + 2, m - (i + 2), &bf);
			if (infont) {
				strnncpy2(s, &len, "</font>", 7);
				infont = 0;
			}
			if (fontstr(&bf, buf)) {
				infont = 1;
				strnncpy2(s, &len, buf, strlen(buf));
			}
			i = m;
			break;
		default:
			s[len++] = c;
		}
	}
	s[len] = 0;
}

static void 
rss_fqhprintf(FILE * output, char *str)
{
	static char buf[8096];
	rss_hsprintf(buf, str);
	fputs(buf, output);
}

static int
rss_fhhprintf(FILE * output, char *fmt, ...) {
	char buf0[1024], buf[1024], *s;
	int len;
	va_list ap;
	va_start(ap, fmt);
	len = vsnprintf(buf, 1023, fmt, ap);
	va_end(ap);
	buf[1023] = 0;
	s = buf;
	if (w_info->link_mode) {
		rss_fqhprintf(output, buf);
		return 0;
	}
	if (len < 10 || !strchr(s + 3, ':')) {
		rss_fqhprintf(output, buf);
		return 0;
	}
	if (strstr(s, "\033[<")
	    || ((!strcasestr(s, "http://") && !strcasestr(s, "ftp://")
		&& !strcasestr(s, "mailto:")) && !strcasestr(s, "https://"))) {
		rss_fqhprintf(output, buf);
		return 0;
	}
	if (!strncmp(s, "±ê  Ìâ:", 7)) {
		rss_fqhprintf(output, buf);
		return 0;
	}
	len = 0;
	while (s[0]) {
		if (!strncasecmp(s, "http://", 7)
		    || !strncasecmp(s, "mailto:", 7)
		    || !strncasecmp(s, "ftp://", 6)
		    || !strncasecmp(s,"https://", 8)) {
			char *tmp, *noh, tmpchar;
			if (len > 0) {
				buf0[len] = 0;
				rss_fqhprintf(output, buf0);
				len = 0;
			}
			tmp = s;
			while (*s && !strchr("<>\'\" \r\t)(,;\n\033", *s))
				s++;
			tmpchar = *s;
			*s = 0;
			if (1) {
				if (!strcasecmp(s - 4, ".gif")
				    || !strcasecmp(s - 4, ".jpg")
				    || !strcasecmp(s - 4, ".bmp")) {
					fprintf(output, "<IMG SRC='%s'>",
						nohtml(tmp));
					*s = tmpchar;
					continue;
				}
			}
			noh = nohtml(tmp);
			fprintf(output, "<a target=_blank href='%s'>%s</a>",
				noh, noh);
			*s = tmpchar;
			continue;
		} else {
			buf0[len] = s[0];
			if (len < sizeof (buf0) - 1)
				len++;
			s++;
		}
	}
	if (len) {
		buf0[len] = 0;
		rss_fqhprintf(output, buf0);
	}
	return 0;
}

int mem2rss(FILE *fp, char *str, int memsize, char *filename) {
	char *str0, *ptr, buf[512];
	int size, len, ano = 0;

	str0 = str;
	size = memsize;
	fdisplay_attach(NULL, NULL, NULL, NULL);
	while(size) {
		len = min(size, sizeof(buf) - 1);
		ptr = str;
		str = memchr(ptr, '\n', len);
		if (!str && len == sizeof(buf) - 1) {
			str = memchr(ptr + 1, '\033', len);
			if (str)
				str--;
		}
		if (!str)
			str = ptr + len;
		else {
			str++;
			len = str - ptr;
		}
		size -= len;
		if (len > 18 && !strncmp(ptr, "beginbinaryattach ", 18) 
				&& size >= 5 && !*str) {
			unsigned int attlen;
			strsncpy(buf, ptr, len);
			ptr = strchr(buf, '\r');
			if (ptr)
				*ptr = 0;
			ptr = strchr(buf, '\n');
			if (ptr)
				*ptr = 0;
			ano++;
			ptr = buf + 18;
			attlen = ntohl(*(unsigned int *) (str + 1));
			printf("<p>[¸½¼þ %d (%.2f KB)] %s</p>", ano,
					(double)(attlen / 1024), ptr);
			attlen = min(size, (int)(attlen + 5));
			str += attlen;
			size -= attlen;
			continue;
		}
		rss_fhhprintf(stdout, "%.*s", len, ptr);
	}
	return 1;
}

static void
rss_date(char *str) {
	char *ptr, *tmp;

	ptr = strchr(str, ' ');
	if (!ptr)
		return;
	*ptr = 0;
	ptr++;
	printf("%s, ", str);
	tmp = ptr;
	ptr = strchr(ptr, ' ');
	if (!ptr)
		return;
	*ptr = 0;
	ptr++;
	if (*ptr == ' ')
		ptr++;
	printf("%2.2d %s ", atoi(ptr), tmp);
	ptr = strchr(ptr, ' ');
	if (!ptr)
		return;
	*ptr = 0;
	tmp = ++ptr;
	ptr = strchr(ptr, ' ');
	if (!ptr)
		return;
	*ptr = 0;
	ptr++;
	printf("%s %s +0800", ptr, tmp);
	
	return;
}
