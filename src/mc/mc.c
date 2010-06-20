#include <math.h>
#include "mc.h"

MC_Env *mcEnv;
mcUserInfo *myInfo;
McMap *maps;

struct command2func {
        char name;
        int (*fptr) ();
};


const struct command2func mc_c2f[] = {
	{'s', (void*)money_hall}, 
	{'1', (void*)money_bank},
	{'2', (void*)money_gamble},
	{'3', (void*)money_shop},
	{'4', (void*)money_stock},
	{'5', (void*)money_lottery},
	{'d', (void*)money_exp},
	{'6', (void*)money_robber},
	{'7', (void*)money_beggar},
	{'8', (void*)money_cop},
	{'9', (void*)money_task},
	{'\0', (void*)NULL}};


static void* exec_func(char func_name) {
	int i = 0;
	for(i=0; mc_c2f[i].name != '\0'; i++) {
		if (mc_c2f[i].name == _tolower(func_name))
			return (void*)(mc_c2f[i].fptr)();
	}
	return NULL;
}


int
moneycenter()
{
	int ch, retv;
	char filepath[256];
	char buf[256];
	modify_user_mode(MONEY);
	strcpy(currboard, "millionaire");	// for deliverreport() 
	moneycenter_welcome();
// 加载全局参数和我的数据 
	sprintf(filepath, "%s", DIR_MC "mc.env");
	if (!file_exist(filepath))
		initData(0, filepath);
	if((mcEnv = loadData(filepath, sizeof (MC_Env))) == (void*)-1)
		return -1;
	sethomefile(filepath, currentuser->userid, "mc.save");
	if (!file_exist(filepath))
		initData(1, filepath);
	if ((myInfo = loadData(filepath, sizeof (mcUserInfo))) == (void*)-1)
		goto MCENV;
// 检查是否可以进入 
	retv = check_allow_in();
	if (retv == 0)
		goto UNMAP;
	if (retv == -1)
		goto MUTEX;
//myInfo->aliveTime = time(NULL); //自动更新体力转移到从进站开始 main.c 
	if (newSalary()) {
                      sprintf(buf, "请于%d天内到%s银行领取，过期视为放弃。",
                                        PAYDAY, CENTER_NAME);
                      deliverreport("【银行】本站公务员领取工资",
                                              buf);
		}
	if(load_maps() == -1)
		goto UNMAP;

	while (1) {
		nomoney_show_stat("十字路口");
		move(6, 0);
		prints("\033[33;41m " MY_BBS_NAME " \033[m\n\n");
		prints
		    ("\033[1;33m        ▁▁▁ ▔█▔         ▁▁╱          ▁▁╱\n"
		     "\033[1;33m          █  █▔▔█  ▁▁  █ | █   ▁▁  █ | █    \033[31m  ▁▁▁▁▁\033[m\n"
		     "\033[1;33m          █  █ █ █  █ ● █ | █   █ ● █ | █    \033[31m▕大富翁世界▏\033[m\n"
		     "\033[1;33m        —█ ▁╱ ●    ▔▔▁◤ ●◥◣ ▔▔▁◤ ●◥◣  \033[31m  ▔▔▔▔▔\033[m");
		move(t_lines - 2, 0);
		prints
		    ("\033[1;44m 你要去 \033[1;46m [S]中央广场 [1]银行 [2]赌场 [3]商店 [4]股市 [5]彩票                 \033[m");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m        \033[1;46m [D]水晶之门 [6]黑帮 [7]丐帮 [8]警署 [9]工会 [Q]离开                 \033[m");
		ch = igetkey();
		if(ch == 'q' || ch == 'Q')
			break;
		exec_func(ch);
	} //end of menu loop
	if(random() % 10 == 0)
		check_top();
	//forcetax();
      UNMAP:
	myInfo->mutex = 0;
      MUTEX:
	unloadData(myInfo, sizeof (mcUserInfo));
      MCENV:
	unloadData(mcEnv, sizeof (MC_Env));
	moneycenter_byebye();
	limit_cpu();
	return 0;
}

/*   体力更新 add by koyu */
void
update_health()
{
	time_t curtime;
	int add_health, add_point;

	curtime = time(NULL);
	if (myInfo->con > 25)
		myInfo->con = 25;
	add_health = (curtime - myInfo->aliveTime) / (30 - myInfo->con);	//每(30-根骨)秒体力加1 
	add_point = curtime - myInfo->aliveTime;
	myInfo->WorkPoint += add_point;
	if (myInfo->WorkPoint > 30000)
		myInfo->WorkPoint = 30000;
	if (myInfo->health < 0)
		myInfo->health = 0;
	myInfo->health =
	    MIN((myInfo->health + add_health), 100 + myInfo->con * 2);
	myInfo->aliveTime += add_point;

//人品[-100, 100]
	if (myInfo->luck > 100)
		myInfo->luck = 100;
	if (myInfo->luck < -100)
		myInfo->luck = -100;
//胆识身法上限30000. add by yxk
	if (myInfo->robExp > 30000 || myInfo->robExp < -10000)
		myInfo->robExp = 30000;
	if (myInfo->begExp > 30000 || myInfo->begExp < -10000)
		myInfo->begExp = 30000;
//金钱上限20亿. add by yxk
	if (myInfo->cash > 2000000000) {
		mcEnv->Treasury += myInfo->cash - 2000000000;
		myInfo->cash = 2000000000;
	}
	if (myInfo->cash < -1000000000) {
		mcEnv->Treasury += 294967296 + myInfo->cash;
		mcEnv->Treasury += 2000000000;
		myInfo->cash = 2000000000;
	}
	if (myInfo->credit > 2000000000) {
		mcEnv->Treasury += myInfo->credit - 2000000000;
		myInfo->credit = 2000000000;
	}
	if (myInfo->credit < -1000000000) {
		mcEnv->Treasury += 294967296 + myInfo->credit;
		mcEnv->Treasury += 2000000000;
		myInfo->credit = 2000000000;
	}
	if (!(random() % (MAX_ONLINE * 25))
	    && (5000000 < myInfo->cash + myInfo->credit))
		forcetax();

	return;
}

/*  检查行动需要的体力，体力够返回0，不够显示消息返回1  add by koyu */
int
check_health(int need_health, int x, int y, char *str, int wait)
{
	if (myInfo->health >= need_health)
		return 0;
	else {
		showAt(x, y, str, wait);
		return 1;
	}
}

/* 帮派活动成功率与属性相关  add by darkeagle */
int
check_chance(int attack, int defence, int weapon, int armor, int effect,
	     int bonus)
{
	int num;
	double attacktemp;
	attacktemp = attack;
	num =
	    500 +
	    log((attacktemp + 30) / (defence + 30) * (weapon + 5) / (armor +
								     5)) *
	    effect + bonus;
	//攻击者属性超过防御者过多倍时，必然成功
	if (random() % 1000 < num)
		return 1;
	else
		return 0;
}

void
showAt(int line, int col, char *str, int wait)
{
	move(line, col);
	prints("%s", str);
	if (wait)
		pressanykey();
}

void
moneycenter_welcome()
{
	char buf[256];

	clear();
	move(5, 0);
	prints("                      \033[1;32m这是一个砖块横飞的世界！\n\n");

	prints
	    ("                      \033[1;32m这是一个梦与冒险的世界！\033[m\n\n");

	prints
	    ("                         \033[1;31m在这里，适者生存！\033[m\n\n");

	prints
	    ("                         \033[1;31m在这里，强者为王！\033[m\n\n");

	prints
	    ("              \033[1;35m【友情提醒】钱财为身外之物 胜败乃玩家常事\033[m\n\n\n");

	prints
	    ("                    \033[0;1;33m★%s 2005大富翁世界欢迎您★  \033[0m\n\n",
	     CENTER_NAME);

	prints
	    ("                    \033[0;1;33m大富翁游戏系统信件已经开启  \033[0m");
	pressanykey();

	clear();
	move(2, 0);
	sprintf(buf, "vote/millionaire/secnotes");
	if (dashf(buf))
		ansimore2stuff(buf, NA, 2, 24);
	pressanykey();
}

void
check_top()
{
	int n, n2, topMoney[LISTNUM], tmpM, ns, i, worth;
	short topRob[LISTNUM], topBeg[LISTNUM], tmpR, tmpB;
	char topIDM[LISTNUM][20], topIDR[LISTNUM][20], topIDB[LISTNUM][20],
	    tmpID[20];
	FILE *fp, *fpnew;
	struct BoardStock *bs;
	size_t filesize;
	void *stockMem;

	//初始化
	for (n = 0; n < LISTNUM; n++) {
		strcpy(topIDM[n], "null.");
		strcpy(topIDR[n], "null.");
		strcpy(topIDB[n], "null.");
		topMoney[n] = 0;
		topRob[n] = 0;
		topBeg[n] = 0;
	}

	//计算股票价值
	worth = 0;
	if (!file_exist(DIR_STOCK "stock"))
		initData(2, DIR_STOCK "stock");
	ns = get_num_records(DIR_STOCK "stock", sizeof (struct BoardStock));
	if (ns <= 0)
		goto SORT;
	filesize = sizeof (struct BoardStock) * ns;
//加载股市信息 
	if ((stockMem = loadData(DIR_STOCK "stock", filesize)) == (void*)-1)
		goto SORT;

	for (i = 0; i < STOCK_NUM; i++) {
		if (myInfo->stock[i].num <= 0)
			continue;
		bs = stockMem +
		    myInfo->stock[i].stockid * sizeof (struct BoardStock);
		worth += bs->todayPrice[3] * myInfo->stock[i].num;
	}
	unloadData(stockMem, filesize);

      SORT:
	tmpM = myInfo->cash + myInfo->credit + worth - myInfo->loan;
	tmpR = myInfo->robExp;
	tmpB = myInfo->begExp;
	strcpy(tmpID, currentuser->userid);

	//记录文件不存在
	if ((fp = fopen(DIR_MC "top", "r")) == NULL) {
		fpnew = fopen(DIR_MC "top.new", "w");
		flock(fileno(fpnew), LOCK_EX);

		strcpy(topIDM[0], tmpID);
		topMoney[0] = tmpM;
		for (n = 0; n < LISTNUM; n++)
			fprintf(fpnew, "%s %d\n", topIDM[n], topMoney[n]);

		strcpy(topIDR[0], tmpID);
		topRob[0] = tmpR;
		for (n = 0; n < LISTNUM; n++)
			fprintf(fpnew, "%s %d\n", topIDR[n], topRob[n]);

		strcpy(topIDB[0], tmpID);
		topBeg[0] = tmpB;
		for (n = 0; n < LISTNUM; n++)
			fprintf(fpnew, "%s %d\n", topIDB[n], topBeg[n]);

		flock(fileno(fpnew), LOCK_UN);
		fclose(fpnew);

		if ((fp = fopen(DIR_MC "top", "r")) == NULL)
			rename(DIR_MC "top.new", DIR_MC "top");
		else {
			fclose(fp);
			deliverreport("系统故障",
				      "\033[1;31m排名记录文件已经存在！\033[m");
		}
		return;
	}
	//记录文件存在
	flock(fileno(fp), LOCK_EX);
	//读记录
	for (n = 0; n < LISTNUM; n++)
		fscanf(fp, "%s %d\n", topIDM[n], &topMoney[n]);
	for (n = 0; n < LISTNUM; n++)
		fscanf(fp, "%s %hd\n", topIDR[n], &topRob[n]);
	for (n = 0; n < LISTNUM; n++)
		fscanf(fp, "%s %hd\n", topIDB[n], &topBeg[n]);
	//ID已上榜
	for (n = 0; n < LISTNUM; n++) {
		if (!strcmp(topIDM[n], tmpID)) {
			topMoney[n] = tmpM;
			tmpM = -1;
		}
		if (!strcmp(topIDR[n], tmpID)) {
			topRob[n] = tmpR;
			tmpR = -1;
		}
		if (!strcmp(topIDB[n], tmpID)) {
			topBeg[n] = tmpB;
			tmpB = -1;
		}
	}
	//ID未上榜
	if (tmpM > topMoney[LISTNUM - 1]) {
		strcpy(topIDM[LISTNUM - 1], tmpID);
		topMoney[LISTNUM - 1] = tmpM;
	}
	if (tmpR > topRob[LISTNUM - 1]) {
		strcpy(topIDR[LISTNUM - 1], tmpID);
		topRob[LISTNUM - 1] = tmpR;
	}
	if (tmpB > topBeg[LISTNUM - 1]) {
		strcpy(topIDB[LISTNUM - 1], tmpID);
		topBeg[LISTNUM - 1] = tmpB;
	}
	//排序
	tmpM = myInfo->cash + myInfo->credit + worth - myInfo->loan;
	tmpR = myInfo->robExp;
	tmpB = myInfo->begExp;
	for (n = 0; n < LISTNUM - 1; n++)
		for (n2 = n + 1; n2 < LISTNUM; n2++) {
			if (topMoney[n] < topMoney[n2]) {
				strcpy(tmpID, topIDM[n]);
				strcpy(topIDM[n], topIDM[n2]);
				strcpy(topIDM[n2], tmpID);
				tmpM = topMoney[n];
				topMoney[n] = topMoney[n2];
				topMoney[n2] = tmpM;
			}
			if (topRob[n] < topRob[n2]) {
				strcpy(tmpID, topIDR[n]);
				strcpy(topIDR[n], topIDR[n2]);
				strcpy(topIDR[n2], tmpID);
				tmpR = topRob[n];
				topRob[n] = topRob[n2];
				topRob[n2] = tmpR;
			}
			if (topBeg[n] < topBeg[n2]) {
				strcpy(tmpID, topIDB[n]);
				strcpy(topIDB[n], topIDB[n2]);
				strcpy(topIDB[n2], tmpID);
				tmpB = topBeg[n];
				topBeg[n] = topBeg[n2];
				topBeg[n2] = tmpB;
			}
		}
	//写入新文件
	fpnew = fopen(DIR_MC "top.new", "w");
	flock(fileno(fpnew), LOCK_EX);

	for (n = 0; n < LISTNUM; n++)
		fprintf(fpnew, "%s %d\n", topIDM[n], topMoney[n]);
	for (n = 0; n < LISTNUM; n++)
		fprintf(fpnew, "%s %d\n", topIDR[n], topRob[n]);
	for (n = 0; n < LISTNUM; n++)
		fprintf(fpnew, "%s %d\n", topIDB[n], topBeg[n]);

	flock(fileno(fpnew), LOCK_UN);
	fclose(fpnew);

	flock(fileno(fp), LOCK_UN);
	fclose(fp);

	rename(DIR_MC "top.new", DIR_MC "top");
	return;
}

void
moneycenter_byebye()
{
	clear();
	showAt(4, 14, "\033[1;32m期待您再次光临大富翁世界，"
	       "早日实现您的梦想。\033[m", YEA);
}

void
show_top()
{
	int m, n, topMoney[LISTNUM], screen_num;
	short topRob[LISTNUM], topBeg[LISTNUM];
	char topIDM[LISTNUM][20], topIDR[LISTNUM][20], topIDB[LISTNUM][20];
	FILE *fp;

	fp = fopen(DIR_MC "top", "r");
	flock(fileno(fp), LOCK_EX);

	for (n = 0; n < LISTNUM; n++)
		fscanf(fp, "%s %d\n", topIDM[n], &topMoney[n]);
	for (n = 0; n < LISTNUM; n++)
		fscanf(fp, "%s %hd\n", topIDR[n], &topRob[n]);
	for (n = 0; n < LISTNUM; n++)
		fscanf(fp, "%s %hd\n", topIDB[n], &topBeg[n]);

	flock(fileno(fp), LOCK_UN);
	fclose(fp);

	screen_num = (LISTNUM + 1) / 20;
	for (m = 0; m <= screen_num - 1; m++) {
		clear();
		prints("\033[1;44;37m  " MY_BBS_NAME
		       " BBS    \033[32m--== 富  翁  榜 ==-- "
		       "\033[33m--== 胆  识  榜 ==-- \033[36m--== 身  法  榜 ==--\033[m\r\n");
		prints
		    ("\033[1;41;37m   排名           名字       总资产         名字    "
		     "胆识         名字    身法 \033[m\r\n");

		for (n = 0; n < 20; n++)
			prints
			    ("\033[1;37m%6d\033[32m%18s\033[33m%11d\033[32m%15s"
			     "\033[33m%6d\033[32m%15s\033[33m%6d\033[m\r\n",
			     m * 20 + n + 1, topIDM[m * 20 + n],
			     topMoney[m * 20 + n], topIDR[m * 20 + n],
			     topRob[m * 20 + n], topIDB[m * 20 + n],
			     topBeg[m * 20 + n]);
		prints
		    ("\033[1;41;33m                    这是第%2d屏，按任意键查看下一屏"
		     "                            \033[m\r\n", m + 1);
		pressanykey();
	}
}

int
getOkUser(char *msg, char *uident, int line, int col)
{				// 将输入的有效id放到uident里, 成功返回1, 否则返回0 

	move(line, col);
	usercomplete(msg, uident);
	if (uident[0] == '\0')
		return 0;
	if (!getuser(uident, NULL)) {
		showAt(line + 1, col, "错误的使用者代号...", YEA);
		return 0;
	}
	return 1;
}

void
whoTakeCharge(int pos, char *boss)
{
	const char feaStr[][20] =
	    { "admin", "bank", "lottery", "gambling", "gang",
		"beggar", "stock", "shop", "police"
	};
	if (readstrvalue(MC_BOSS_FILE, feaStr[pos], boss, IDLEN + 1) != 0)
		*boss = '\0';
}

int
ismaster(char *uid)		//admin return -1, master return 1, none 0 
{
	int i, ret = 0;
	char boss[IDLEN + 1];

	for (i = 0; i <= 8; i++) {
		whoTakeCharge(i, boss);
		if (!strcmp(currentuser->userid, boss)) {
			if (i == 0)
				ret = -1;
			else
				ret = 1;
		}
	}

	return ret;
}

int
userInputValue(int line, int col, char *act, char *name, int inf, int sup)
{
	char buf[STRLEN], content[STRLEN];
	int num;

	snprintf(content, STRLEN - 1, "%s多少%s？[%d--%d]", act, name, inf,
		 sup);
	getdata(line, col, content, buf, 10, DOECHO, YEA);
	num = atoi(buf);
	num = MAX(num, inf);
	num = MIN(num, sup);
	move(line + 1, col);
	snprintf(content, STRLEN - 1, "确定%s %d %s吗？", act, num, name);
	if (askyn(content, NA, NA) == NA)
		return -1;
	return num;
}

#define EVENT_NUM (sizeof(rd_event)/sizeof(rd_event[0]))

int
randomevent()
//  碰事件国库保底资金500w,以保证稿费和工资的发放
{
	int num, totle, rat;
	char title[STRLEN], buf[256];
	struct st_Event {
		char desc1[STRLEN], desc2[STRLEN], desc3[STRLEN];
		int type, bonus;	//type:1 现金 2 胆识  3 身法 4 运气 5 体力 6 存款 etc 
	} rd_event[] = {	//注意排列顺序，人品越好，遇到上面的可能性越大 
		{
		"慷慨赞助" MY_BBS_NAME, "所有参数增加", "", 7, 10}, {
		"慷慨捐资" MY_BBS_NAME, "花费了", "的存款", 6, -5}, {
		"配合" MY_BBS_NAME "政府征地", "获得了", "赔偿金", 6, 5}, {
		"遇到大财神", "得到了", "的存款", 6, 10}, {
		"遇到小财神", "得到了", "的奖励", 0, 5}, {
		"揭露烤鱼大肆贩卖假药", "获得", "奖励", 1, 100000}, {
		"骑自行车摔伤", "体力减少到", "点", 5, -10}, {
		"成功畅游未名湖", "胆识增加了", "点", 2, 15}, {
		"利用职权倒买倒卖", "赚了", "不义之财", 1, 50000}, {
		"拍砖练功", "体力减少到", "", 5, -20}, {
		"积极发表文章", "赚到了", "稿费", 1, 30000}, {
		"修炼凌波微步", "身法提高了", "点", 3, 15}, {
		"竞选站务成功", "发工资", "", 1, 200000}, {
		"喝了光明回奶", "体力减少到", "", 5, -30}, {
		"在Accusation版与米饭辩论", "胆识增加", "点", 2, 10}, {
		"早锻炼遇到FR jj", "学习舞蹈后身法提高了", "点", 3, 10}, {
		"看贴不回贴", "人品降低", "点", 4, -5}, {
		"对意中人勇敢表白", "胆识增加", "点", 2, 10}, {
		"在公路上飙车", "身法提高了", "点", 3, 10}, {
		"在中关村出售盗版游戏光盘", "非法获利", "", 1, 100000}, {
		"用太极拳打败小流氓", "胆识增加", "点", 2, 5}, {
		"在四大名捕监考的考场作弊", "胆识增加", "点", 2, 15}, {
		"碰上了林胖子,沾染晦气", "人品降低", "点", 4, -10}, {
		"不好好学习", "逃课打篮球身法增加", "点", 3, 5}, {
		"感冒去校医院", "胆识增加", "点", 2, 5}, {
		"每天坐11路直达车上班", "身法提高了", "点", 3, 5}, {
		"一心多用劈腿", "人品减少", "点", 4, -15}, {
		"孝敬父母", "人品提升", "点", 4, 20}, {
		"竞选站务成功", "答谢支持者花费", "", 1, -200000}, {
		"开车撞上电线杆", "身法降低了", "点", 3, -5}, {
		"违章被警察拦住", "胆识降低了", "点", 2, -5}, {
		"遇到疯狗", "被咬伤后身法降低了", "点", 3, -5}, {
		"组织超男比赛失败导致全国人民呕吐", "赔偿了",
			    "精神损失费", 1, -200000}, {
		"抗议食堂在沙粒里面掺米饭", "人品提升", "点", 4, 10}, {
		"跟泼妇吵架被抓破脸", "胆识降低了", "点", 2, -5}, {
		"成功证明了金融中心编写者的阴险狡诈", "人品增加", "点", 4, 10},
		{
		"对非游戏玩家恶意撕票", "人品降低", "点", 4, -20}, {
		"到处灌水", "被处以", "的罚款", 1, -30000}, {
		"策划伦敦爆炸", "人品降低", "点", 4, -20}, {
		"在公交车上被性骚扰", "胆识减少了", "点", 2, -10}, {
		"拣到钱包", "翻出来", "偷偷放进了自己口袋。", 1, 100000}, {
		"被小流氓打伤", "身法降低", "点", 3, -10}, {
		"对意中人勇敢表白", "为此采购道具花了", "的现金", 1, -100000},
		{
		"拾金不昧", "人品提升", "点", 4, 5}, {
		"晚上回家路上被抢劫", "身法降低", "点", 3, -10}, {
		"收购废铜烂铁", "赚了", "", 1, 50000}, {
		"组织“我爱航母”募捐活动", "获得捐款", "", 1, 200000}, {
		"新买自行车", "花费", "的现金", 1, -50000}, {
		"被黑帮绑架", "胆识减少了", "点", 2, -10}, {
		"贪小便宜买了假货", "损失", "的现金", 1, -100000}, {
		"吃了千年人参", "体力增加到", "", 5, 100}, {
		"遇到痴女对他表白", "胆识减少了", "点", 2, -15}, {
		"服下大力丸", "体力增加到", "", 5, 40}, {
		"遇到小衰神", "丢失了", "的现金", 0, -5}, {
		"沉迷于赌博", "输掉了", "现金", 1, -200000}, {
		"买了新车Passat", "花了", "", 1, -100000}, {
		"遇到大衰神", "丢了一张", "的存折", 6, -10}, {
		"在游乐场不慎摔伤", "身法降低", "点", 3, -15}, {
		"坚持锻炼身体", "体力增加到", "点", 5, 20}, {
		"企图不利于" MY_BBS_NAME, "全部指数下降", "", 7, -10}
	};

	if (!(myInfo->GetLetter == 1)) {
		clear();
		showAt(5, 4, "你已经关闭了金融中心游戏功能，请开启后再来。",
		       YEA);
		return 0;
	}

	myInfo->Actived = 0;
	money_show_stat("神秘世界");

	move(4, 4);
	prints("不知道过了多久，你慢慢醒了过来，发现自己到了一个神秘的地方。\n"
	       "    你正奇怪，突然听到一个声音对你说到：”欢迎来到神秘世界！“\n"
	       "    你回答正确一个问题后，就会遇到一次随机事件，祝你好运！");
	pressreturn();
	money_show_stat("神秘世界");
	if (check_health(10, 12, 4, "啊喔, 没体力了，还是算了吧。。。", YEA))
		return 0;

	if (show_cake()) {
		prints("你别是个机器人吧...\n");
		pressreturn();
		return 0;
	}

	clear();
	update_health();
	money_show_stat("神秘世界");

	rat = EVENT_NUM * (myInfo->luck + 100) / 800;	//降低了人品对事件好坏的影响,避免新手陷入恶性循环
	num = random() % (EVENT_NUM - rat);
	if (myInfo->luck == -100)	//-100人品的惩罚
		num = EVENT_NUM - 1;
	if (!num && (myInfo->luck < 100))	//人品100才能赞助
		num++;
/*	if (rat < 0)
		num -= rat;

	num = abs(num) % EVENT_NUM;	//上面几行在算什么？限制在数组界内吧
*/
	switch (rd_event[num].type) {
	case 0:		//金钱百分比 
		totle =
		    MIN(myInfo->cash * rd_event[num].bonus / 100,
			MAX_MONEY_NUM / 2);
		move(6, 4);
		if (totle < 0)
			totle = MAX(totle, -myInfo->cash);
		else {
			totle = MIN(totle, (mcEnv->Treasury - 5000000) / 2);
			totle = MAX(totle, 0);
		}
		mcEnv->Treasury -= totle;
		myInfo->cash = MAX(0, (myInfo->cash + totle));
		prints("你%s，%s%d%s%s。", rd_event[num].desc1,
		       rd_event[num].desc2, abs(totle), MONEY_NAME,
		       rd_event[num].desc3);
		sprintf(title, "【事件】%s%s", currentuser->userid,
			rd_event[num].desc1);
		sprintf(buf, "%s%s，%s%d%s%s。", currentuser->userid,
			rd_event[num].desc1, rd_event[num].desc2, abs(totle),
			MONEY_NAME, rd_event[num].desc3);
		break;
	case 1:		//金钱数量 
		if ((myInfo->cash + myInfo->credit <=
		     abs(rd_event[num].bonus) * 10)
		    || (myInfo->cash <= rd_event[num].bonus * 2))
			//降低减少现金事件对新手的影响
			totle = rd_event[num].bonus / 5;
		else
			totle = rd_event[num].bonus;
		if (totle < 0)
			totle = MAX(totle, -myInfo->cash);
		else {
			totle = MIN(totle, (mcEnv->Treasury - 5000000) / 2);
			totle = MAX(totle, 0);
		}
		mcEnv->Treasury -= totle;
		move(6, 4);
		myInfo->cash = MAX(0, (myInfo->cash + totle));
		prints("你%s，%s%d%s%s。", rd_event[num].desc1,
		       rd_event[num].desc2, abs(totle), MONEY_NAME,
		       rd_event[num].desc3);
		sprintf(title, "【事件】%s%s", currentuser->userid,
			rd_event[num].desc1);
		sprintf(buf, "%s%s，%s%d%s%s。", currentuser->userid,
			rd_event[num].desc1, rd_event[num].desc2, abs(totle),
			MONEY_NAME, rd_event[num].desc3);
		break;
	case 2:		//胆识 
		totle = rd_event[num].bonus;
		move(6, 4);
		myInfo->robExp = MAX(0, (myInfo->robExp + totle));
		prints("你%s，%s%d%s。", rd_event[num].desc1,
		       rd_event[num].desc2, abs(totle), rd_event[num].desc3);
		sprintf(title, "【事件】%s%s", currentuser->userid,
			rd_event[num].desc1);
		sprintf(buf, "%s%s，%s%d%s。", currentuser->userid,
			rd_event[num].desc1, rd_event[num].desc2, abs(totle),
			rd_event[num].desc3);
		break;
	case 3:		//身法 
		totle = rd_event[num].bonus;
		move(6, 4);
		myInfo->begExp = MAX(0, (myInfo->begExp + totle));
		prints("你%s，%s%d%s。", rd_event[num].desc1,
		       rd_event[num].desc2, abs(totle), rd_event[num].desc3);
		sprintf(title, "【事件】%s%s", currentuser->userid,
			rd_event[num].desc1);
		sprintf(buf, "%s%s，%s%d%s。", currentuser->userid,
			rd_event[num].desc1, rd_event[num].desc2, abs(totle),
			rd_event[num].desc3);
		break;
	case 4:		//人品 
		totle = rd_event[num].bonus;
		if ((myInfo->cash + myInfo->credit < abs(totle) * 100000) && (myInfo->robExp + myInfo->begExp < 1000))	//降低人品事件对新手的影响
			totle /= 5;
		move(6, 4);
		myInfo->luck = MAX(-100, (myInfo->luck + totle));
		prints("你%s，%s%d%s。", rd_event[num].desc1,
		       rd_event[num].desc2, abs(totle), rd_event[num].desc3);
		sprintf(title, "【事件】%s%s", currentuser->userid,
			rd_event[num].desc1);
		sprintf(buf, "%s%s，%s%d%s。", currentuser->userid,
			rd_event[num].desc1, rd_event[num].desc2, abs(totle),
			rd_event[num].desc3);
		break;
	case 5:		//体力 
		update_health();
		totle =
		    MIN(100 + 2 * myInfo->con,
			rd_event[num].bonus + myInfo->health);
		move(6, 4);
		myInfo->health = MAX(totle, 0);
		prints("你%s，%s%d%s。", rd_event[num].desc1,
		       rd_event[num].desc2, myInfo->health,
		       rd_event[num].desc3);
		sprintf(title, "【事件】%s%s", currentuser->userid,
			rd_event[num].desc1);
		sprintf(buf, "%s%s，%s%d%s。", currentuser->userid,
			rd_event[num].desc1, rd_event[num].desc2,
			myInfo->health, rd_event[num].desc3);
		break;
	case 6:		//存款百分比 
		if (rd_event[num].bonus < 0)
			//totle = MAX(-MAX_MONEY_NUM, myInfo->credit * rd_event[num].bonus/100);
			totle =
			    MAX(-2000000,
				myInfo->credit * rd_event[num].bonus / 100);
		else
			//totle = MIN(MAX_MONEY_NUM, myInfo->credit * rd_event[num].bonus/100);
			totle =
			    MIN(2000000,
				myInfo->credit * rd_event[num].bonus / 100);
		if (totle < 0)
			totle = MAX(totle, -myInfo->credit);
		else {
			totle = MIN(totle, (mcEnv->Treasury - 5000000) / 2);
			totle = MAX(totle, 0);
		}
		mcEnv->Treasury -= totle;
		move(6, 4);
		myInfo->credit = MAX(0, (myInfo->credit + totle));
		prints("你%s，%s%d%s%s。", rd_event[num].desc1,
		       rd_event[num].desc2, abs(totle), MONEY_NAME,
		       rd_event[num].desc3);
		if (rd_event[num].bonus <= 3) {
			myInfo->health = 0;
			move(7, 4);
			prints("经历了这么大的事，你忙得虚脱了。");
		}
		sprintf(title, "【事件】%s%s", currentuser->userid,
			rd_event[num].desc1);
		sprintf(buf, "%s%s，%s%d%s%s。", currentuser->userid,
			rd_event[num].desc1, rd_event[num].desc2, abs(totle),
			MONEY_NAME, rd_event[num].desc3);
		break;
	case 7:		//全部
		totle = MIN(myInfo->cash * rd_event[num].bonus / 100, 2000000);
		totle = MIN(totle, (mcEnv->Treasury - 5000000) / 2);
		totle = MAX(totle, 0);
		myInfo->cash += totle;
		mcEnv->Treasury -= totle;
		totle =
		    MIN(myInfo->credit * rd_event[num].bonus / 100, 2000000);
		totle = MIN(totle, (mcEnv->Treasury - 5000000) / 2);
		totle = MAX(totle, 0);
		myInfo->credit += totle;
		mcEnv->Treasury -= totle;
		myInfo->robExp +=
		    MIN(myInfo->robExp * rd_event[num].bonus / 100, 50);
		myInfo->begExp +=
		    MIN(myInfo->begExp * rd_event[num].bonus / 100, 50);
		myInfo->luck += abs(myInfo->luck) * rd_event[num].bonus / 100;
		myInfo->health = 0;
		move(6, 4);
		prints("你%s，%s。\n    经历那么大的事，你忙的虚脱了。",
		       rd_event[num].desc1, rd_event[num].desc2);
		sprintf(title, "【事件】%s%s", currentuser->userid,
			rd_event[num].desc1);
		sprintf(buf, "%s%s，%s。", currentuser->userid,
			rd_event[num].desc1, rd_event[num].desc2);
		break;
	}
	if (random() % (MAX_ONLINE / 10) == 0 || rd_event[num].type == 7
	    || rd_event[num].type == 6)
		deliverreport(title, buf);
	pressanykey();
	sleep(1);
	return 1;

}

void
money_show_stat(char *position)
{
	clear();
	if (chkmail()) {
		move(0, 30);
		prints("\033[1;5;36m[您有信件]\033[m");
	}
	if (mcEnv->closed && !USERPERM(currentuser, PERM_SYSOP) && (strcmp("admin", currentuser->userid))) {
		move(12, 4);
                prints("    大富翁关闭了。。。掰掰，下次见喽:D");
		unloadData(myInfo, sizeof (mcUserInfo));
		unloadData(mcEnv, sizeof (MC_Env));
		pressreturn();
                Q_Goodbye();
	  }

	if (time(NULL) < myInfo->freeTime) {
		move(12, 4);
		prints("    啊，你上次做的事犯了！你被警察抓获了！");
		myInfo->mutex = 0;
		unloadData(myInfo, sizeof (mcUserInfo));
		unloadData(mcEnv, sizeof (MC_Env));
		pressreturn();
		Q_Goodbye();
	}

	move(1, 0);
	update_health();
	prints
	    ("您的代号：\033[1;33m%s\033[m      胆识 \033[1;33m%d\033[m  身法 \033[1;33m%d\033[m  人品 \033[1;33m%d\033[m  根骨 \033[1;33m%d\033[m  体力 \033[1;33m%d\033[m \n",
	     currentuser->userid, myInfo->robExp, myInfo->begExp, myInfo->luck,
	     myInfo->con, myInfo->health);
	prints("您身上带着 \033[1;31m%d\033[m %s，", myInfo->cash, MONEY_NAME);
	prints("存款 \033[1;31m%d\033[m %s。当前位置 \033[1;33m%s\033[m",
	       myInfo->credit, MONEY_NAME, position);
	move(3, 0);
	prints
	    ("\033[1m--------------------------------------------------------------------------------\033[m");
}

void
nomoney_show_stat(char *position)
{
	clear();
	if (chkmail()) {
		move(0, 30);
		prints("\033[1;5;36m[您有信件]\033[m");
	}
	if (mcEnv->closed && !USERPERM(currentuser, PERM_SYSOP) && (strcmp("admin", currentuser->userid))) {
	        move(12, 4);
                prints("    大富翁关闭了。。。掰掰，下次见喽:D");
                unloadData(myInfo, sizeof (mcUserInfo));
                unloadData(mcEnv, sizeof (MC_Env));
                pressreturn();
                Q_Goodbye();
          }

	if (time(NULL) < myInfo->freeTime) {
		move(12, 4);
		prints("    啊，你上次做的事犯了！你被警察抓获了！");
		myInfo->mutex = 0;
		unloadData(myInfo, sizeof (mcUserInfo));
		unloadData(mcEnv, sizeof (MC_Env));
		pressreturn();
		Q_Goodbye();
	}
	update_health();
	if (myInfo->luck > 100)
		myInfo->luck = 100;
	if (myInfo->luck < -100)
		myInfo->luck = -100;
	move(1, 0);
	prints
	    ("您的代号：\033[1;33m%s\033[m      胆识 \033[1;33m%d\033[m  身法 \033[1;33m%d\033[m  人品 \033[1;33m%d\033[m  根骨 \033[1;33m%d\033[m  体力 \033[1;33m%d\033[m \n",
	     currentuser->userid, myInfo->robExp, myInfo->begExp, myInfo->luck,
	     myInfo->con, myInfo->health);
	prints
	    ("\033[1;32m欢迎光临%s大富翁世界，当前位置是\033[0m \033[1;33m%s\033[0m",
	     CENTER_NAME, position);
	move(3, 0);
	prints
	    ("\033[1m--------------------------------------------------------------------------------\033[m");
}

int
check_allow_in()
{
	time_t freeTime, backTime, currTime = time(NULL);
	int day, hour, minute, num;
	char buf[256], admin[IDLEN + 1];

	clear();
	move(9, 8);
	whoTakeCharge(0, admin);
	if (mcEnv->closed && !USERPERM(currentuser, PERM_SYSOP) && (strcmp(admin, currentuser->userid))) {	/* 大富翁世界关闭 */
		showAt(10, 10, "大富翁世界关闭中...请稍后再来", YEA);
		return 0;
	}
	if (mcEnv->openTime > currentuser->lastlogin) {
		showAt(10, 4,
		       "由于修改了代码，你需要退出所有窗口再重新登录才允许进入大富翁世界",
		       YEA);
		return 0;
	}

	if (myInfo->mutex++ && count_uindex_telnet(usernum) > 1) {	// 避免多窗口, 同时处理掉线 
		showAt(10, 10, "你已经在大富翁世界里啦!", YEA);
		return -1;
	}
/* 犯罪被监禁 */
	clrtoeol();
	freeTime = myInfo->freeTime;
	if (currTime < freeTime) {
		day = (freeTime - currTime) / 86400;
		hour = (freeTime - currTime) % 86400 / 3600;
		minute = (freeTime - currTime) % 3600 / 60 + 1;
		if (seek_in_file(DIR_MC "policemen", currentuser->userid)) {
			prints("你执行任务受伤还需要修养%d天%d小时%d分钟。",
			       day, hour, minute);
			num =
			    (sqrt(myInfo->robExp + myInfo->begExp) / 2 +
			     40) * (freeTime - currTime);
			if (num>2000000000)
				num=2000000000;
			if (num < 0)
				num=MAX_MONEY_NUM; // bug fix, temp
			sprintf(buf, "神说：你只要捐献%d%s就可以重获新生!", num,
				MONEY_NAME);
			move(11, 8);
			if (askyn(buf, YEA, NA) == NA) {
				pressanykey();
				return 0;
			} else {
				if (myInfo->credit < num) {
					prints("\n对神不敬，让你多躺一会儿!");
					myInfo->freeTime += 600;
					pressanykey();
					return 0;
				} else {
					myInfo->credit -= num;
					mcEnv->Treasury += num;
					prints
					    ("\n你突然有了脱胎换骨的感觉，果然是神迹啊!");
					myInfo->freeTime = 0;
				}
			}
		} else {
			prints("你被%s警署监禁了。还有%d天%d小时%d分钟的监禁。",
			       CENTER_NAME, day, hour, minute);
			num =
			    (sqrt(myInfo->robExp + myInfo->begExp) / 2 +
			     40) * (freeTime - currTime);
			if (num>2000000000)
				num=2000000000;
			if(num < 0)
				num = MAX_MONEY_NUM;
			sprintf(buf, "你只需要缴纳%d%s就可以重新获得自由!",
				num,
				MONEY_NAME);
			move(11, 8);
			if (askyn(buf, YEA, NA) == NA) {
				pressanykey();
				return 0;
			} else {
				if (myInfo->credit < num) {
					prints
					    ("\n没钱还敢戏弄警方？再加刑10分钟！");
					myInfo->freeTime += 600;
					pressanykey();
					return 0;
				} else {
					myInfo->credit -= num;
					mcEnv->Treasury += num;
					prints
					    ("\n恭喜你重新获得自由,这次可要安分守己哦!");
					myInfo->freeTime = 0;
				}
			}
		}
		pressanykey();
	} else if (currTime > freeTime && freeTime > 0) {
		myInfo->freeTime = 0;
		if (seek_in_file(DIR_MC "policemen", currentuser->userid))
			showAt(10, 10, "恭喜你伤愈出院！", YEA);
		else
			showAt(10, 10, "监禁期满，恭喜你重新获得自由！", YEA);
	}
	clrtoeol();
/* 欠款不还 */
	backTime = myInfo->backTime;
	if (currTime > backTime && backTime > 0) {
		if (askyn("你欠银行的贷款到期了，赶紧还吧？", YEA, NA) == NA)
			return 0;
		money_bank();
		return 0;
	}
// 坏人名单
	if (seek_in_file(DIR_MC "bannedID", currentuser->userid)) {
		showAt(10, 10,
		       "你扰乱大富翁世界金融秩序，被取消进入大富翁世界的资格。\n"
		       "          请与大富翁总管联系。", YEA);
		pressanykey();
		return 0;
	}
	return 1;
}

int
positionChange(int pos, char *boss, char *posStr, int type)
{
	char head[16], in[16], end[16];
	char buf[STRLEN], title[STRLEN], letter[2 * STRLEN];
	char posDesc[][20] =
	    { "大富翁世界总管", "银行行长", "博彩公司经理", "赌场经理",
		"黑帮帮主", "丐帮帮主", "证监会主席",
		"商场经理", "警署署长"
	};
	char ps[][STRLEN] =
	    { "谨望其能廉洁奉公，不以权谋私利，为本站金融事业的发展鞠躬尽瘁。",
		"大富翁世界对其一直以来的工作表示感谢，祝以后顺利！"
	};
	if (type == 0) {
		strcpy(head, "任命");
		strcpy(in, "为");
		strcpy(end, "");
	} else {
		strcpy(head, "免去");
		strcpy(in, "的");
		strcpy(end, "职务");
	}
	move(20, 4);
	snprintf(title, STRLEN - 1, "【总管】%s%s%s%s%s", head, boss, in,
		 posDesc[pos], end);
	sprintf(genbuf, "确定要 %s 吗", (title + 8));	//截去 [公告] 
	if (askyn(genbuf, YEA, NA) == NA)
		return 0;
	sprintf(genbuf, "%s %s", posStr, boss);
	if (type == 0) {
		addtofile(MC_BOSS_FILE, genbuf);
		sprintf(letter, "%s", ps[0]);
	} else {
		getdata(21, 4, "原因：", buf, 40, DOECHO, YEA);
		sprintf(letter, "原因：%s\n\n%s", buf, ps[1]);
		del_from_file(MC_BOSS_FILE, posStr);
	}
	deliverreport(title, letter);
	system_mail_buf(letter, strlen(letter), boss, title,
			currentuser->userid);
	showAt(22, 4, "手续完成", YEA);
	return 1;
}

//  --------------------------    管理    ------------------------  // 

void
specil_MoneyGive()
{
	int money;
	char uident[IDLEN + 1], admin[IDLEN + 1], bank[IDLEN + 1], reason[256],
	    buf[256], title[STRLEN];
	mcUserInfo *mcuInfo;

	money_show_stat("特别拨款小金库");
	if (!getOkUser("你拨款给谁？", uident, 6, 4)) {
		move(7, 4);
		prints("查无此人");
		pressanykey();
		return;
	}

	sethomefile(buf, uident, "mc.save");
	if (!file_exist(buf))
		initData(1, buf);
	money = userInputValue(7, 4, "拨", "万", 1, 10000);
	if (money < 0)
		return;

	getdata(10, 4, "理由：", reason, 40, DOECHO, YEA);

	if (reason[0] == '\0' || reason[0] == ' ') {
		showAt(12, 4, "没有理由不许私自拨款！", YEA);
		return;
	}

	if (mcEnv->Treasury < money * 10000) {
		showAt(14, 4, "国库空虚，无法拨款。", YEA);
		return;
	}

	move(11, 4);
	if (askyn("你确定要拨款吗？", YEA, NA) == YEA) {
		move(12, 4);
		if (askyn
		    ("使用秘密拨款形式？（难道有什么见不得人的？慎用！）", NA,
		     NA) == YEA) {
			mcuInfo = loadData(buf, sizeof (mcUserInfo));
			if(mcuInfo == (void*)-1)
				return;
			mcuInfo->credit += 10000 * money;
			//mcEnv->Treasury -= 10000 * money;   //秘密拨款不通过国库，是在发行货币
			myInfo->Actived++;
			sprintf(title, "总管%s秘密拨款%d万给%s",
				currentuser->userid, money, uident);
			sprintf(buf,
				"总管%s秘密拨款%d万给%s,其中也许有猫腻。 \n理由：%s\n",
				currentuser->userid, money, uident, reason);
			system_mail_buf(buf, strlen(buf), uident, title,
					currentuser->userid);
			if (readstrvalue
			    (MC_BOSS_FILE, "admin", admin, IDLEN + 1) != 0)
				admin[0] = '\0';
			system_mail_buf(buf, strlen(buf), admin, title,
					currentuser->userid);
			if (readstrvalue(MC_BOSS_FILE, "bank", bank, IDLEN + 1)
			    != 0)
				bank[0] = '\0';
			system_mail_buf(buf, strlen(buf), bank, title,
					currentuser->userid);

			unloadData(mcuInfo, sizeof (mcUserInfo));
			showAt(16, 4, "拨款成功", YEA);
		} else {
			mcuInfo = loadData(buf, sizeof (mcUserInfo));
			if(mcuInfo == (void*)-1)
				return;
			mcuInfo->credit += 10000 * money;
			if (mcuInfo->GetLetter == 0)
				mcEnv->Treasury -= 100000 * money;
			else
				mcEnv->Treasury -= 10000 * money;
			myInfo->Actived++;
			sprintf(title, "总管%s特别拨款%d万给%s",
				currentuser->userid, money, uident);
			sprintf(buf, "总管%s特别拨款%d万给%s。\n理由：%s\n",
				currentuser->userid, money, uident, reason);
			system_mail_buf(buf, strlen(buf), uident, title,
					currentuser->userid);
			deliverreport(title, buf);

			unloadData(mcuInfo, sizeof (mcUserInfo));
			showAt(16, 4, "拨款成功", YEA);
		}
	}
	return;
}

void
promotion(int type, char *prompt)
{
	int pos;
	char boss[IDLEN], buf[STRLEN];
	char *feaStr[] = { "admin", "bank", "lottery", "gambling",
		"gang", "beggar", "stock", "shop", "police"
	};

	getdata(16, 4,
		"职位: [0.总管 1.银行 2.彩票 3.赌场 4.黑帮 5.丐帮 6.股市 7.商场 8.警署 ]",
		buf, 2, DOECHO, YEA);
	pos = atoi(buf);
	if (pos > 8 || pos < 0)
		return;
	whoTakeCharge(pos, boss);
	move(17, 4);
	if (boss[0] != '\0' && type == 0) {
		prints("%s已经负责该职位。", boss);
		pressanykey();
		return;
	}
	if (boss[0] == '\0' && type == 1) {
		showAt(16, 4, "目前并无人负责该职位。", YEA);
		return;
	}
	if (type == 0 && !getOkUser(prompt, boss, 18, 4))
		return;
	positionChange(pos, boss, feaStr[pos], type);
}

int
money_admin()
{
	int ch, quit = 0, money = 0;
	char admin[IDLEN + 1], title[STRLEN], buf[256];

	if (readstrvalue(MC_BOSS_FILE, "admin", admin, IDLEN + 1) != 0)
		admin[0] = '\0';
	if (strcmp(admin, currentuser->userid)
	    && !(currentuser->userlevel & PERM_SYSOP))
		return 0;
	while (!quit) {
		nomoney_show_stat("大富翁世界管理");
		sprintf(buf, "    这里负责%s大富翁世界的人事管理。\n"
			"    现任总管是: \033[1;32m%s\033[m\n"
			"    国库储备金: \033[1;33m%15Ld\033[m %s\n",
			//"    用户总财产: \033[1;32m%15Ld\033[m %s",
			CENTER_NAME, admin, mcEnv->Treasury, MONEY_NAME);
		//,all_user_money(), MONEY_NAME);
		showAt(5, 0, buf, NA);
		showAt(10, 0,
		       "        1. 任命职位                    2. 免去职位\n"
		       "        3. 列出职位名单                4. 关闭/开启大富翁世界\n"
		       "        5. 特别拨款                    6. 印钱 (慎用！！！)\n"
		       "        7. 设定坏人名单                8. 统计用户信息\n"
		       "        Q. 退出", NA);
		ch = igetkey();
		switch (ch) {
		case '1':
			promotion(0, "任命谁？");
			break;
		case '2':
			promotion(1, "免去谁？");
			break;
		case '3':
			clear();
			move(1, 0);
			prints("目前%s大富翁世界各职位情况：", CENTER_NAME);
			listfilecontent(MC_BOSS_FILE);
			FreeNameList();
			pressanykey();
			break;
		case '4':
			move(15, 4);
			if (mcEnv->closed)
				sprintf(genbuf, "%s", "确定要开启么");
			else
				sprintf(genbuf, "%s", "确定要关闭么");
			if (askyn(genbuf, NA, NA) == NA)
				break;
			mcEnv->closed = !mcEnv->closed;
			if (!mcEnv->closed)
				mcEnv->openTime = time(NULL);
			showAt(16, 4, "操作成功", YEA);
			break;
		case '5':
			update_health();
			if (check_health
			    (1, 12, 4, "您工作太辛苦了，休息一下吧！", YEA))
				break;
			specil_MoneyGive();
			update_health();
			myInfo->health--;
			break;
		case '6':
			update_health();
			if (check_health
			    (1, 12, 4, "您工作太辛苦了，休息一下吧！", YEA))
				break;
			money = userInputValue(12, 4, "发行", "亿", 1, 10);
			if (money < 0)
				break;
			mcEnv->Treasury += money * 100000000;
			move(15, 4);
			prints("你已经印了%d亿%s并注入了国库。", money,
			       MONEY_NAME);
			sprintf(title, "【国库】%s总管%s向国库注入了%s",
				CENTER_NAME, currentuser->userid, MONEY_NAME);
			sprintf(buf, "为缓解经济危机，总管%s批准印钞%d亿%s",
				currentuser->userid, money, MONEY_NAME);
			deliverreport(title, buf);
			break;
		case '7':
			banID();
			break;
		case '8':
			all_user_money();
			break;
		case 'Q':
		case 'q':
			quit = 1;
			break;
		}
	}
	return 0;
}

void
banID(void)
{
	int found;
	char uident[IDLEN + 1], buf[256], title[STRLEN];
	mcUserInfo *mcuInfo;

	if (!getOkUser("请输入ID: ", uident, 12, 4))
		return;
	found = seek_in_file(DIR_MC "bannedID", uident);
	if (found) {
		sprintf(buf, "%s已在坏人名单，要将其释放吗？", uident);
		move(13, 4);
		if (askyn(buf, NA, NA) == NA)
			return;
		del_from_file(DIR_MC "bannedID", uident);
		sprintf(title, "【总管】允许%s进入大富翁世界", uident);
		sprintf(buf,
			"    鉴于%s已有改悔之心，大富翁总管%s允许他重新进入大富翁世界。",
			uident, currentuser->userid);
		deliverreport(title, buf);
		system_mail_buf(buf, strlen(buf), uident, title,
				currentuser->userid);
		showAt(14, 4, "操作成功。", YEA);
		return;
	} else {
		sprintf(buf, "你确定将%s加入坏人名单吗？他的所有指数将被清零！",
			uident);
		move(13, 4);
		if (askyn(buf, NA, NA) == NA)
			return;
		addtofile(DIR_MC "bannedID", uident);
		sethomefile(buf, uident, "mc.save");
		if (!file_exist(buf))
			initData(1, buf);
		if ((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
			return;
		mcuInfo->robExp = 0;
		mcuInfo->begExp = 0;
		mcuInfo->luck = 0;
		mcEnv->Treasury += mcuInfo->cash + mcuInfo->credit;
		mcuInfo->cash = 0;
		mcuInfo->credit = 0;
		unloadData(mcuInfo, sizeof (mcUserInfo));
		sprintf(title, "【总管】大富翁总管%s取消%s进入大富翁世界的资格",
			currentuser->userid, uident);
		sprintf(buf,
			"    由于%s扰乱大富翁世界金融秩序，被取消进入大富翁世界的资格。\n"
			"    其所有指数已被清零。如果想重新进入大富翁世界，请与总管联系。",
			uident);
		deliverreport(title, buf);
		system_mail_buf(buf, strlen(buf), uident, title,
				currentuser->userid);
		showAt(14, 4, "操作成功。", YEA);
		return;
	}
}

void
all_user_money()
{
	long long AllUserMoneyA = 0, AllUserMoneyB = 0;
	FILE *fp, *fw;
	char buf[256], fname[STRLEN], admin[IDLEN + 1];
	mcUserInfo *mcuInfo;
	int money;

	nomoney_show_stat("大富翁世界管理");
	money = userInputValue(12, 4, "调查", "万以上的用户", 10, 1000);
	if (money < 0)
		return;
	sprintf(fname, DIR_MC "UserMoney");
	fp = fopen(DIR_MC "mc_user", "r");
	fw = fopen(DIR_MC "UserMoney", "w");
	flock(fileno(fp), LOCK_EX);
	while (fscanf(fp, "%s\n", buf) != EOF) {
		if (!file_exist(buf)) {
			del_from_file(DIR_MC "mc_user", buf);
			continue;
		}
		if ((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
			continue;
		if (mcuInfo->GetLetter == 1)
			AllUserMoneyA +=
			    mcuInfo->cash + mcuInfo->credit + myInfo->interest -
			    mcuInfo->loan;
		else
			AllUserMoneyB +=
			    mcuInfo->cash + mcuInfo->credit + myInfo->interest -
			    mcuInfo->loan;
		if (mcuInfo->cash + mcuInfo->credit > 10000 * money) {
			fprintf(fw, "%s  %d\n", buf,
				mcuInfo->cash + mcuInfo->credit +
				myInfo->interest - mcuInfo->loan);
		}
		unloadData(mcuInfo, sizeof (mcUserInfo));
	}

	move(8, 5);
	sprintf(buf, "    用户游戏货币: \033[1;32m%15lld\033[m %s",
		AllUserMoneyA, MONEY_NAME);
	prints("%s", buf);
	move(9, 5);
	sprintf(buf, "    用户虚拟货币: \033[1;32m%15lld\033[m %s",
		AllUserMoneyB, MONEY_NAME);
	prints("%s", buf);
	pressanykey();
	if (readstrvalue(MC_BOSS_FILE, "admin", admin, IDLEN + 1) != 0)
		admin[0] = '\0';
	system_mail_file(fname, admin, "统计报表", "deliver");
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	fclose(fw);
//      unlink(fname);

	return;
}

int
money_game()
{
	char ch, quit = 0;

	while (!quit) {
		money_show_stat("控制中心");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m 选单 \033[1;46m [1]游戏功能开启 [2]游戏功能关闭 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			move(6, 4);
			prints
			    ("如果您同意大富翁游戏规则以及将来可能出现的变化，并愿意参与到游戏中");
			move(8, 4);
			prints("请开启游戏功能,无限精彩等着您。");
			move(10, 4);
			prints
			    ("开启游戏功能之后，您将享有使用金融中心内所有游戏功能的权利。");
			move(12, 4);
			prints
			    ("相应的，您可能遭受金融中心内游戏功能所带来的损失。");
			move(14, 4);
			prints
			    ("当您开启游戏功能的时候，您所有的虚拟货币将按照1:10的比例兑换为游戏币。");
			move(16, 4);
			if (askyn("您确实要开启游戏功能吗?", NA, NA) == YEA) {
				if (myInfo->GetLetter == 0) {
					mcEnv->Treasury -=
					    (myInfo->cash + myInfo->credit +
					     myInfo->interest) * 9;
					myInfo->cash *= 10;
					myInfo->credit *= 10;
					myInfo->interest *= 10;
					myInfo->GetLetter = 1;
				} else
					myInfo->GetLetter = 1;
			}
			break;
		case '2':
			move(6, 4);
			prints
			    ("如果您不愿意进行大富翁游戏或者希望暂时休息一段时间，请选择关闭游戏功能。");
			move(8, 4);
			prints
			    ("关闭游戏功能之后，您将不能使用金融中心内除转帐和储蓄外的所有游戏功能的权利。");
			move(10, 4);
			prints
			    ("相应的，您也不会因为金融中心内的游戏功能而受到损失。");
			move(12, 4);
			prints("如果您选择了关闭游戏功能");
			move(14, 4);
			prints
			    ("您所有的游戏币将按照20:1的比例兑换为虚拟货币，并且您的游戏内属性将损失10%%。");
			move(16, 4);
			if (askyn("您确实要关闭游戏功能吗?", NA, NA) == YEA) {
				if (!(myInfo->GetLetter == 0)) {
					mcEnv->Treasury +=
					    (myInfo->cash + myInfo->credit +
					     myInfo->interest) / 20 * 19;
					myInfo->cash /= 20;
					myInfo->credit /= 20;
					myInfo->interest /= 20;
					myInfo->GetLetter = 0;
					myInfo->robExp -= myInfo->robExp / 10;
					myInfo->begExp -= myInfo->begExp / 10;
				} else
					myInfo->GetLetter = 0;
			}
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	return 1;
}

int
money_hall()
{
	char ch, quit = 0;

	while (!quit) {
		nomoney_show_stat("大富翁世界中央大厅");
		move(6, 4);
		prints
		    ("这里是%s大富翁世界的中央大厅，可以通往各地。人来人往很是热闹。"
		     "\n    据说最近还经常有人在这里遇上神秘事件。。。",
		     CENTER_NAME);
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m 选单 \033[1;46m [1]大富翁榜 [2]承包竞价 [3]游戏功能 [4]总管办公室 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			show_top();
			break;
		case '2':
			showAt(12, 4, "\033[1;32m功能开发中。\033[m", YEA);
			break;
		case '3':
			money_game();
			break;
		case '4':
			money_admin();
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	return 0;
}

void
policeCheck(void)
{
	char passbuf[32];
	clear();
	nomoney_show_stat("警官巡视");
	move(5, 8);
	prints("=============== 警 官 巡 视 ============");
	move(8, 0);
	prints
	    ("      最近赌场劫案频发，所以经常会有警官进来巡视，盘查可疑目标。"
	     "\n  啊呀，不好！警官向你走了过来。");
	pressanykey();
	move(10, 6);
	if (random() % 3)
		prints("还好还好，警官目不斜视的从你身边走过去了。");
	else {
		prints
		    ("警官上下打量了你一眼，慢条斯理的说：“需要身份确认！”");
		getdata(11, 6, "请输入密码(只有一次机会): ",
			passbuf, PASSLEN, NOECHO, YEA);
		if (passbuf[0] == '\0' || passbuf[0] == '\n'
		    || !checkpasswd(currentuser->passwd,
				    currentuser->salt, passbuf)) {
			prints
			    ("\n      警官怒吼：“身份验证失败！跟我走一趟！”"
			     "\n      555，你被没收所有现金，并且被监禁10分钟。");
			//mcLog("在赌场被警察监禁，损失", myInfo->cash, "现金");
			myInfo->freeTime = time(NULL) + 600;
			mcEnv->Treasury += myInfo->cash;
			myInfo->cash = 0;
			myInfo->mutex = 0;
			unloadData(myInfo, sizeof (mcUserInfo));
			unloadData(mcEnv, sizeof (MC_Env));
			pressreturn();
			Q_Goodbye();
		} else if (random() % 2) {
			if (show_cake()) {
				prints
				    ("\n      警官怒吼：“身份验证失败！跟我走一趟！”"
				     "\n      555，你被没收所有现金，并且被监禁10分钟。");
				//mcLog("玩777被警察监禁，损失", myInfo->cash, "现金");
				myInfo->freeTime = time(NULL) + 600;
				mcEnv->Treasury += myInfo->cash;
				myInfo->cash = 0;
				myInfo->mutex = 0;
				unloadData(myInfo, sizeof (mcUserInfo));
				unloadData(mcEnv, sizeof (MC_Env));
				pressreturn();
				Q_Goodbye();
			}
		}
		move(20, 0);
		prints("\n      警官点点头：“嗯，没错，继续玩吧。”");
	}
	pressanykey();
}
