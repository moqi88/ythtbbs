#include "ythtlib.h"
#include <locale.h>
#include <iconv.h>
#include <string.h>

char hex2char(unsigned int hex_num)
{
	if(hex_num >= 0 && hex_num < 10)
		return '0' + hex_num;
	else if (hex_num >=10 && hex_num < 16)
		return 'a' + hex_num - 10;
	else
	{
		printf("Wrong hex number in hex2char\n");
		exit(0);
	}

}

void
strsncpy(char *s1, const char *s2, int n)
{
	int l = strnlen(s2, n);
	if (n <= 0) {
		if (n < 0) {
			errlog("n less than zero!!");
		}
		*s1 = 0;
		return;
	}
	if (n > l + 1)
		n = l + 1;
	memcpy(s1, s2, n - 1);
	s1[n - 1] = 0;
}

char *
strltrim(char *s)
{
	char *s2 = s;
	if (s[0] == 0)
		return s;
	while (s2[0] && strchr(" \t\r\n", s2[0]))
		s2++;
	return s2;
}

char *
strrtrim(char *s)
{
	static char t[1024], *t2;
	if (s[0] == 0)
		return s;
	strsncpy(t, s, sizeof(t));
	t2 = t + strlen(s) - 1;
	while (strchr(" \t\r\n", t2[0]) && t2 > t)
		t2--;
	t2[1] = 0;
	return t;
}

void
normalize(char *buf)
{
	int i = 0;
	while (buf[i]) {
		if (buf[i] == '/')
			buf[i] = ':';
		i++;
	}
}

int
myatoi(unsigned char *buf, int len)
{
	int i, k, ret = 0;
	for (i = 0; buf[i] && i < len; i++) {
		if (buf[i] >= 'a')
			k = buf[i] - 'a' + 26;
		else if (buf[i] >= 'A')
			k = buf[i] - 'A';
		else if (buf[i] >= '0')
			k = buf[i] - '0' + 52;
		else
			k = buf[i] - '*' + 62;
		ret = (ret << 6) + k;
	}
	return ret;
}

static const unsigned char mybase64[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789*+";

void
myitoa(int num, unsigned char *buf, int len)
{
	buf[len] = 0;
	while (len--) {
		buf[len] = mybase64[num & 0x3f];
		num = num >> 6;
	}
}

void
filteransi(char *line)
{
	int i, stat, j;
	stat = 0;
	j = 0;
	for (i = 0; line[i]; i++) {
		if (line[i] == '\033')
			stat = 1;
		if (!stat) {
			if (j != i)
				line[j] = line[i];
			j++;
		}
		if (stat && ((line[i] > 'a' && line[i] < 'z')
			     || (line[i] > 'A' && line[i] < 'Z')
			     || line[i] == '@'))
			stat = 0;
	}
	line[j] = 0;
}

char *
utf8cut(char *str, int maxsize)
{
        int len = maxsize - 1;
        setlocale(LC_CTYPE, "zh_CN.utf8");
        str[len] = 0;
        while (len > 0 && mbstowcs(NULL, str, 0) == (size_t) (-1)) {
                len--;
                str[len] = 0;
        }
        setlocale(LC_CTYPE, "");
        return str;
}
                                                                                                                                            
static char * convertEncoding(char *to0, int tolen0, char *from0, int flag) {
        static iconv_t cd[2] = {(iconv_t)(-1), (iconv_t)(-1)};
        char *to = to0, *from = from0;
        int retv, tolen, flen = strlen(from);
                                                                                                                                            
        if (cd[flag] == (iconv_t) - 1) {
		switch(flag) {
		case 0:
	                cd[flag] = iconv_open("UTF-8", "GB2312"); // gb to utf8
			break;
		case 1:
	                cd[flag] = iconv_open("GB2312", "UTF-8");
			break;
		default:
			break;
		}
	}
        if (cd[flag] == (iconv_t) - 1) { // open failed...
                strsncpy(to0, from0, tolen0);
                return to0;
        }
        tolen = tolen0 - 1;
        retv = iconv(cd[flag], &from, &flen, &to, &tolen);
        //iconv_close(cd[flag]);
        if (retv != -1) {
                to0[tolen0 - 1 - tolen] = 0;
        } else {
                strsncpy(to0, from0, tolen0);
        }
        return to0;
}

char *
gb2utf8(char *to0, int tolen0, char *from0)
{
	return convertEncoding(to0, tolen0, from0, 0);
}

char *
utf82gb(char *to0, int tolen0, char *from0)
{
	return convertEncoding(to0, tolen0, from0, 1);
}

int code_convert(const char *from_charset,const char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
	iconv_t cd;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset,from_charset);
	if (cd==0) {printf("iconv open err\n"); return -1;}
	memset(outbuf,0,outlen);
	if (iconv(cd,pin,&inlen,pout,&outlen)==-1) {printf("iconv conv err\n"); return -1;}
	iconv_close(cd);
	return 0;
}
int ascii2hex(char* ascii, char* hex )
{
	int i,j,ascii_len, hex_len;
	char hi4;  // high 4 digits of ascii alphabet
	char low4; // low 4 digits of ascii alphabet

	if(ascii==NULL || hex==NULL)
	{
		printf("invalid input or output");
		return -1;
	}
	ascii_len=16;
	hex_len = 32; // discuz passwd length
	i=0;j=0;
	while(i<ascii_len)
	{
		if(ascii[i] == '\0')
			break;
		hi4 = (ascii[i] >> 4) & 0x0f;
		low4 = ascii[i] & 0x0f;
		i++;
		hex[j] = hex2char((unsigned int)hi4);
		hex[j+1] = hex2char((unsigned int)low4);
		j+=2;
	    if ( j > hex_len )
	    {
	    	printf("Wrong passwd hex length\n");
			exit(0);
		}
	}
	hex[33]= '\0';
	return 1;
}

int int2ascii(int num, char* ascii)
{
	if(ascii==NULL)
	{
		printf("invalid input or output");
		return -1;
	}
	int i=0;
	while(num!=0)
	{
		ascii[i]= num & 0xff;
		num = ((unsigned int)num) >> 8;
		i++;
	}
	ascii[i] = '\0';
	return 1;
}
