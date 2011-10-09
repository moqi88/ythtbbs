/*
 * discuzreg.c
 *
 *  Created on: 2010-8-2
 *      Author: duanlian
 */
#include <stdio.h>
#include <time.h>
#include "discuzreg.h"
#include "ythtbbs.h"

int discuzreg(char* useridutf8, char* passbuf, int salt)
{
	//  add user in discuzx database synchronously when register in telnet
		char sqlbuf[512];
		char hexsalt[7]; //6 bits hex salt for discuz
		char discuzpassmd5[DISCUZ_PASSWD_LENGTH + 1];
		time_t regdate;
		MYSQL *mysql = NULL;
		MYSQL_RES *res;
		MYSQL_ROW row;

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
		time(&regdate);
		sprintf(sqlbuf,"select username from %sucenter_members where username = \'%s\'; " , DISCUZ_DATABASE_PRE, useridutf8 );
		mysql_query(mysql, sqlbuf);
		res = mysql_store_result(mysql);

		if (mysql_num_rows(res)!=0)
		{
			char log[256];
			sprintf(log, " ID: %s  already in discuz\n", useridutf8);
			writesyslog(ERRORLOG, log);
			return -1;
		}
		sprintf(sqlbuf, "insert into %sucenter_members (username, password, salt) values ('%s', '%s', '%s')",
			DISCUZ_DATABASE_PRE, useridutf8, discuzpassmd5, hexsalt);
		mysql_query(mysql, sqlbuf);
//  	Select uid, which should be unique in later stage in order to synchronize the writing
		sprintf(sqlbuf,"select uid from %sucenter_members where username = \'%s\'; " , DISCUZ_DATABASE_PRE, useridutf8 );
		mysql_query(mysql, sqlbuf);
		res = mysql_store_result(mysql);
		if(mysql_num_rows(res)!=0)
		{
			row = mysql_fetch_row(res);   //row[0] is the uid number
		}
		else
		{
			char log[256];
			sprintf(log, "ID: %s  can not find in %sucenter_member after insertion\n", DISCUZ_DATABASE_PRE, useridutf8);
			writesyslog(ERRORLOG, log);
			return -1;
		}

		sprintf(sqlbuf, "insert into %scommon_member (uid, username, password, groupid, timeoffset) values ('%s', '%s', '%s', %d, %d)",
				DISCUZ_DATABASE_PRE, row[0], useridutf8, discuzpassmd5, 10, 9999);  // 8是discuz里的等待验证会员。10是新手上路。定义在common_usergroup里
		mysql_query(mysql, sqlbuf);

		sprintf(sqlbuf, "insert into %scommon_member_count (uid, extcredits2) values ('%s', 2)", DISCUZ_DATABASE_PRE, row[0]);
		mysql_query(mysql, sqlbuf);

		sprintf(sqlbuf, "insert into %scommon_member_field_forum (uid, customshow) values ('%s', 26)", DISCUZ_DATABASE_PRE, row[0]);
		mysql_query(mysql, sqlbuf);

		sprintf(sqlbuf, "insert into %scommon_member_field_home (uid, addsize) values ('%s', 0)", DISCUZ_DATABASE_PRE, row[0]);
		mysql_query(mysql, sqlbuf);

		sprintf(sqlbuf, "insert into %scommon_member_log (uid, action, dateline) values ('%s', 'add', %ld)", DISCUZ_DATABASE_PRE, row[0], regdate);
		mysql_query(mysql, sqlbuf);

		sprintf(sqlbuf, "insert into %scommon_member_profile (uid) values ('%s')", DISCUZ_DATABASE_PRE, row[0]);
		mysql_query(mysql, sqlbuf);

		sprintf(sqlbuf, "insert into %scommon_member_status (uid) values ('%s')", DISCUZ_DATABASE_PRE, row[0]);
		mysql_query(mysql, sqlbuf);

		sprintf(sqlbuf, "insert into %sucenter_memberfields (uid) values ('%s')", DISCUZ_DATABASE_PRE, row[0]);
		mysql_query(mysql, sqlbuf);

		mysql_close(mysql);
		return 0;
}

int discuzupdateemail(char* userid, char* email, time_t firstlogin)
{
	// update email and first login time after first register
	char sqlbuf[512];
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

int changediscuzpasswd(char* userid, int salt, char* passbuf, short int renew)
{
//  change password under telnet will result in modifications in discuzx database
//	renew代表是否强制修改，为1则强制修改，为0则只在密码保持telnet格式（groupid == 20）的时候修改
	char sqlbuf[512];
	char useridutf8[22];  //21 is the longest utf8 length
	char md5hexpasswd[DISCUZ_PASSWD_LENGTH+1];  // 32 is defined in discuz and add \0 at the end
	char hexsalt[7]; //6 bits hex salt for discuz
	char log[256];

	MYSQL *mysql = NULL;
	MYSQL_RES *res;
	MYSQL_ROW row;

	mysql = mysql_init(mysql);
	mysql = mysql_real_connect(mysql,"localhost",SQLUSER,SQLPASSWD,SQLDB,0, NULL,0);
	if(!mysql){
		writesyslog(ERRORLOG, "Can not open database!\n Please contact SYSOP\n");
		return -1;
	}
	mysql_query(mysql, "set names utf8");

	if(code_convert("gbk","utf8",userid, strlen(userid),useridutf8,22)== -1)
	{
		sprintf(log, "change discuz password. Convert username %s error \n", userid);
		writesyslog(ERRORLOG, log);
		return -1;
	}

	sprintf(sqlbuf,"select groupid from %scommon_member where username = \'%s\'; " , DISCUZ_DATABASE_PRE, useridutf8 );
	mysql_query(mysql, sqlbuf);
	res = mysql_store_result(mysql);
	if (mysql_num_rows(res)!=0)
	{
		row = mysql_fetch_row(res);   //row[0] is the groupid number
		if (atoi(row[0]) == 20)   // not converted
		{
			// 根据输入的passbuf，重新生成discuzx下的salt与md5hexpasswd
			sprintf(hexsalt, "%.6x", salt&0xFFFFFF);
			gendiscuzpasswd(md5hexpasswd, hexsalt, passbuf);
			sprintf(sqlbuf, "update %sucenter_members set password = '%s', lastlogintime = '%u', salt = '%s' where username = '%s' ",
				DISCUZ_DATABASE_PRE, md5hexpasswd, 0, hexsalt, useridutf8);
			mysql_query(mysql, sqlbuf);
			sprintf(sqlbuf, "update %scommon_member set password = '%s', groupid = 10, timeoffset = 9999 where username = '%s' ",
				DISCUZ_DATABASE_PRE, md5hexpasswd, useridutf8);
			mysql_query(mysql, sqlbuf);
			sprintf(log, "transfer user %s from ytht to discuzx\n", userid);
			writesyslog(TESTLOG, log);
		}
		else  //already converted to discuzx password
		{
			if(renew == 1)   // force to change discuzx password
			{
				sprintf(sqlbuf,"select salt from %sucenter_members where username = \'%s\'; " , DISCUZ_DATABASE_PRE, useridutf8 );
				mysql_query(mysql, sqlbuf);
				res = mysql_store_result(mysql);
				row = mysql_fetch_row(res);   //row[0] is the salt number
				gendiscuzpasswd(md5hexpasswd, row[0], passbuf);
				sprintf(sqlbuf, "update %sucenter_members set password = '%s' where username = '%s' ",
					DISCUZ_DATABASE_PRE, md5hexpasswd, useridutf8);
				mysql_query(mysql, sqlbuf);
				sprintf(sqlbuf, "update %scommon_member set password = '%s' where username = '%s' ",
					DISCUZ_DATABASE_PRE, md5hexpasswd, useridutf8);
				mysql_query(mysql, sqlbuf);
			}
		}
		mysql_close(mysql);
		return 0;
	}
	else  // can not find id in discuz database
	{
		writesyslog(ERRORLOG, "Can not find discuz id in database when changing password!\n");
		return -1;
	}
}
