#include <stdio.h>
#include <ctype.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#include "ythtbbs.h"
#include "discuzsql.h"
#include "discuzmodule.h"
#include <iconv.h>    // convert gb2312 to utf8
#include "bbs.h"

static int isoverride(struct override *o, char *id);

char *
sethomepath(char *buf, const char *userid)
{
	sprintf(buf, MY_BBS_HOME "/home/%c/%s", mytoupper(userid[0]), userid);
	return buf;
}

char *
sethomefile(char *buf, const char *userid, const char *filename)
{
	sprintf(buf, MY_BBS_HOME "/home/%c/%s/%s", mytoupper(userid[0]), userid,
		filename);
	return buf;
}

char *
setmailfile(char *buf, const char *userid, const char *filename)
{
	sprintf(buf, MY_BBS_HOME "/mail/%c/%s/%s", mytoupper(userid[0]), userid,
		filename);
	return buf;
}

int
saveuservalue(char *userid, char *key, char *value)
{
	char path[256];
	sethomefile(path, userid, "values");
	return savestrvalue(path, key, value);
}

int
readuservalue(char *userid, char *key, char *value, int size)
{
	char path[256];
	sethomefile(path, userid, "values");
	return readstrvalue(path, key, value, size);
}

#if 0
char *
cuserexp(int exp)
{
	int expbase = 0;

	if (exp == -9999)
		return "没等级";
	if (exp <= 100 + expbase)
		return "新手上路";
	if (exp <= 450 + expbase)
		return "一般站友";
	if (exp <= 850 + expbase)
		return "中级站友";
	if (exp <= 1500 + expbase)
		return "高级站友";
	if (exp <= 2500 + expbase)
		return "老站友";
	if (exp <= 3000 + expbase)
		return "长老级";
	if (exp <= 5000 + expbase)
		return "本站元老";
	return "开国大老";
}
#else
#define GROUP_NUM 24
#define	LONG_GROUP_NUM 1
char exp_des[GROUP_NUM][12][16] =
{ {"没等级",	"一见如故",	"双喜临门",	"三羊开泰",	"四季平安",	"五福临门",	"六六大顺",	"七星高照",	"八方来财",	"九九同心",	"十全十美",	"一塌糊涂"},
  {"身无分文", 	"市井混混",	"千王跟班",	"赌徒", 	"赌棍", 	"赌鬼", 	"千王",		"赌霸",		"赌侠", 	"赌王", 	"赌圣", 	"赌神"},
  {"初学乍练",	"初窥门径",	"略知一二",	"半生不熟",	"已有小成",	"了然于胸",	"神乎其技",	"出神入化",	"登峰造极",	"举世无双",	"震古铄今",	"深不可测"},
  {"罗汉拳",	"般若掌", 	"一指禅", 	"金钟罩", 	"一阳指",  	"火焰刀",	"化骨绵掌",	"降龙十八掌", 	"九阳神功", 	"六脉神剑",	"天魔解体", 	"野球拳"},
  {"画饼充饥", 	"食不果腹",	"咸菜稀饭", 	"醋溜土豆", 	"番茄鸡蛋", 	"鱼香肉丝", 	"酱爆排骨", 	"清蒸鲫鱼", 	"燕窝鱼翅", 	"山珍海味", 	"满汉全席",	"青菜豆腐"},
  {"五音不全",	"呀呀学语", 	"淳朴民歌", 	"清音雅奏", 	"渔歌互答", 	"霓裳六幺", 	"小楼吹彻", 	"曲高和寡", 	"广陵散", 	"笑傲江湖", 	"天籁之音", 	"此处无声"},
  {"石器时代",	"镰刀斧头",	"小米步枪",	"迫击炮",	"铁甲坦克",	"轰炸机",	"巡航导弹",	"核潜艇",	"航空母舰",	"原子弹",	"星球大战",	"赤手搏龙"},
  {"晴空万里",	"蓝天白云",	"风雨欲来",	"阴云密布",	"毛毛细雨",	"风雨交加",	"倾盆大雨",	"狂风骤雨",	"热带风暴",	"洪水滔天",	"世界末日",	"真水无痕"},
  {"初来乍到",	"走马观花",	"崭露头角",	"一鸣惊人",	"身经百战",	"无敌战士",	"长胜将军",	"百战金刚",	"九天战神",	"独孤求败",	"金刚不坏",	"木乃伊"},
  {"雏鸡破壳",	"初生牛犊",	"草原孤狼",	"豹跳如雷",	"虎啸山林",	"金毛狮王",	"凤舞九天",	"龙行天下",	"恐龙化石",	"始祖鸟",	"三叶虫",	"单细胞生物"},
  {"一介白衣",	"小妖",		"至尊宝",	"紫霞仙子",	"牛魔王",	"沙僧",		"八戒",		"孙猴子",	"唐僧",		"观音菩萨",	"如来佛祖",	"斗战胜佛"},
  {"指腹为婚",	"青梅竹马",	"两小无猜",	"花前月下",	"卿卿我我",	"海誓山盟",	"洞房花烛",	"新婚燕尔",	"相敬如宾",	"举案齐眉",	"白头偕老",	"化蝶双飞"},
  {"苦海无涯",	"无人问津",	"孤苦伶丁",	"茕茕孑立",	"形单影只",	"孤枕难眠",	"与物寡情",	"古井不波",	"孤独终老",	"黯然销魂",	"天煞孤星",	"火星来客"},
  {"列兵",	"少尉",		"中尉",		"上尉",		"少校",		"中校",		"上校",		"大校",		"少将",		"中将",		"上将",		"军委主席"},
  {"无明",	"行",		"识",		"名色",		"六入",		"触",		"受",		"爱",		"取",		"有",		"生",		"老死"},
  {"兜率降世",	"入住母胎",	"圆满诞生",	"善娴技艺",	"受用妃眷",	"从家出家",	"行苦难行",	"趋金刚座",	"调伏魔军",	"成正等觉",	"转妙法轮",	"入大涅槃"},
  {"小宝宝",	"幼儿园",	"学前班",	"小学生",	"初中生",	"高中生",	"大学生",	"硕士生",	"博士生",	"博士后",	"教授",		"院士"},
  {"没等级", 	"新手上路", 	"一般站友", 	"中级站友", 	"高级站友", 	"老站友", 	"长老级", 	"本站元老", 	"开国大老",	"开国大老",	"开国大老",	"开国大老"},
  {"不会玩",	"一心敬",	"哥俩好",	"三桃园",	"四季发",	"五魁手",	"六六顺",	"七匹马",	"八仙到",	"酒在手",	"十满堂",	"干一杯"},
  {"小女生",	"孟姜女",	"李清照",	"黄道婆",	"林黛玉",	"杨贵妃",	"王昭君",	"貂蝉",		"西施",		"花木兰",	"穆桂英",	"如花"},
  {"", "","","","","","","","","","",""},
#if 1
  {"骰子",	"饼子",		"条子",		"万子",		"东风",		"南风",		"西风",		"北风",		"红中",		"发财",		"白板",		"雀神"},
  {"龙蛋",	"初生龙",	"幼龙",		"鸭嘴龙",	"双棘龙",	"迅龙",		"雷龙",		"剑龙",		"霸王龙",	"黄金龙",	"中华圣龙",	"如故龙"},
#endif
  {"侏罗纪",    "人",           "天使",         "大天使",       "权天使",       "能天使",       "力天使",       "主天使",	"座天使",       "智天使",       "炽天使",       "神"}
};
#if 1	//5555555555555:(
char long_exp_des[LONG_GROUP_NUM][101][16] =
{{"布衣", 	"归德执戟长上", "将仕郎" , 	"陪戎副尉", 	"文林郎", 	"陪戎校尉", 	"登仕郎", 	"怀化执戟长上", "仁勇副尉", 	"仁勇校尉",
 "儒林郎" , 	"承务郎", 	"归德司戈",	"御侮副尉", 	"承奉郎", 	"御侮校尉", 	"征事郎", 	"怀化司戈", 	"宣节副尉", 	"给事郎", 
 "宣节校尉", 	"宣议郎", 	"归德中侯", 	"翊麾副尉", 	"朝散郎", 	"翊麾校尉", 	"武骑尉", 	"宣德郎", 	"怀化中侯", 	"致果副尉", 
 "朝请郎", 	"致果校尉", 	"云骑尉", 	"通直郎", 	"归德司阶", 	"振威副尉", 	"奉议郎", 	"振威校尉", 	"飞骑尉", 	"承议郎", 
 "怀化司阶", 	"昭武副尉", 	"朝议郎", 	"昭武校尉", 	"骁骑尉", 	"朝散大夫", 	"归德郎将", 	"游击将军", 	"朝请大夫", 	"游骑将军", 
 "骑都尉", 	"朝议大夫", 	"怀化郎将", 	"宁远将军", 	"中散大夫", 	"定远将军", 	"上骑都尉", 	"中大夫", 	"归德中郎将", 	"明威将军", 
 "太中大夫", 	"宣威将军", 	"轻车都尉", 	"通议大夫", 	"怀化中郎将", 	"壮武将军", 	"正议大夫", 	"忠武将军", 	"上轻车都尉", 	"银青光禄大夫",
 "归德大将军", "云麾将军", 	"护军", 	"归德将军", 	"怀化将军", 	"上护军", 	"金紫光禄大夫", "怀化大将军", 	"冠军大将军", 	"柱国", 
 "光禄大夫", 	"镇军大将军", 	"特进", 	"上柱国", 	"辅国大将军", 	"开府仪同三司", "膘骑大将军", 	"开国县男", 	"开国县子", 	"开国县伯",
 "开国县侯", 	"开国县公", 	"开国郡公", 	"国公", 	"郡王", 	"辅佐王", 	"封疆王", 	"佐政王", 	"亲王", 	"皇帝"}
 };
#endif

char exp_buf[20];
	
char *
cuserexp( int group, int exp)
{
	int lvl, expbase = 0;
	
	if (exp < 0)
		return	(exp_des[0][0]);
	for(lvl=0; exp > 5*lvl*lvl+15*lvl + expbase; lvl++ );

	if(lvl>100)
		lvl = 100;
	

	if(group == 'U')
		return "隐藏";
	else if(group>='A' && group<('A'+GROUP_NUM))
		sprintf(exp_buf,"%d级 %s",lvl, exp_des[group-'A'][lvl/10 + 1] );
#if 1	//555555555555:(
	else if(group == '1')
		return( long_exp_des[group-'1'][lvl-1] );
#endif
	else
		sprintf(exp_buf,"%d级 %s", lvl, exp_des[0][lvl/10 + 1]);
	
	return	exp_buf;
}

#endif

char *
cperf(int perf)
{
	if (perf == -9999)
		return "没等级";
	if (perf <= 5)
		return "赶快加油";
	if (perf <= 12)
		return "努力中";
	if (perf <= 35)
		return "还不错";
	if (perf <= 50)
		return "很好";
	if (perf <= 90)
		return "优等生";
	if (perf <= 140)
		return "太优秀了";
	if (perf <= 200)
		return "本站支柱";
	if (perf <= 500)
		return "神～～";
	return "机器人！";
}
#if 0
int
countexp(struct userec *urec)
{
	int exp;

	if (!strcmp(urec->userid, "guest"))
		return -9999;
	if(urec->exp_group == 'U')
		return 0;
	exp =
	    urec->numposts/5 + urec->numlogins + (time(0) -
						    urec->firstlogin) / 86400 +
	    urec->stay / 1800 + urec->extraexp1 * 20;
	return exp > 0 ? exp : 0;
}
#else
int
countexp(struct userec *urec, int level)
{      //level 0:传统经验值 1:精华值 2:合计
       int exp1 = 0, exp2 = 0;
       struct userdata currentdata;

       if (!strcmp(urec->userid, "guest"))
               return -9999;
       if(level == 0 || level == 2)    //传统经验值
               exp1 = urec->numposts/5 + urec->numlogins + (time(0) -
                       urec->firstlogin) / 86400 + urec->stay / 1800;
       if(level == 1 || level == 2){   //精华值
               loaduserdata(urec->userid, &currentdata );
               if(currentdata.extraexp > 100000000){	//这里检查一下，一般没有bt超过1y吧
               		currentdata.extraexp = 0;
               		saveuserdata(urec->userid, &currentdata );
               	}
               exp2 = urec->extraexp1 * 20 + currentdata.extraexp;
       }

       return (exp1+exp2) > 0 ? (exp1+exp2) : 0;
}
#endif

int
countperf(struct userec *urec)
{
	int perf;
	int reg_days;

	if (!strcmp(urec->userid, "guest"))
		return -9999;
	reg_days = (time(0) - urec->firstlogin) / 86400 + 1;
	perf =
	    ((float) (urec->numposts) / (float) urec->numlogins +
	     (float) urec->numlogins / (float) reg_days) * 10;
	return perf > 0 ? perf : 0;
}

int
countlife(struct userec *urec)
{
	int value;

	/* if (urec) has XEMPT permission, don't kick it */
	if ((urec->userlevel & PERM_XEMPT)
	    || strcmp(urec->userid, "guest") == 0)
		return 999;
	value = (time(0) - urec->lastlogin) / 60;	/* min */
#if 0
	if (urec->numlogins <= 3)
		return (15 * 1440 - value) / 1440;
#endif
	if (!(urec->userlevel & PERM_LOGINOK))
		return (30 * 1440 - value) / 1440;
	return (120 * 1440 - value) / 1440 + min(urec->numdays,
						 (unsigned short) 800);
}

int
userlock(char *userid, int locktype)
{
	char path[256];
	int fd;
	sethomefile(path, userid, ".lock");
	fd = open(path, O_RDONLY | O_CREAT, 0660);
	if (fd == -1)
		return -1;
	flock(fd, locktype);
	return fd;
}

int
userunlock(char *userid, int fd)
{
	flock(fd, LOCK_UN);
	close(fd);
	return 0;
}

static int
checkbansitefile(const char *addr, const char *filename)
{
	FILE *fp;
	char temp[STRLEN];
	if ((fp = fopen(filename, "r")) == NULL)
		return 0;
	while (fgets(temp, STRLEN, fp) != NULL) {
		strtok(temp, " \n");
		if ((!strncmp(addr, temp, 16))
		    || (!strncmp(temp, addr, strlen(temp))
			&& temp[strlen(temp) - 1] == '.')
		    || (temp[0] == '.' && strstr(addr, temp) != NULL)) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

int
checkbansite(const char *addr)
{
	return checkbansitefile(addr, MY_BBS_HOME "/.bansite")
	    || checkbansitefile(addr, MY_BBS_HOME "/bbstmpfs/dynamic/bansite");
}

int
userbansite(const char *userid, const char *fromhost)
{
	char path[STRLEN];
	FILE *fp;
	char buf[STRLEN];
	int i, deny;
	char addr[STRLEN], mask[STRLEN], allow[STRLEN];
	char *tmp[3] = { addr, mask, allow };
	unsigned int banaddr, banmask;
	unsigned int from;
	from = inet_addr(fromhost);
	sethomefile(path, userid, "bansite");
	if ((fp = fopen(path, "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		i = mystrtok(buf, ' ', tmp, 3);
		if (i == 1) {	//单独 ip
			banaddr = inet_addr(addr);
			banmask = inet_addr("255.255.255.255");
			deny = 1;
		} else if (i == 2) {
			banaddr = inet_addr(addr);
			banmask = inet_addr(mask);
			deny = 1;
		} else if (i == 3) {	//带 allow 项
			banaddr = inet_addr(addr);
			banmask = inet_addr(mask);
			deny = !strcmp(allow, "allow");
		} else		//空行？
			continue;
		if ((from & banmask) == (banaddr & banmask)) {
			fclose(fp);
			return deny;
		}
	}
	fclose(fp);
	return 0;
}

void
logattempt(const char *user, char *from, char *zone, time_t time)
{
	char buf[256], filename[80];
	int fd, len;

	sprintf(buf, "system passerr %s", from);
	newtrace(buf);
	snprintf(buf, 256, "%-12.12s  %-30s %-16s %-6s\n",
		 user, Ctime(time), from, zone);
	len = strlen(buf);
	if ((fd =
	     open(MY_BBS_HOME "/" BADLOGINFILE, O_WRONLY | O_CREAT | O_APPEND,
		  0644)) >= 0) {
		write(fd, buf, len);
		close(fd);
	}
	sethomefile(filename, user, BADLOGINFILE);
	if ((fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644)) >= 0) {
		write(fd, buf, len);
		close(fd);
	}
}

static int
isoverride(struct override *o, char *id)
{
	if (strcasecmp(o->id, id) == 0)
		return 1;
	return 0;
}

int
inoverride(char *who, char *owner, char *file)
{
	char buf[80];
	struct override o;
	sethomefile(buf, owner, file);
	if (search_record(buf, &o, sizeof (o), (void *) isoverride, who) != 0)
		return 1;
	return 0;
}

int
saveuserdata(char *uid, struct userdata *udata)
{
	char newfname[80];
	char fname[80];
	int fd;
	sethomefile(newfname, uid, "newuserdata");
	sethomefile(fname, uid, "userdata");
	fd = open(newfname, O_WRONLY | O_CREAT, 0660);
	if (fd == -1) {
		errlog("open userdata error %s", uid);
		return -1;
	}
	write(fd, udata, sizeof (struct userdata));
	close(fd);
	rename(newfname, fname);
	return 0;
}

int
loaduserdata(char *uid, struct userdata *udata)
{
	char buf[80];
	int fd;
	sethomefile(buf, uid, "userdata");
	bzero(udata, sizeof (struct userdata));
	fd = open(buf, O_RDONLY);
	if (fd == -1) {
		return -1;
	}
	read(fd, udata, sizeof (struct userdata));
	close(fd);
	return 0;
}

int
lock_passwd()
{
	int lockfd;

	lockfd = open(MY_BBS_HOME "/.PASSWDS.lock", O_RDWR | O_CREAT, 0600);
	if (lockfd < 0) {
		errlog("uhash lock err: %s", strerror(errno));
		exit(1);
	}
	flock(lockfd, LOCK_EX);
	return lockfd;
}

int
unlock_passwd(int fd)
{
	flock(fd, LOCK_UN);
	close(fd);
	return 0;
}

/*返回插入的passwd位置，第一个编号为 1
  id已经被注册，返回 0
  插入失败，返回 -1
*/
int
insertuserec(const struct userec *x)
{
	int lockfd, fd;
	int i;
	lockfd = lock_passwd();
	if (finduseridhash(x->userid) > 0) {
		unlock_passwd(lockfd);
		return 0;
	}
	i = deluseridhash("");
	if (i <= 0) {
		unlock_passwd(lockfd);
		return -1;
	}
	fd = open(".PASSWDS", O_WRONLY);
	if (fd == -1) {
		insertuseridhash("", i, 1);
		unlock_passwd(lockfd);
		return -1;
	}
	if (lseek(fd, (i - 1) * sizeof (struct userec), SEEK_SET) == -1) {
		insertuseridhash("", i, 1);
		close(fd);
		unlock_passwd(lockfd);
		return -1;
	}
	if (write(fd, x, sizeof (struct userec)) != sizeof (struct userec)) {
		close(fd);
		unlock_passwd(lockfd);
		errlog("can't write!");
		return -1;
	};
	close(fd);
	insertuseridhash(x->userid, i, 1);
	unlock_passwd(lockfd);
	return i;
}

//根据userid，填充urec指针到，指向passwdptr中相应条目，该结构只读
//返回对应偏移量，第一个编号为 1
//如果用户已经kickout，那也找不到，找不到返回 0
int
getuser(const char *userid, struct userec **urec)
{
	int i;
	if (userid[0] == 0 || strchr(userid, '.')) {
		if (urec != NULL)
			*urec = NULL;
		return 0;
	}
	i = finduseridhash(userid);
	if (i > 0 && i <= MAXUSERS
	    && !strcasecmp(passwdptr[i - 1].userid, userid)
	    && !(passwdptr[i - 1].kickout)) {
		if (urec != NULL) {
			*urec = (struct userec *) (passwdptr + (i - 1));
		}
		return i;
	}
	return 0;
}

//根据userid，访问数据库，如果需要，则使用checkdiscuzpasswd()匹配密码
//如果没有，或用户已经kickout，那也找不到，找不到返回 0
//如果有，参考register函数，写入.PASSWDS
int
checkdiscuzuser(const char *userid)
{
	char sqlbuf[512];
	char useridutf8[22];
	MYSQL *mysql = NULL;
	MYSQL_RES *res;

	mysql = mysql_init(mysql);
    mysql = mysql_real_connect(mysql,"localhost",SQLUSER, SQLPASSWD, SQLDB,0, NULL,0);
	if (!mysql) {
		perror("Can not open database\n");
		return -1;
	}
	if(code_convert("gbk","utf8",userid, strlen(userid),useridutf8, 22 )==-1)   // 21 = (IDLEN+2)/2*3   longest utf-8
	{
		printf("convert username error \n");
		mysql_close(mysql);
		return -1;
	}
	mysql_query(mysql, "set names utf8");
	sprintf(sqlbuf,"select username from pre_ucenter_members where username = \'%s\'; " , useridutf8 );
	mysql_query(mysql, sqlbuf);
	res = mysql_store_result(mysql);

	if (mysql_num_rows(res)!=0)
	{
		// userid already in discuz
		mysql_free_result(res);
		mysql_close(mysql);
		return 1;
	}
	else
	{
		mysql_free_result(res);
		mysql_close(mysql);
		return 0;
	}
}

int
checkdiscuzpasswd(char *userid, const char *passbuf)
{
	// userid will update because of Capital letter issues
	char discuzuseridtrans[22];

	char sqlbuf[512];
	char useridutf8[22];
	char discuzpassmd5[DISCUZ_PASSWD_LENGTH + 1];
	MYSQL *mysql = NULL;
	MYSQL_RES *res;
	MYSQL_ROW row;
	if (userid[0] == 0 || strchr(userid, '.')) {
		return 0;
	}

	mysql = mysql_init(mysql);
    mysql = mysql_real_connect(mysql,"localhost",SQLUSER, SQLPASSWD, SQLDB,0, NULL,0);
	if (!mysql) {
		perror("Can not open database\n");
		return -1;
	}
	if(code_convert("gbk","utf8",userid, strlen(userid),useridutf8, 22 )==-1)   // 21 = (IDLEN+2)/2*3   longest utf-8
	{
		printf("convert username error \n");
		mysql_close(mysql);
		return -1;
	}
	mysql_query(mysql, "set names utf8");
	sprintf(sqlbuf,"select password,salt,username from pre_ucenter_members where username = \'%s\'; " , useridutf8 );
	mysql_query(mysql, sqlbuf);
	res = mysql_store_result(mysql);

	if (mysql_num_rows(res)!=0)
	{
		// userid already in discuz
		row = mysql_fetch_row(res);  // row[0] is password , row[1] is salt, row[2] is username in discuz
		gendiscuzpasswd(discuzpassmd5, row[1], passbuf);
/*		// for log
		char logfile[256], errorlog[256];
			sprintf(logfile, "/home/bbs/reclog/telnet.log");
			sprintf(errorlog, "discuzpass=%s  genpass=%s  originalpass=%s\n",
					row[1], discuzpassmd5, passbuf);
			writelog(logfile, errorlog);
		// logend
*/
		// update user name to make sure it's the same (not capitalized) as the one in discuz
		if(code_convert("utf8","gbk",row[2],strlen(row[2]),discuzuseridtrans, 16 )==-1)   // its userid should be less than 16
		{
			printf("convert username error \n");
			mysql_close(mysql);
			return -1;
		}
		sprintf(userid, "%s", discuzuseridtrans);
		return !memcmp(row[0], discuzpassmd5, DISCUZ_PASSWD_LENGTH);
	}
	else
		mysql_free_result(res);
		mysql_close(mysql);
		return 0;
}



//根据uid，填充urec指针到，指向passwdptr中相应条目，该结构只读
//返回对应偏移量，第一个编号为 1
//如果用户已经kickout，那也找不到，找不到返回 0
int
getuserbynum(const int uid, struct userec **urec)
{
	if (uid > 0 && uid <= MAXUSERS && !(passwdptr[uid - 1].kickout)) {
		if (urec != NULL) {
			*urec = (struct userec *) (passwdptr + (uid - 1));
		}
		return uid;
	}
	return 0;
}
char *
getuseridbynum(const int uid) {
	struct userec *urec;
	if (getuserbynum(uid, &urec) <= 0)
		return 0;
	else
		return urec->userid;
}

int login_get_user(const char *user, struct userec **urec)
{
	if(isdigit(*user))
		return getuserbynum(atoi(user), urec);
	return getuser(user, urec);
}

//找到userid按照passwd的位置, 第一个位置编号为 1
//若kickout则返回负值
//用于新用户注册时预检是否可以注册
//找不到返回 0 
int
user_registered(const char *userid)
{
	int i;
	if (userid[0] == 0 || strchr(userid, '.')) {
		return 0;
	}
	i = finduseridhash(userid);
	if (i > 0 && i <= MAXUSERS
	    && !strcasecmp(passwdptr[i - 1].userid, userid)) {
		if (passwdptr[i - 1].kickout)
			return -i;
		else
			return i;
	}
	return 0;
}

//kickout不从hash中删除用户，用户flags[0] KICKOUT_FLAG位 置1
//dietime置为当前时间
//不修改hash
//24小时后，调用deluserec从passwd中删除
int
kickoutuserec(const char *userid)
{
	int i;
	struct userec tmpuser;
	time_t now_t;
	int fd;
	char buf[1024];
	i = finduseridhash(userid);
	if (i <= 0 || i > MAXUSERS) {	//查找hash失败
		return -1;
	}
	if (passwdptr[i - 1].kickout)	//已经被kick了
		return 0;
	now_t = time(NULL);
	memcpy(&tmpuser, &(passwdptr[i - 1]), sizeof (struct userec));
	tmpuser.kickout = now_t;
	fd = open(".PASSWDS", O_WRONLY);
	if (fd == -1) {
		return -1;
	}
	if (lseek(fd, (i - 1) * sizeof (struct userec), SEEK_SET) == -1) {
		close(fd);
		return -1;
	}
	sprintf(buf, "mail/%c/%s", mytoupper(userid[0]), userid);
	//prints("%s",buf);
	deltree(buf);
	sprintf(buf, "home/%c/%s", mytoupper(userid[0]), userid);
	deltree(buf);
	write(fd, &tmpuser, sizeof (struct userec));
	close(fd);
	return 0;
}

//从passwd和hash中删除条目
//成功返回 0，失败返回 -1
int
deluserec(const char *userid)
{
	int i;
	time_t now_t;
	struct userec tmpuser;
	int fd, lockfd;
	lockfd = lock_passwd();
	i = finduseridhash(userid);
	if (i <= 0 || i > MAXUSERS) {
		errlog("del user error %d", i);
		unlock_passwd(lockfd);
		return 0;
	}
	now_t = time(NULL);
	if (!(passwdptr[i - 1].kickout)
	    || (now_t - passwdptr[i - 1].kickout <= 86400 - 3600)) {
		errlog("del user error 2 %d", (int) passwdptr[i - 1].kickout);
		unlock_passwd(lockfd);
		return 0;
	}
	if (deluseridhash(userid) <= 0) {
		errlog("del user error 3 %s", userid);
		unlock_passwd(lockfd);
		return -1;
	}
	fd = open(".PASSWDS", O_WRONLY);
	if (fd == -1) {
		insertuseridhash(userid, i, 1);
		errlog("del user error open");
		unlock_passwd(lockfd);
		return -1;
	}
	if (lseek(fd, (i - 1) * sizeof (struct userec), SEEK_SET) == -1) {
		insertuseridhash(userid, i, 1);
		errlog("del user error seek");
		close(fd);
		unlock_passwd(lockfd);
		return -1;
	}
	bzero(&tmpuser, sizeof (struct userec));
	write(fd, &tmpuser, sizeof (struct userec));
	close(fd);
	insertuseridhash("", i, 1);
	unlock_passwd(lockfd);
	return 0;
}

/* 修改 x->userid 对应条目为 x
 * 即修改不修改 id
 * 也不修改 hash
 * 如果 usernum 为 0，从hash中查找，否则直接检查对应位置是否是该用户
 * usernum从 1 开始计数
 */
int
updateuserec(const struct userec *x, const int usernum)
{
	int i = usernum, fd;
	if (i == 0)
		i = finduseridhash(x->userid);
	if (i <= 0 || i > MAXUSERS
	    || strcasecmp(passwdptr[i - 1].userid, x->userid)) {
		errlog("update user error. %s %s", passwdptr[i - 1].userid,
		       x->userid);
		return -1;
	}
	if (passwdptr[i - 1].kickout) {
		return -1;
	}
	fd = open(".PASSWDS", O_WRONLY);
	if (fd == -1) {
		return -1;
	}
	if (lseek(fd, (i - 1) * sizeof (struct userec), SEEK_SET) == -1) {
		close(fd);
		return -1;
	}
	write(fd, x, sizeof (struct userec) - sizeof (time_t));
	//FIX ME, maybe it's a bug for 64-bit mode
	close(fd);
	return 0;
}

int
apply_passwd(int (*fptr) (const struct userec *, char *), char *arg)
{
	int i;
	int ret;
	int count;
	count = 0;
	for (i = 1; i <= MAXUSERS; i++) {
		ret = (*fptr) (&(passwdptr[i - 1]), arg);
		if (ret == -1)
			break;
		if (ret == 0)
			count++;
	}
	return count;
}

int
has_fill_form(char *userid)
{
	DIR *dirp;
	struct dirent *direntp;
	FILE *fp;
	int i = 1;
	char tmp[256], buf[256], *ptr;
	dirp = opendir(SCANREGDIR);
	if (dirp == NULL)
		return 0;
	while (i) {
		if ((direntp = readdir(dirp)) != NULL)
			snprintf(tmp, 256, "%s%s", SCANREGDIR, direntp->d_name);
		else {
			snprintf(tmp, 256, "%s/new_register", MY_BBS_HOME);
			i = 0;
		}
		if ((fp = fopen(tmp, "r")) != NULL) {
			while (1) {
				if (fgets(buf, 256, fp) == 0)
					break;
				if ((ptr = strchr(buf, '\n')) != NULL)
					*ptr = '\0';
				if (strncmp(buf, "userid: ", 8) == 0 &&
				    strcmp(buf + 8, userid) == 0) {
					fclose(fp);
					closedir(dirp);
					return 1;
				}
			}
			fclose(fp);
		}
	}
	closedir(dirp);
	return 0;
}
