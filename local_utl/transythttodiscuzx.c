/* This file will transfer the data from ytht files to discuzX */
/* TRANSUSER for how many users you want to transfer to discuzX */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <iconv.h>    // convert gb2312 to utf8
#include "bbs.h"
#include "discuzsql.h"  // like usesql.h define the name,password and database of discuz
#include "discuzmodule.h"  // for discuz PHP modules and transfer
#define PASSWDFILE ".PASSWD_discuz"
#define DISCUZ_USERNAME_LENGTH 15

int transferuser()
{
	FILE *fp;
	struct userec lookupuser;
	struct userdata data;
	int sizeofuserec = sizeof(lookupuser);
	int uid;
//	int tuid;
	uid = 1;
	char sqlbuf[512];
	char useridutf8[22];  // 21 = (IDLEN+2)/2*3   longest utf-8 and a \0 at the end
	char md5hexpasswd[DISCUZ_PASSWD_LENGTH+1];  // 32 is defined in discuz and add \0 at the end
//	char compresssalt[9];  // compress the salt number in ytht, which is a 32bit random int,
						   // to ASCII char in discuz. discuz only has 6bit length of salt
						   // the maximum length of int number is 32bit (8bytes) in C/C++

//	for database
	MYSQL *mysql = NULL;
	MYSQL_RES *res;

	if ((fp = fopen(PASSWDFILE, "r")) == NULL) {
		perror("open PASSWDFILE\n");
		return -1;
	}

	mysql = mysql_init(mysql);
    mysql = mysql_real_connect(mysql,"localhost",SQLUSER, SQLPASSWD, SQLDB,0, NULL,0);
	if (!mysql) {
		perror("Can not open database\n");
		return -1;    
	}
	mysql_query(mysql, "set names utf8");


	while (fread(&lookupuser, 1, sizeofuserec, fp) == sizeofuserec){
// print the contents of .PASSWD 
		if (strlen(lookupuser.userid) <= 2)
			continue;
		loaduserdata(lookupuser.userid, &data);
//		printf("%s  |  %ld  |  %d  |  %d  |  %s  |  %s | %s \n",
//			lookupuser.userid, lookupuser.firstlogin, lookupuser.numlogins, lookupuser.numposts, lookupuser.username, lookupuser.passwd, data.email);
		printf("%s       ", lookupuser.userid);
		if(code_convert("gbk","utf8",lookupuser.userid, strlen(lookupuser.userid),useridutf8, 22 )==-1)   // 21 = (IDLEN+2)/2*3   longest utf-8
		{
			printf("convert username error \n");
			exit(0);
		}
		if(ascii2hex(lookupuser.passwd, md5hexpasswd) == -1)
		{
			printf("convert password error \n");
			exit(0);
		}
		printf("%s\n", md5hexpasswd);
		if(strlen(useridutf8)> DISCUZ_USERNAME_LENGTH)
		{
			printf("too long id\n");
			continue;
		}
// Do not compress the salt, use another available key lastlogtime int(10) for salt saving
/*		if(int2ascii(lookupuser.salt, compresssalt) == -1)
		{
			printf("convert salt error \n");
			exit(0);
		}
*/
// write it in mysql member table
		sprintf(sqlbuf,"select username from pre_ucenter_members where username = \'%s\'; " , useridutf8 );
		mysql_query(mysql, sqlbuf);
		res = mysql_store_result(mysql);

		if (mysql_num_rows(res)!=0)
		{
			printf(" ID: %s  already in discuz\n", useridutf8);
//			continue;
		}
		else  //new id in discuz
		{
			sprintf(sqlbuf, "insert into pre_common_member (email, username, password, groupid, regdate, timeoffset) values ('%s', '%s', '%s', %d, %ld, %d)", 
				data.email, useridutf8, md5hexpasswd, 20, lookupuser.firstlogin, 9999);
			// 20 is the special group for telnet users (first login)
			mysql_query(mysql, sqlbuf);
		}

		sprintf(sqlbuf,"select username from pre_ucenter_members where username = \'%s\'; " , useridutf8 );
		mysql_query(mysql, sqlbuf);
		res = mysql_store_result(mysql);

		if (mysql_num_rows(res)!=0)
		{
			printf(" ID: %s  already in ucenter\n", useridutf8);
//			continue;
		}
		else  //new id in discuz
		{
			// save salt in lastlogintime because the length of salt in discuz is not long enough
			sprintf(sqlbuf, "insert into pre_ucenter_members (email, username, password, regdate, lastlogintime) values ('%s', '%s', '%s', %ld, %u)",
				data.email, useridutf8, md5hexpasswd, lookupuser.firstlogin, (unsigned int)lookupuser.salt);
			mysql_query(mysql, sqlbuf);
		}

	}
	mysql_close(mysql);
	return 1;
}

int
main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("Usage: transythttodiscuzx <function_name>\n");
		printf(" -common_member: transfer the new id in .PASSWD_discuz to the database of discuz\n");
		return -1;
	}
	else
	{
		if(!strcmp(argv[1], "-common_member"))
			transferuser();
		return 0;
	}
}
