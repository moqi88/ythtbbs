#include <stdio.h>
#include <stdlib.h>
#include "bmplib.h"

int main(int argc, char *argv[])
{
	bmphandle_t bh;
	char buf[4000000];
	int len, h, w, i, j;
	FILE *fp;
	struct bgrpixel r;
	fp=fopen(argv[1], "r");
	if(NULL==fp){
		printf("argv\n");
		return -1;
	}
	len=fread(buf, 1, sizeof(buf), fp);
	bh = bmp_open(buf, len);
	if (bh == NULL){
		printf("img error\n");
		return -2;
	}
	h = bmp_height(bh);
	w = bmp_width(bh);
	for (i = 0; i < w; i++) {
		for (j = 0; j < h; j++) {
			r = bmp_getpixel(bh, i, j);
			printf("%d %d %d %d %d\n", i,j,r.b,r.g,r.r);
		}
	}
	return 0;
}
