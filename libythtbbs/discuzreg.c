/*
 * discuzreg.c
 *
 *  Created on: 2010-8-2
 *      Author: duanlian
 */
#include <stdio.h>
#include "discuzreg.h"
#include "ythtbbs.h"

int discuzreg(char* useridutf8, char* passbuf, int salt)
{
	//  add user in discuzx database synchronously when register in telnet
		char sqlbuf[512];
		char hexsalt[7]; //6 bits hex salt for discuz
		char discuzpassmd5[DISCUZ_PASSWD_LENGTH + 1];
		MYSQL *mysql = NULL;
		MYSQL_RES *res;

		mysql = mysql_init(mysql);
		mysql = mysql_real_connect(mysql,"localhost",SQLUSER,SQLPASSWD,SQLDB,0, NULL,0);
		if(!mysql){
			writesyslog(ERRORLOG, "Can not open database!\n Please contact SYSOP\n");
			return -1;
		}
		mysql_query(mysql, "set names utf8");
		sprintf(hexsalt, "%.6x", salt&0xFFFFFF);
		gendiscuzpasswd(discuzpassmd5, hexsalt, passbuf);
		if(strlen(useridutf8)> DISCUZ_USERNAME_LENGTH)
		{
			char log[256];
			sprintf(log, "too long register id %s\n", useridutf8);
			writesyslog(ERRORLOG, log);
			return -1;
		}
		sprintf(sqlbuf,"select username from pre_ucenter_members where username = \'%s\'; " , useridutf8 );
		mysql_query(mysql, sqlbuf);
		res = mysql_store_result(mysql);

		if (mysql_num_rows(res)!=0)
		{
			char log[256];
			sprintf(log, " ID: %s  already in discuz\n", useridutf8);
			writesyslog(ERRORLOG, log);
			return -1;
		}
		sprintf(sqlbuf, "insert into pre_common_member (username, password, groupid, timeoffset) values ('%s', '%s', %d, %d)",
			useridutf8, discuzpassmd5, 8, 9999);  // 8 for normal register id in discuz
		mysql_query(mysql, sqlbuf);

		sprintf(sqlbuf, "insert into pre_ucenter_members (username, password, salt) values ('%s', '%s', '%s')",
			useridutf8, discuzpassmd5, hexsalt);
		mysql_query(mysql, sqlbuf);

		mysql_close(mysql);
		return 0;
}

int discuzupdateemail(char* userid, char* email, time_t firstlogin)
{
	// update email and first login time after first register
	char sqlbuf[512];
	char md5hexpasswd[DISCUZ_PASSWD_LENGTH+1];
	char useridutf8[22];  //21 is the longest utf8 length

	MYSQL *mysql = NULL;

	mysql = mysql_init(mysql);
	mysql = mysql_real_connect(mysql,"localhost",SQLUSER,SQLPASSWD,SQLDB,0, NULL,0);
	if(!mysql){
		writesyslog(ERRORLOG, "Can not open database!\n Please contact SYSOP\n");
		return -1;
	}
	mysql_query(mysql, "set names utf8");

	if(code_convert("gbk","utf8",userid, strlen(userid),useridutf8,22)== -1)
	{
		char log[256];
		sprintf(log, "Convert username %s error \n", userid);
		writesyslog(ERRORLOG, log);
		return -1;
	}
	sprintf(sqlbuf, "update pre_common_member set email = '%s', regdate = %ld where username = '%s'",
		email, firstlogin, userid);
	mysql_query(mysql, sqlbuf);
	sprintf(sqlbuf, "update pre_ucenter_members set email = '%s', regdate = %ld where username = '%s'",
		email, firstlogin, userid);
	mysql_query(mysql, sqlbuf);
	mysql_close(mysql);
	return 0;

}
