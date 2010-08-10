/*
 * discuzgenpasswd.c
 *
 * For PHP usage. As ytht and discuz use different md5 encryption algorithm. Thus we compile
 * this code and use it in php code by system() function.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "discuzmodule.h"
#include "crypt.h"
#include "ythtlib.h"

int main(int argc, char *argv[])
{
	char password[DISCUZ_PASSWD_LENGTH];
	int salt;
	char md5password[MD5LEN];
	char md5hexpasswd[DISCUZ_PASSWD_LENGTH+1];  // 32 is defined in discuz and add \0 at the end

	if(argc != 3)
	{
		printf("Usage: discuzgenpasswd <salt> <input_password>\n");
		exit(0);
	}
	strcpy(password, argv[2]);
	salt = atoi(argv[1]);
	genpasswd(md5password, salt, password);
	if(ascii2hex(md5password, md5hexpasswd) == -1)
	{
		printf("convert password error \n");
		exit(0);
	}
	printf("%s",md5hexpasswd);
	return 1;
}
