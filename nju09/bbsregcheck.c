#include "bbslib.h"
#define isletter(a)	(a>= 'A' && a <= 'Z')?1:((a>= 'a' && a <= 'z')?1:0)

int
bbsregcheck_main()
{
	char file[STRLEN];
	html_header(1);
	printf("<body>");
  char userid[IDLEN+2];
  int r;
  //char email[42],email2[42];
  strsncpy(userid, getparm("userid"), sizeof (userid));
  //strsncpy(email, getparm("email"), sizeof (email));
  //strsncpy(email2, getparm("email2"), sizeof (email2));

	if (strlen(userid) < 2 || (strlen(userid) == 2 && !(isletter(userid[0]) && isletter(userid[1])) ) ) {
  		printf("<li>抱歉，帐号长度太短或帐号中英文字母不足两个(2-12个中英文字符),无法注册，请重新选择。");
		  return 0;
		}
	if (strlen(userid)>12) {
	   printf("<li>抱歉，帐号长度太长(2-12个中英文字符)，无法注册，请重新选择。");
	   return 0;
	}
	if (is_bad_id(userid)) {
    printf("<li>抱歉，%s为系统保留帐号，无法注册，请另选一个",userid);
    return 0;
  }
	if (!goodgbid(userid)) {
		printf("<li>抱歉，帐号只能由英文字母和标准汉字组成，请重新选择");
		return 0;
	}
	r=user_registered(userid);
	if (r==0) {
      printf("<li>恭喜，%s未被注册，您可以使用该帐号",userid);
      return 0;
  }
	if (r > 0) {
			printf("<li>%s已经有人使用,请重新选择。",userid);
			return 0;
	}
	if (r < 0) {
			printf("%s刚刚死亡待清理，请明天再来，或重新选择id",userid);
			return 0;
	}
	errlog("www reg error: %s", userid);
	http_fatal("内部错误");
//printf("%s,%s,%s",userid,email,email2);
	showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
  printf("</body></html>");
  return 0;
}
