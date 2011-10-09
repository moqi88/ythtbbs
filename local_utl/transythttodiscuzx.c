/* This file will transfer the data from ytht files to discuzX */
/* TRANSUSER for how many users you want to transfer to discuzX */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <iconv.h>    // convert gb2312 to utf8
#include "bbs.h"
#include "record.h"
#include "sysrecord.h"
#include "discuzsql.h"  // like usesql.h define the name,password and database of discuz
#include "discuzmodule.h"  // for discuz PHP modules and transfer
#define PASSWDFILE ".PASSWD_discuz"
#define DISCUZ_USERNAME_LENGTH 15
#define UBBFILESIZE 1024*1024*4

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
	MYSQL_ROW row;

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
// Use another available key lastlogtime int(10) for salt saving
		sprintf(sqlbuf,"select username from %sucenter_members where username = \'%s\'; " , DISCUZ_DATABASE_PRE, useridutf8 );
		mysql_query(mysql, sqlbuf);
		res = mysql_store_result(mysql);

		if (mysql_num_rows(res)!=0)
		{
			// we have this id in discuz database
			printf(" ID: %s  already in ucenter\n", useridutf8);
			// select groupid and passwordmd5
			sprintf(sqlbuf,"select password, groupid from %scommon_member where username = \'%s\'; " , DISCUZ_DATABASE_PRE, useridutf8 );
			mysql_query(mysql, sqlbuf);
			res = mysql_store_result(mysql);
			if(mysql_num_rows(res)!=0)
			{
				row = mysql_fetch_row(res);   //row[0] is the md5 password in discuz database, row[1] is groupid
			}
			else
			{
				printf("ID: %s  can not find in %scommon_member when updating\n", useridutf8, DISCUZ_DATABASE_PRE);
				continue;
			}
			if (atoi(row[1]) == 20)  // still not convert to discuz password md5, groupid == 20
			{
				if(strcmp(row[0], md5hexpasswd) != 0) // not equal
				{
					printf("password not match!\t   discuz database: %s      md5 gen: %s\n",row[0], md5hexpasswd );
					// write in database
					sprintf(sqlbuf, "update %sucenter_members set password = '%s' where username = '%s' ",
						DISCUZ_DATABASE_PRE, md5hexpasswd, useridutf8);
					mysql_query(mysql, sqlbuf);
					sprintf(sqlbuf, "update %scommon_member set password = '%s' where username = '%s' ",
						DISCUZ_DATABASE_PRE, md5hexpasswd, useridutf8);
					mysql_query(mysql, sqlbuf);
				}
			}
			continue;
		}
		//new id in discuz
		// save salt in lastlogintime because the length of salt in discuz is not long enough
		sprintf(sqlbuf, "insert into %sucenter_members (email, username, password, regdate, lastlogintime) values ('%s', '%s', '%s', %ld, %u)",
			DISCUZ_DATABASE_PRE, data.email, useridutf8, md5hexpasswd, lookupuser.firstlogin, (unsigned int)lookupuser.salt);
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
			printf("ID: %s  can not find in %sucenter_member after insertion\n", useridutf8, DISCUZ_DATABASE_PRE);
			continue;
		}

		sprintf(sqlbuf, "insert into %scommon_member (uid, email, username, password, groupid, regdate, timeoffset) values ('%s', '%s', '%s', '%s', %d, %ld, %d)",
			DISCUZ_DATABASE_PRE, row[0], data.email, useridutf8, md5hexpasswd, 20, lookupuser.firstlogin, 9999);
		// 20 is the special group for telnet users (first login)
		mysql_query(mysql, sqlbuf);

		sprintf(sqlbuf, "insert into %scommon_member_count (uid, extcredits2) values ('%s', 2)", DISCUZ_DATABASE_PRE, row[0]);
		mysql_query(mysql, sqlbuf);

		sprintf(sqlbuf, "insert into %scommon_member_field_forum (uid, customshow) values ('%s', 26)", DISCUZ_DATABASE_PRE, row[0]);
		mysql_query(mysql, sqlbuf);

		sprintf(sqlbuf, "insert into %scommon_member_field_home (uid, addsize) values ('%s', 0)", DISCUZ_DATABASE_PRE, row[0]);
		mysql_query(mysql, sqlbuf);

		sprintf(sqlbuf, "insert into %scommon_member_log (uid, action, dateline) values ('%s', 'add', %ld)", DISCUZ_DATABASE_PRE, row[0], lookupuser.firstlogin);
		mysql_query(mysql, sqlbuf);

		sprintf(sqlbuf, "insert into %scommon_member_profile (uid) values ('%s')", DISCUZ_DATABASE_PRE, row[0]);
		mysql_query(mysql, sqlbuf);

		sprintf(sqlbuf, "insert into %scommon_member_status (uid) values ('%s')", DISCUZ_DATABASE_PRE, row[0]);
		mysql_query(mysql, sqlbuf);

		sprintf(sqlbuf, "insert into %sucenter_memberfields (uid) values ('%s')", DISCUZ_DATABASE_PRE, row[0]);
		mysql_query(mysql, sqlbuf);

	}
	mysql_close(mysql);
	return 1;
}

int
transferboard(char* boardname, int timep, int trans_article_num)
{
	FILE *fp;
	struct boardheader fh;
	int pos;
	char boardcnameutf8[24];
	char attachfilenameutf8[256];
	char sqlbuf[1024];
	char filepath[1024];
	char boardbuf[STRLEN], transferrecord[STRLEN], dxthreadrecord[STRLEN];
	char currdirect[STRLEN];  // for board article list
	int ssize = sizeof (struct fileheader);
	int last_line, id, article_transferred;
	struct fileheader filefh;
	int fid;
	char usernameutf8[22], titleutf8[200], useip[20];
	char lasttransfernum[80];
	char *tmpnum;
	int authorid, tid, threadmapcount, postmode, pid;
	// authorid is the id of author in discuz
	// tid is the thread id in discuz.
	// threadmapcount is the location of the thread id mapping between ytht and discuz
	// decide if the post is a new thread or reply and save in postmode
	// pid is the post id in discuz
	struct threaddiscuzxytht map[1024];
	int i;
	char *sqlfile;
	char *ubbresult; // for file insert sql
	char *ubbresultutf8; // transfer file to utf-8
	char *sqlfileend;  // point to the end of sql file
//	struct mmapfile mf={ptr:NULL};  // for file storing the last transfer record
//	int transfilesize;
	char *line = NULL;
	char threadmatch[80];
	int row_len, tmpthread, tmpythtthreadid;
	short int isimage; // 0 no attachment 1 not image  2 image
	int aid;  // attachment id

	//	for database
	MYSQL *mysql = NULL;
	MYSQL_RES *res;
	MYSQL_ROW row;

	//该结构体用于传递附件
	//forum_post的attachment 0为无附件，1可能为非图片附件，2可能为图片附件
	//附件详细情况在	pre_forum_attachment
	//受影响的库有：	pre_forum_attachmentfield

	struct attachmentstruct attachmentstructlist[MAX_ATTACHMENT_NUM];  // maximum 50 attachments in one post

	printf("transfer board -- boardname: %s  , startdate: %d , trans num: %d\n", boardname, timep, trans_article_num);
	srand(time(0));

	if (*boardname == '\0') {
		printf("Empty or wrong board name. \n");
		return -1;
	}
	pos = new_search_record(BOARDS, &fh, sizeof (fh), (void *) cmpbnames, boardname);
	if (!pos) {
		printf("Can not find the board name. \n");
		return -1;
	}
	printf("English name: %s\nChinese name: %s   ", fh.filename, fh.title);

	if (fh.flag & INNBBSD_FLAG)
		printf("cn.bbs转信");
	else if (fh.flag2 & NJUINN_FLAG)
		printf("点对点转信");
	else if (fh.flag2 & DISCUZWEB_FLAG)
		printf("DX WEB互通，DX版面号：%d", fh.discuzxfid);
	else
		printf("不转信");
	printf("\n");

	if(code_convert("gbk","utf8",fh.title, strlen(fh.title),boardcnameutf8, 24 )==-1)  // 24 is the length of title
	{
		printf("convert board cname error \n");
		return -1;
	}

	mysql = mysql_init(mysql);
    mysql = mysql_real_connect(mysql,"localhost",SQLUSER, SQLPASSWD, SQLDB,0, NULL,0);
	if (!mysql) {
		perror("Can not open database\n");
		return -1;
	}
	mysql_query(mysql, "set names utf8");

	sprintf(sqlbuf,"select fid from %sforum_forum where name = \'%s\'; " , DISCUZ_DATABASE_PRE, boardcnameutf8 );
	mysql_query(mysql, sqlbuf);
	res = mysql_store_result(mysql);

	if (mysql_num_rows(res)!=0)
	{
		row = mysql_fetch_row(res);
		fid = atoi(row[0]);
		printf("board %s found in discuzx, fid = %d \n", fh.title, fid);
	}
	else
	{
		printf("board %s not found!\n", fh.title);
		return -1;
	}

	// read file
	sprintf(boardbuf, "boards/%s/%s", boardname, DOT_DIR);
	strcpy(currdirect, boardbuf);
	last_line = get_num_records(currdirect, ssize);
	printf("last_line=%d\n", last_line);
	// 判断上一次传到什么地方了
	sprintf(transferrecord, "boards/%s/%s", boardname, TRANSRECORD_DIR);
	printf("%s\n", transferrecord);
	if((fp = fopen(transferrecord, "a+"))==NULL)
	{
		// if no file, create new file
		printf("create new record file\n");
		id = 1;
	}
	else
	{
		if(fgets(lasttransfernum, 80, fp)!=NULL)
		{
//			printf("%s\n", lasttransfernum);
			tmpnum = strtok(lasttransfernum, ":");
			if(tmpnum != NULL)
			{
				tmpnum = strtok(NULL, ":");
				printf("start id num=%s\n", tmpnum);
				id = atoi(tmpnum);
			}
			else
				id = 1;
		}
		else
			id = 1;
	}
	fclose(fp);
	threadmapcount = 0;
	article_transferred = 0;
	aid = 0;
	// begin testing
	printf("begin testing id=%d\n", id);
	while(id<= last_line && article_transferred<trans_article_num)
	{
		if(get_record(currdirect, &filefh, ssize, id)==0)
		{
			printf("%d   ", filefh.filetime);
			printf("%d-%s\n", id, fh2fname(&filefh));
			printf("%s\n", filefh.owner);
			if(filefh.owner[0] == '-')
			{
				continue;
			}
			// judge if the date is more than start_date
			tid = 0;
			if(filefh.filetime > timep)
			{
				printf("filetime=%d  timep=%d\n", filefh.filetime, timep);
				postmode = NEWTHREAD;
				// map数组存放thread id的ytht与discuzx对应表，保存在版面目录下的
				sprintf(dxthreadrecord, "boards/%s/%s", boardname, DX_THREAD);
				if((fp = fopen(dxthreadrecord, "a+"))==NULL)
				{
					// if no file, create new file
					printf("create new DX thread record file\n");
				}
				else
				{
					// 由于跟贴和主贴往往离的较近，所以先从一个近期存储的thread对应表里取。
					// 取不到的话，尝试从文件里头取。否则是新thread
					for (i = 0; i < threadmapcount ; i++ )
					{
						if(filefh.thread == map[i].ythttid)
						{
							postmode = REPLY;
							tid = map[i].discuzxtid;  // discuzx thread id selected
							break;
						}
					}
					if(postmode!=REPLY)  // if did not find in map[], then try to search in file .DXTHREAD
					{
						while(getline(&line, &row_len, fp)>0)
						{
							sscanf(line, "%d %d", &tmpthread, &tmpythtthreadid);
							if(tmpythtthreadid == filefh.thread)
							{
								map[threadmapcount].discuzxtid = tmpthread;
								map[threadmapcount].ythttid = tmpythtthreadid;
								tid = tmpthread;
								postmode = REPLY;
								break;
							}
						}
					}
				}
				fclose(fp);
				// put it in the database
				sprintf(filepath, "boards/%s/%s", boardname, fh2fname(&filefh));
				// for forum_thread
				// 创建一个文件空间，并进行文件转换
				// malloc的参数是size_t类型，上限与unsigned int一致。42M
				// 一般来说，用realloc较好，但是因为是临时程序，不管这么多了。
				ubbresult = (char*)malloc(UBBFILESIZE);  // omit sizeof(char) because it's default setting
				if(ythtfiletodiscuzxubb(filepath, ubbresult, useip, attachmentstructlist)== -1)
				{
					printf("read ytht file error \n");
					return -1;
				}
				ubbresultutf8 = (char*)malloc(UBBFILESIZE); // transfer ubbresult from gbk to itf8
				if(code_convert("gbk","utf8",ubbresult, strlen(ubbresult),ubbresultutf8, strlen(ubbresult)*2)== -1)
				{
					printf("convert article contents warning\n");
//					return -1;
				}
				free(ubbresult);  //防止泄漏
				// 生成数据库所需的各项参数
				if(code_convert("gbk","utf8",filefh.owner, strlen(filefh.owner),usernameutf8, 22 )==-1)
				{
					printf("convert user cname error \n");
					return -1;
				}
				if(code_convert("gbk","utf8",filefh.title, strlen(filefh.title),titleutf8, 200 )==-1)
				{
					printf("convert title error \n");
					return -1;
				}
				sprintf(sqlbuf,"select uid from %scommon_member where username = \'%s\'; " , DISCUZ_DATABASE_PRE, usernameutf8 );
				mysql_query(mysql, sqlbuf);
				res = mysql_store_result(mysql);

				if (mysql_num_rows(res)!=0)
				{
					row = mysql_fetch_row(res);
					authorid = atoi(row[0]);
					printf("user %s found in discuzx, uid = %d \n", filefh.owner, authorid);
				}
				else
				{
					printf("user %s not found!\n", filefh.owner);
					authorid = 0;
					id ++;
					continue;
//					return -1;
				}


				if (postmode == NEWTHREAD)
				{
					// 数据库写入，修改forum_thread
					sprintf(sqlbuf, "insert into %sforum_thread (fid, author, authorid, subject, dateline, lastpost, lastposter, stamp, icon) values ('%d','%s','%d','%s','%d','%d','%s','%d','%d');",
							DISCUZ_DATABASE_PRE, fid, usernameutf8, authorid, titleutf8, filefh.filetime, filefh.filetime, usernameutf8, -1, -1);
					mysql_query(mysql, sqlbuf);
					sprintf(sqlbuf, "select last_insert_id();");
					mysql_query(mysql, sqlbuf);
	//				tid是forum_thread表中的自增id，用last_insert_id()取回
					res = mysql_store_result(mysql);

					if (mysql_num_rows(res)!=0)
					{
						row = mysql_fetch_row(res);
						tid = atoi(row[0]);
						printf("new thread. thread id = %d \n", tid);
					}
					else
					{
						printf("thread insert error!\n");
						tid = 0;
						continue;
					}
					// map tid with the thread id in ytht
					map[threadmapcount].discuzxtid = tid;
					map[threadmapcount].ythttid = filefh.thread;
					// write new map back to dxthreadrecord
					if((fp = fopen(dxthreadrecord, "a+"))==NULL)
					{
						// if no file, create new file
						printf("create new DX thread record file\n");
					}
					else
					{
						// 将新的discuzxtid和ythtid追加在文件尾
						sprintf(threadmatch, "%d %d\n", tid, filefh.thread);
						fputs(threadmatch, fp);
					}
					fclose(fp);
				}
				else  // postmode == REPLY
				{
					// 修改forum_thread表，在相应thread下加上回复数量
					printf("old thread: thread id = %d\n", tid);
					sprintf(sqlbuf,"select replies from %sforum_thread where tid = '%d'; " , DISCUZ_DATABASE_PRE, tid );
					mysql_query(mysql, sqlbuf);
					res = mysql_store_result(mysql);

					if (mysql_num_rows(res)!=0)
					{
						row = mysql_fetch_row(res);
						sprintf(sqlbuf, "update %sforum_thread set replies = '%d', lastposter='%s', lastpost='%d' where tid = '%d'; ",
								DISCUZ_DATABASE_PRE, atoi(row[0])+1, usernameutf8, filefh.filetime, tid);
						mysql_query(mysql, sqlbuf);
					}
					else
					{
						printf("thread %d not found in discuzx!\n", tid);
						return -1;
					}
				}
				// 对new thread 和reply不敏感的其他表
				// update common_member_field_home
				// 数据库写入，修改common_member_field_home，这个似乎在导入阶段不需要
				// 只有在同步ytht版面实时发文到discuzx时才需要
/*					sprintf(sqlbuf, "update %scommon_member_field_home set recentnote='%s' where uid=%d ;",
						DISCUZ_DATABASE_PRE, titleutf8, authorid);
				mysql_query(mysql, sqlbuf);
				*/
				// insert into forum_post
				// 数据库写入，修改forum_post_tableid
				sprintf(sqlbuf, "insert into %sforum_post_tableid (pid) values (null);", DISCUZ_DATABASE_PRE);
				mysql_query(mysql, sqlbuf);
				sprintf(sqlbuf, "select last_insert_id();");
				mysql_query(mysql, sqlbuf);
//					pid是forum_post_tableid表中的自增id，用last_insert_id()取回
				res = mysql_store_result(mysql);
				if (mysql_num_rows(res)!=0)
				{
					row = mysql_fetch_row(res);
					pid = atoi(row[0]);
					printf("post id = %d \n", pid);
				}
				else
				{
					printf("post insert error!\n");
					pid = 0;
					continue;
				}
				// insert into forum_post table
				// 数据库写入，修改forum_post
				sprintf(sqlbuf, "insert into %sforum_post (pid, fid, tid, first, author, authorid, subject, dateline, useip, usesig, smileyoff) values ('%d', '%d', '%d', '%d', '%s', '%d', '%s', '%d', '%s', '1', '-1');",
						DISCUZ_DATABASE_PRE, pid, fid, tid, (postmode == NEWTHREAD? 1: 0), usernameutf8, authorid, titleutf8, filefh.filetime, useip);
				// message 很长，sql字符串里塞不下，只能容后处理。message需要使用mysql_real_escape_string()函数转义
				// ip暂时没有记录 attachment也没有处理 需要一个前置处理
				mysql_query(mysql, sqlbuf);
				// 插入message
				sprintf(sqlbuf, "update %sforum_post set message = '", DISCUZ_DATABASE_PRE);
				sqlfile = (char*)malloc(UBBFILESIZE);  // 应该超不过4M
				// stpcpy是个非常冷僻的字符串C函数，与strcpy不同的是，它返回dest string的尾部而不是头部
				sqlfileend = stpcpy(sqlfile, sqlbuf);  // sqlfileend should be the tail of the first string sqlfile
				sqlfileend += mysql_real_escape_string(mysql, sqlfileend, ubbresultutf8, strlen(ubbresultutf8));
				sprintf(sqlbuf, "' where pid = %d;", pid);
				strcat(sqlfileend, sqlbuf);
				// mysql_real_escape_string是转义ubbresultutf8这个sql语句，并加到sqlfileend所对应的内存空间中，
				// 转义最坏的情况是每个字符都需要转义，所以需要sizeof(ubbresult)*2的大小，再加上sql语句的大小
				// ubbresultutf8是ytht file转义为ubb码后的结果，由ythtfiletodiscuzxubb生成
				// 存储到forum_post表的message字段里，这个字段的类型是MEDIUMTEXT，上限为16.7MB
				// ubbresultutf8和sqlfileend都是吃大内存的，但是暂时没有办法，直接用函数返回ubbresult并不会省内存
				mysql_query(mysql, sqlfile);
				free(sqlfile);
				// 数据库写入，修改forum_forum
				sprintf(sqlbuf,"select threads, posts from %sforum_forum where fid = '%d'; " , DISCUZ_DATABASE_PRE, fid );
				mysql_query(mysql, sqlbuf);
				res = mysql_store_result(mysql);

				if (mysql_num_rows(res)!=0)
				{
					row = mysql_fetch_row(res);
					sprintf(sqlbuf, "update %sforum_forum set threads = '%d', posts = '%d', lastpost = '%d %s %d %s' where fid = '%d'; ",
							DISCUZ_DATABASE_PRE, (postmode == NEWTHREAD? atoi(row[0])+1 : atoi(row[0])), atoi(row[1])+1, tid, titleutf8, filefh.filetime, usernameutf8, fid);
					mysql_query(mysql, sqlbuf);
				}
				else
				{
					printf("forum %s not found in discuzx!\n", fh.title);
					return -1;
				}
				// 数据库写入，修改common_member_count
				sprintf(sqlbuf, "select extcredits2 from %scommon_member_count where uid = '%d'; ", DISCUZ_DATABASE_PRE, authorid);
				mysql_query(mysql, sqlbuf);
				res = mysql_store_result(mysql);

				if (mysql_num_rows(res)!=0)
				{
					row = mysql_fetch_row(res);
					sprintf(sqlbuf, "update %scommon_member_count set extcredits2 = '%d' where uid = '%d'; ",
							DISCUZ_DATABASE_PRE, atoi(row[0])+1, authorid);
					mysql_query(mysql, sqlbuf);
				}
				else
				{
					printf("userid %d not found in discuzx!\n", authorid);
					return -1;
				}

				free(ubbresultutf8);

				// 处理attachment
				isimage = 0;
				for(i = 0; i < MAX_ATTACHMENT_NUM; i++)
				{
//					printf("attachment outside OUT\n");
					printf("attachmentstructlist[%d]: ano=%d  width=%d  dateline=%ld  \n", i, attachmentstructlist[i].ano, attachmentstructlist[i].width, attachmentstructlist[i].dateline);
					printf("filename=%s  filesize=%d \n ",attachmentstructlist[i].filename, attachmentstructlist[i].filesize);
					printf("attachment=%s  isimage=%d \n ", attachmentstructlist[i].attachment, attachmentstructlist[i].isimage);
					if(attachmentstructlist[i].ano !=-1)
					{
						if(attachmentstructlist[i].isimage ==1)
						{
							isimage = 2;
						}
						else
						{
							isimage = 1;
						}
						sprintf(sqlbuf, "insert into %sforum_attachment (pid, tid, uid, tableid) values ('%d', '%d', '%d', '%d');",
								DISCUZ_DATABASE_PRE, pid, tid, authorid, tid%10);
						mysql_query(mysql, sqlbuf);
						sprintf(sqlbuf, "select last_insert_id();");
						mysql_query(mysql, sqlbuf);
						res = mysql_store_result(mysql);
						if (mysql_num_rows(res)!=0)
						{
							row = mysql_fetch_row(res);
							aid = atoi(row[0]);
							printf("attachment id = %d \n", aid);
						}
/*						printf("insert into %sforum_attachment_%d (aid, pid, tid, width, dateline, filename, filetype, filesize, attachment, isimage, uid) values ('%d', '%d', '%d', '%d', '%ld', '%s', 'application/octet-stream', '%d', '%s', '%d', '%d');\n",
								DISCUZ_DATABASE_PRE, tid%10, aid, pid, tid, attachmentstructlist[i].width, attachmentstructlist[i].dateline, attachmentstructlist[i].filename,
								attachmentstructlist[i].filesize, attachmentstructlist[i].attachment, attachmentstructlist[i].isimage, authorid);*/
						if(code_convert("gbk","utf8",attachmentstructlist[i].filename, strlen(attachmentstructlist[i].filename),attachfilenameutf8, 256 )==-1)  // 24 is the length of title
						{
							printf("convert board cname error \n");
							return -1;
						}

						sprintf(sqlbuf, "insert into %sforum_attachment_%d (aid, pid, tid, width, dateline, filename, filesize, attachment, isimage, uid) values ('%d', '%d', '%d', '%d', '%ld', '%s', '%d', '%s', '%d', '%d');",
								DISCUZ_DATABASE_PRE, tid%10, aid, pid, tid, attachmentstructlist[i].width, attachmentstructlist[i].dateline, attachfilenameutf8,
								attachmentstructlist[i].filesize, attachmentstructlist[i].attachment, attachmentstructlist[i].isimage, authorid);
						mysql_query(mysql, sqlbuf);
					}
					else
					{
						break;
					}
				}
				sprintf(sqlbuf, "update %sforum_post set attachment = '%d' where fid = '%d'; ",
						DISCUZ_DATABASE_PRE, isimage, fid);
				mysql_query(mysql, sqlbuf);
				aid = 0;
				article_transferred++;
				// 处理文章本身的各种状态
				// m状态处理
				if(filefh.accessed & FH_MARKED)
				{
					sprintf(sqlbuf, "update %sforum_thread set digest = '1' where tid = '%d'; ",
							DISCUZ_DATABASE_PRE, tid);
					mysql_query(mysql, sqlbuf);
					// 精华加分，修改common_member_count
					sprintf(sqlbuf, "select extcredits1, extcredits2, extcredits3, digestposts from %scommon_member_count where uid = '%d'; ", DISCUZ_DATABASE_PRE, authorid);
					mysql_query(mysql, sqlbuf);
					res = mysql_store_result(mysql);

					if (mysql_num_rows(res)!=0)
					{
						row = mysql_fetch_row(res);
						sprintf(sqlbuf, "update %scommon_member_count set extcredits1='%d', extcredits2 = '%d', extcredits3='%d', digestposts='%d' where uid = '%d'; ",
								DISCUZ_DATABASE_PRE, atoi(row[0])+5, atoi(row[1])+100,atoi(row[2])+3,atoi(row[3])+1,authorid);
						mysql_query(mysql, sqlbuf);
					}
				}
			}
			else
			{
				printf("toooold\n");
			}
		}
		else
		{
			printf("can not open %s\n", currdirect);
			article_transferred++;
		}
		id++;
	}
	fp = fopen(transferrecord, "w");
	sprintf(lasttransfernum, "last transfer article num : %d", id);
	fputs(lasttransfernum, fp);
	fclose(fp);
	return 0;
}

int
main(int argc, char *argv[])
{
	struct tm *starttime;
	time_t formattime;
	int startdateint, trans_article_num;
	int count;
	if(argc < 2)
	{
		printf("Usage: transythttodiscuzx <function_name>\n");
		printf(" -common_member: transfer the new id in .PASSWD_discuz to the database of discuz\n");
		printf(" -board_name [boardname] -start_date [yymmdd] -step [article_num]: \n");
		printf("    transfer selected board to the database of discuz from start date. \n");
		printf("    article_num indicates number of articles from last transfer. \n");
		return -1;
	}
	else
	{
		if(!strcmp(argv[1], "-common_member"))
		{
			transferuser();
		}
		else if(!strcmp(argv[1], "-board_name"))
		{
			count = 3;
			formattime = 0;
			trans_article_num = 1;
			while (count< argc)
			{
				if(!strcmp(argv[count], "-start_date"))
				{
					count++;
					starttime = (struct tm*)malloc(sizeof(struct tm));
					startdateint = atoi(argv[count]);
					starttime->tm_mday = startdateint % 100;
					starttime->tm_mon = (startdateint % 10000 - starttime->tm_mday) /100 -1;
					starttime->tm_year =(startdateint - starttime->tm_mon * 100 - starttime->tm_mday)/10000 + 100;
					// tm_year is the number from 1900 to input year
					formattime = mktime(starttime);
					free(starttime);
				}
				if(!strcmp(argv[count], "-step"))
				{
					count++;
					trans_article_num = atoi(argv[count]);
				}
				count++;
			}
//			printf("%ld %d\n", formattime,(int)formattime);
			transferboard(argv[2], (int)formattime, trans_article_num);
		}
		else
		{
			printf("Usage: transythttodiscuzx <function_name>\n");
			printf(" -common_member: transfer the new id in .PASSWD_discuz to the database of discuz\n");
			printf(" -board_name [boardname] : map the selected board to the database of discuzx\n");
			printf(" -board_name [boardname] -start_date [yymmdd] : transfer selected board to the database of discuzx\n");
			return -1;
		}
		return 0;
	}
}
