#include "bbs.h"
#include <openssl/des.h>

void
g_key(char *file, char *magic)
{
	FILE *fp;
	des_cblock k;
	des_key_schedule ks;
	int i, j, ret;

	fp = fopen(file, "w");
	if (NULL == fp) {
		printf("can't open file %s\n", file);
		return;
	}
	fwrite(magic, 1, 8, fp);
	i = j = 0;
	while (i < 3 && j < 100) {
		j++;
		ret = des_random_key(&k);
		if (!ret)
			continue;
		ret = des_set_key_checked(&k, ks);
		if (ret < 0)
			continue;
		fwrite(&k, 1, sizeof (k), fp);
		i++;
	}
	if (j == 100) {
		printf("can't gen key!\n");
		fclose(fp);
		unlink(file);
	}
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("error! please enter two magic string!\n");
		return -1;
	}
	if (strlen(argv[1]) < 8 || strlen(argv[2]) < 8) {
		printf("error! no enough ...!\n");
		return -2;
	}
	printf("magic1:%s\nmagic2:%s\n", argv[1], argv[2]);
	g_key(MY_BBS_HOME "/etc/my_key", argv[1]);
	g_key(MY_BBS_HOME "/etc/service_key", argv[2]);
	return 0;
}
