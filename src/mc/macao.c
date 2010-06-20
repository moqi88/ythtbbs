#include "mc.h"

static int money_big();

// ------------------------  彩票  -------------------------- // 
//part one: 36_7 
static int
valid367Bet(char *buf)
{
	int i, j, temp[7], slot = 0;

	if (strlen(buf) != 20)	//  长度必须为20= 2 * 7 + 6 
		return 0;
	for (i = 0; i < 20; i += 3) {	//  基本格式必须正确 
		if (i + 2 != 20)
			if (buf[i + 2] != '-')	// 分隔符不正确 
				return 0;
		if (!isdigit(buf[i]) || !isdigit(buf[i + 1]))	// 不是数字 
			return 0;
		temp[slot] = (buf[i] - '0') * 10 + (buf[i + 1] - '0');
		if (temp[slot] > 36)
			return 0;
		slot++;
	}
	for (i = 0; i < 7; i++) {	// 数字无重复 
		for (j = 0; j < 7; j++)
			if (temp[j] == temp[i] && i != j)
				return 0;
	}
	return 1;
}

static int
parse367Bet(char *prizeSeq, char *bet, struct LotteryRecord *LR)
{
	int i, j, count = 0;
	int len = strlen(prizeSeq);

	if (strlen(bet) != len)
		return 0;
	for (i = 0; i + 1 < len; i = i + 3) {
		for (j = 0; j + 1 < len; j = j + 3) {
			if (bet[i] == prizeSeq[j]
			    && bet[i + 1] == prizeSeq[j + 1])
				count++;
		}
	}
	if (count >= 3 && count <= 7)
		LR->betCount[7 - count] = 1;
	return count;
}

static void
make367Seq(char *prizeSeq)
{
	int i, j, num, slot = 0, success;
	int temp[7];

	for (i = 0; i < 7; i++) {
		do {		//  数字不能相同 
			success = 1;
			num = 1 + rand() % 36;
			for (j = 0; j <= slot; j++) {
				if (num == temp[j]) {
					success = 0;
					break;
				}
			}
			if (success)
				temp[slot++] = num;
		}
		while (!success);
		prizeSeq[3 * i] = (char) (num / 10 + '0');
		prizeSeq[3 * i + 1] = (char) (num % 10 + '0');
		if (i != 6)
			prizeSeq[3 * i + 2] = '-';
		else
			prizeSeq[3 * i + 2] = '\0';
	}
	sprintf(genbuf, "数字组合是：  %s  。您中奖了吗？", prizeSeq);
	deliverreport("【彩票】本期36选7彩票摇奖结果", genbuf);
}

//part two: soccer 
static int
validSoccerBet(char *buf)
{
	int i, count = 0, meetSeperator = 1;
	int first = 0, second = 0;

	if (buf[0] == '\0')
		return 0;
	for (i = 0; i < strlen(buf); i++) {
		if (buf[i] == '-') {
			if (meetSeperator == 1)	//如果连续遇到-，肯定不正确 
				return 0;
			count = 0;
			meetSeperator = 1;
		} else {
			if (buf[i] != '3' && buf[i] != '1' && buf[i] != '0')
				return 0;
			count++;
			if (count > 3)
				return 0;
			if (count == 1) {
				first = buf[i];
			} else if (count == 2) {
				if (buf[i] == first)	//重合 
					return 0;
				second = buf[i];
			} else if (count == 3) {
				if (buf[i] == first || buf[i] == second)	//重合 
					return 0;
			}
			meetSeperator = 0;
		}
	}
	if (buf[strlen(buf) - 1] == '-')
		return 0;
	return 1;
}

static int
computeSum(char *complexBet)
{				//计算复式注的数量 
	int i, countNum = 0, total = 1;
	int len = strlen(complexBet);

	for (i = 0; i < len; i++) {
		if (complexBet[i] == '-') {
			total *= countNum;
			countNum = 0;
		} else
			countNum++;
	}
	total *= countNum;	// 再乘上最后一个单元 
	return total;
}

static int
makeSoccerPrize(char *bet, char *prizeSeq)
{
	int i, diff = 0;
	int n1 = strlen(bet);
	int n2 = strlen(prizeSeq);

	if (n1 != n2)
		return 10;	// 不中奖 
	for (i = 0; i < n1; i++) {
		if (bet[i] != prizeSeq[i])
			diff++;
	}
	return diff;
}

static void
parseSoccerBet(char *prizeSeq, char *complexBet, struct LotteryRecord *LR)
{				// 解析一个复式买注,保存奖励情况至LR 
	int i, j, simple = 1, meet = 0, count = 0;
	int firstDivEnd, firstDivStart;
	int len = strlen(complexBet);

	firstDivEnd = len;
	for (i = 1; i < len; i += 2) {
		if (complexBet[i] != '-') {
			simple = 0;
			break;
		}
	}
	if (simple) {		//简单标准形式 
		int diff;
		char buf[STRLEN];

		for (i = 0, j = 0; i < len; i++) {
			if (complexBet[i] != '-')
				buf[j++] = complexBet[i];
		}
		buf[j] = '\0';
		diff = makeSoccerPrize(prizeSeq, buf);
		if (diff <= 4 && diff >= 0)
			LR->betCount[diff]++;
	} else {
		for (i = 0; i < len; i++) {	//寻找第一个复式单元 
			if (complexBet[i] == '-') {
				if (count > 1 && !meet) {
					firstDivEnd = i;
					break;
				} else
					count = 0;
			} else
				count++;
		}
		firstDivStart = firstDivEnd - count;
		firstDivEnd--;

		for (i = 0; i < count; i++) {	//对每一个要拆分的单元的元素 
			int slot = 0;
			char temp[STRLEN];

//得到前面的部分 
			if (firstDivStart != 0) {
				for (j = 0; j < firstDivStart; j++, slot++)
					temp[slot] = complexBet[j];
			}
			temp[slot] = complexBet[firstDivStart + i];
			slot++;
//得到后面的部分 
			for (j = firstDivEnd + 1; j < len; j++, slot++) {
				temp[slot] = complexBet[j];
			}
			temp[slot] = '\0';
//对每一个拆分，进行递归调用 
			parseSoccerBet(prizeSeq, temp, LR);
		}

	}
}

//part three: misc 
static int
createLottery(int prizeMode)
{
	char buf[STRLEN];
	char lotteryDesc[][16] = { "-", "36选7彩票", "足彩" };
	int day;
	time_t *startTime = 0, *endTime = 0, currTime = time(NULL);

	if (prizeMode == 1) {
		startTime = &(mcEnv->start367);
		endTime = &(mcEnv->end367);
	} else {
		startTime = &(mcEnv->soccerStart);
		endTime = &(mcEnv->soccerEnd);
	}
	update_health();
	if (check_health(1, 12, 4, "您的体力不够了！", YEA))
		return 1;
	move(12, 4);
	if (currTime < *endTime) {
		prints("%s销售正在火热进行。", lotteryDesc[prizeMode]);
		pressanykey();
		return 1;
	}
	prints("新建%s", lotteryDesc[prizeMode]);
	while (1) {
		getdata(14, 4, "彩票销售天数[3-7]: ", buf, 2, DOECHO, YEA);
		day = atoi(buf);
		if (day >= 3 && day <= 7)
			break;
	}
	*startTime = currTime;
	*endTime = currTime + day * 86400;
	update_health();
	myInfo->health--;
	myInfo->Actived++;
	sprintf(genbuf, "本期彩票将于 %d 天后开奖。欢迎大家踊跃购买！", day);
	sprintf(buf, "【彩票】新一期%s开始销售", lotteryDesc[prizeMode]);
	deliverreport(buf, genbuf);
	showAt(15, 4, "建立成功！请到时开奖。", YEA);
	return 0;
}

static void
savePrizeList(int prizeMode, struct LotteryRecord LR,
	      struct LotteryRecord *totalCount)
{				//将中奖情况按userid保存至临时文件 

	FILE *fp;
	struct LotteryRecord *LR_curr;
	int i, n = 0, miss = 1;
	void *prizeListMem;

	if (prizeMode == 1)
		sprintf(genbuf, "%s", DIR_MC_TEMP "36_7_prizeList");
	else
		sprintf(genbuf, "%s", DIR_MC_TEMP "soccer_prizeList");
	n = get_num_records(genbuf, sizeof (struct LotteryRecord));
	prizeListMem = malloc(sizeof (struct LotteryRecord) * (n + 1));
	if (prizeListMem == NULL)
		return;
	memset(prizeListMem, 0, sizeof (struct LotteryRecord) * (n + 1));
	if (file_exist(genbuf)) {
		if ((fp = fopen(genbuf, "r")) == NULL) {
			free(prizeListMem);
			return;
		}
		fread(prizeListMem, sizeof (struct LotteryRecord), n, fp);
		fclose(fp);
	}
	for (i = 0; i < n; i++) {
		LR_curr = prizeListMem + i * sizeof (struct LotteryRecord);
		if (!strcmp(LR_curr->userid, LR.userid)) {	// 如果userid已经存在 
			for (i = 0; i < 5; i++)
				LR_curr->betCount[i] += LR.betCount[i];
			miss = 0;
			break;
		}
	}
	if (miss)		//userid记录不存在, add 
		memcpy(prizeListMem + n * sizeof (struct LotteryRecord), &LR,
		       sizeof (struct LotteryRecord));
	if ((fp = fopen(genbuf, "w")) == NULL) {
		free(prizeListMem);
		return;
	}
	n = miss ? (n + 1) : n;
	fwrite(prizeListMem, sizeof (struct LotteryRecord), n, fp);
	fclose(fp);
	free(prizeListMem);
// 全局统计累加 
	for (i = 0; i < 5; i++)
		totalCount->betCount[i] += LR.betCount[i];
	return;
}

static int
sendPrizeMail(struct LotteryRecord *LR, struct LotteryRecord *totalCount)
{
	int i, totalMoney, perPrize, myPrizeMoney;
	char title[STRLEN], buf[128];
	char *prizeName[] = { "NULL", "36选7", "足彩", NULL };
	char *prizeClass[] = { "特等", "一等", "二等", "三等", "安慰", NULL };
	float prizeRate[] = { 0.60, 0.20, 0.10, 0.05, 0.02 };
	int prizeMode = strcmp(totalCount->userid, "36_7") == 0 ? 1 : 2;
	mcUserInfo *mcuInfo;

	sethomefile(buf, LR->userid, "mc.save");
	if (!file_exist(buf))
		initData(1, buf);
	if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
		return -1;
	if (prizeMode == 1)
		totalMoney = mcEnv->prize367 + PRIZE_PER;
	else
		totalMoney = mcEnv->prizeSoccer + PRIZE_PER;
	for (i = 0; i < 5; i++) {	// 对每个中奖的userid,检查各种奖励 
		if (LR->betCount[i] > 0 && totalCount->betCount[i] > 0) {
			perPrize =
			    prizeRate[i] * totalMoney / totalCount->betCount[i];
			myPrizeMoney = perPrize * LR->betCount[i];
			mcuInfo->cash += myPrizeMoney;
			mcEnv->Treasury -= myPrizeMoney;
			sprintf(genbuf,
				"您一共中了 %d 注,得到了 %d %s的奖金。恭喜！",
				LR->betCount[i], myPrizeMoney, MONEY_NAME);
			sprintf(title, "恭喜您获得%s%s奖！",
				prizeName[prizeMode], prizeClass[i]);
			if (myInfo->GetLetter == 1)
			    system_mail_buf(genbuf, strlen(genbuf), LR->userid,
				        	    title, currentuser->userid);
		}
	}
	unloadData(mcuInfo, sizeof (mcUserInfo));
	return 0;
}

static int
doOpenLottery(int prizeMode, char *prizeSeq)
{
	FILE *fp;
	char line[256], buf[STRLEN], title[STRLEN];
	char *bet, *userid;
	int totalMoney, remainMoney, i;
	struct LotteryRecord LR, totalCount;
	char *prizeName[] = { "NULL", "36选7", "足彩", NULL };
	char *prizeClass[] = { "特等", "一等", "二等", "三等", "安慰", NULL };
	float prizeRate[] = { 0.60, 0.20, 0.10, 0.05, 0.02 };

	if (prizeMode == 1) {
		make367Seq(prizeSeq);	//产生序列 
		totalMoney = mcEnv->prize367 + PRIZE_PER;
		fp = fopen(DIR_MC "36_7_list", "r");
	} else {
		totalMoney = mcEnv->prizeSoccer + PRIZE_PER;
		fp = fopen(DIR_MC "soccer_list", "r");
	}
	totalMoney = MIN(totalMoney, MAX_POOL_MONEY);
	if (fp == NULL)
		return -1;
//   ---------------------计算奖励----------------------- 

	memset(&totalCount, 0, sizeof (struct LotteryRecord));	// 总计初始化 
	while (fgets(line, 255, fp)) {
		userid = strtok(line, " ");
		bet = strtok(NULL, "\n");
		if (!userid || !bet) {
			continue;
		}
		memset(&LR, 0, sizeof (struct LotteryRecord));
		strcpy(LR.userid, userid);
// 解析买注,中奖情况保存在LR中 
		if (prizeMode == 1)
			parse367Bet(prizeSeq, bet, &LR);
		else
			parseSoccerBet(prizeSeq, bet, &LR);
		for (i = 0; i < 5; i++) {	// 检查是否中奖 
			if (LR.betCount[i] > 0) {
				savePrizeList(prizeMode, LR, &totalCount);
				break;
			}
		}
	}
	fclose(fp);
//  ------------------------ 发奖 --------------------- 
	remainMoney = totalMoney;
	if (prizeMode == 1) {
		sprintf(genbuf, "%s", DIR_MC_TEMP "36_7_prizeList");
		strcpy(totalCount.userid, "36_7");
	} else {
		sprintf(genbuf, "%s", DIR_MC_TEMP "soccer_prizeList");
		strcpy(totalCount.userid, "soccer_");

// 遍历前面生成的中奖文件,给每个中奖ID发奖. 每个ID是一个记录 
// 同时总计累加 
		new_apply_record(genbuf, sizeof (struct LotteryRecord),
				 (void *) sendPrizeMail, &totalCount);

//  ---------------------- 版面通知 --------------------- 
		for (i = 0; i < 5; i++) {
			if (totalCount.betCount[i] > 0) {
				sprintf(title, "【彩票】本期%s%s奖情况揭晓",
					prizeName[prizeMode], prizeClass[i]);
				sprintf(buf, "共计注数: %d\n单注奖金: %d",
					totalCount.betCount[i],
					(int) (prizeRate[i] * totalMoney /
					       totalCount.betCount[i]));
				deliverreport(title, buf);
				remainMoney -= totalMoney * prizeRate[i];
			}
		}
	}
// 清扫战场 
	if (prizeMode == 1) {
		mcEnv->prize367 = remainMoney;
		unlink(DIR_MC "36_7_list");
		unlink(DIR_MC_TEMP "36_7_prizeList");
	} else {
		mcEnv->prizeSoccer = remainMoney;
		unlink(DIR_MC "soccer_list");
		unlink(DIR_MC_TEMP "soccer_prizeList");
	}
	return 0;
}

static int
tryOpenPrize(int prizeMode)
{
	char buf[STRLEN];
	int flag;
	time_t startTime, endTime;

	update_health();
	if (check_health(1, 12, 4, "您的体力不够了！", YEA))
		return 1;
	if (prizeMode == 1) {
		startTime = mcEnv->start367;
		endTime = mcEnv->end367;
	} else {
		startTime = mcEnv->soccerStart;
		endTime = mcEnv->soccerEnd;
	}
	if (startTime == 0) {
		showAt(t_lines - 5, 4, "没有找到该彩票的记录...", YEA);
		return -1;
	}
	if (time(NULL) < endTime) {
		showAt(t_lines - 5, 4, "还没有到开奖的时间啊!", YEA);
		pressanykey();
		return -1;
	}
	if (prizeMode == 1)
		buf[0] = '\0';
	else {
		getdata(t_lines - 5, 4,
			"请输入兑奖序列(无需 - )[按\033[1;33mENTER\033[m放弃]: ",
			buf, 55, DOECHO, YEA);
		if (buf[0] == '\0')
			return 0;
	}
	flag = doOpenLottery(prizeMode, buf);
	move(t_lines - 4, 4);
	if (flag == 0)
		prints("开奖成功！");
	else
		prints("发生意外错误...");
	update_health();
	myInfo->health--;
	myInfo->Actived++;
	pressanykey();
	return 0;
}

static int
buyLottery(int type)
{
	int needMoney, perMoney = 1000;
	int retv, maxBufLen, num, trytime = 0;
	int *poolMoney;
	char letter[128], buf[128], filepath[256];
	time_t startTime, endTime;
	char *desc[] = { "36选7", "足球" };

	money_show_stat("彩票窗口");
	if (check_health(1, 12, 4, "您的体力不够了！", YEA))
		return 0;
	if (type == 0) {	//36选7 
		sprintf(filepath, "%s", DIR_MC "36_7_list");
		maxBufLen = 21;
		poolMoney = &(mcEnv->prize367);
		startTime = mcEnv->start367;
		endTime = mcEnv->end367;
	} else {
		sprintf(filepath, "%s", DIR_MC "soccer_list");
		maxBufLen = 61;
		poolMoney = &(mcEnv->prizeSoccer);
		startTime = mcEnv->soccerStart;
		endTime = mcEnv->soccerEnd;
	}
	if (startTime == 0) {
		sprintf(buf, "抱歉，新一期的%s彩票还未开始销售。", desc[type]);
		showAt(4, 4, buf, YEA);
		return 0;
	}
	if (time(NULL) >= endTime) {
		showAt(4, 4, "抱歉，本期彩票销售期已经结束。请等待开奖。", YEA);
		return 0;
	}
	if (type == 0)
		showAt(4, 4, "数字间用-隔开，例如 08-13-01-25-34-17-18", NA);
	else
		showAt(4, 0,
		       "    主场胜/平/负分别为为3/1/0。各场比赛用-隔开。\n"
		       "    支持复式买注。例如： 1-310-1-01-3-30", NA);
	move(7, 4);
	prints("当前奖金池：\033[1;31m%d\033[m   固定奖金：\033[1;31m%d\033[m",
	       *poolMoney, PRIZE_PER);
	sprintf(genbuf, "每注 %d %s。确定买注吗", perMoney, MONEY_NAME);
	move(9, 4);
	if (askyn(genbuf, NA, NA) == NA)
		return 0;
	while (1) {
		getdata(10, 4, "请填写买注单: ", buf, maxBufLen, DOECHO, YEA);
		if (type == 0)
			retv = valid367Bet(buf);
		else
			retv = validSoccerBet(buf);
		if (retv == 0) {
			showAt(11, 4, "对不起，您的下注单填写得不对喔。", YEA);
			if (trytime++ == 2)
				break;
			continue;
		}
		if (type == 0) {
			num = 1;
			needMoney = perMoney;
		} else {
			num = computeSum(buf);
			needMoney = num * perMoney;
		}
		if (myInfo->cash < needMoney) {
			showAt(11, 4, "对不起，您的钱不够。", YEA);
			return 0;
		}
		myInfo->cash -= needMoney;
		*poolMoney += needMoney;
		update_health();
		myInfo->health--;
		myInfo->Actived++;
		sprintf(genbuf, "%s %s", currentuser->userid, buf);
		if (type == 0)
			addtofile(DIR_MC "36_7_list", genbuf);
		else
			addtofile(DIR_MC "soccer_list", genbuf);
		sprintf(letter, "您购买了一注%s彩票。注号是：%s。", desc[type],
			buf);
		if (myInfo->GetLetter == 1)
		    system_mail_buf(letter, strlen(letter), currentuser->userid,
				            "彩票中心购买凭证", currentuser->userid);
		clrtoeol();
		sprintf(buf, "成功购买 %d 注%s彩票 。祝您中大奖！", num,
			desc[type]);
		sleep(1);
		showAt(11, 4, buf, YEA);
		break;
	}
	return 1;
}

int
money_lottery()
{
	char ch, quit = 0, quitRoom = 0;
	char uident[IDLEN + 1];

       if (!(myInfo->GetLetter == 1)) {
           clear();
           showAt(5,4,"你已经关闭了金融中心游戏功能，请开启后再来。",YEA);
           return 0;
       }

	while (!quit) {
		nomoney_show_stat("彩票中心");
		showAt(4, 4, "目前发行两种彩票: 36选7和足球彩票。欢迎购买！",
		       NA);
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m 选单 \033[1;46m [1]36选7 [2]足彩 [3]经理室 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
		case '2':
			buyLottery(ch - '1');
			break;
		case '3':
			nomoney_show_stat("博彩公司经理室");
			whoTakeCharge(2, uident);
			if (strcmp(currentuser->userid, uident))
				break;
			quitRoom = 0;
			while (!quitRoom) {
				nomoney_show_stat("博彩公司经理室");
				move(t_lines - 1, 0);
				move(6, 10);
				prints("建立彩票:    1.  36选7    2.足球彩票");
				move(7, 10);
				prints
				    ("开    奖:    3.  36选7    4.足球彩票    Q.  退出");
				move(10, 4);
				prints("请选择要操作的代号:");
				ch = igetkey();
				switch (ch) {
				case '1':
				case '2':
					createLottery(ch - '0');
					break;
				case '3':
				case '4':
					tryOpenPrize(ch - '2');
					break;
				case 'q':
				case 'Q':
					quitRoom = 1;
					break;
				}
			}
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

// --------------------------    赌场    ----------------------- // 
static int
money_dice()
{
	int i, ch, quit = 0, target, win, num, sum;
	unsigned int t[3];

	while (!quit) {
		money_show_stat("赌场骰宝厅");
		move(4, 0);
		prints("    分大小两门，4-10点是小，11-17点为大。\n"
		       "    若押小开小，可拿一倍彩金，押大的就全归庄家。\n"
		       "    庄家要是摇出全骰（三个骰子点数一样）则通吃大小家。\n"
		       "    \033[1;31m多买多赚，少买少赔，买定离手，愿赌服输!\033[m");
		move(t_lines - 1, 0);
		prints("\033[1;44m 选单 \033[1;46m [1]下注 [2]VIP [Q]离开\033[m");
		win = 0;
		ch = igetkey();
		switch (ch) {
		case '1':
			update_health();
			if(mcEnv->prize777 < 10000){                 //777 10000 保底资金
		         showAt(11, 4,"赌场现金余额过低，暂停营业！",YEA);
		         break;
			}
			if (check_health(1, 12, 4, "您的体力不够了！", YEA))
				break;
			num =
			    userInputValue(9, 4, "压", MONEY_NAME, 1000,
					   200000);
			if (num == -1)
				break;
			getdata(11, 4, "压大(L)还是小(S)？[L]", genbuf, 3,
				DOECHO, YEA);
			if (genbuf[0] == 'S' || genbuf[0] == 's')
				target = 1;
			else
				target = 0;
			sprintf(genbuf,
				"买 \033[1;31m%d\033[m %s的 \033[1;31m%s\033[m，确定么？",
				num, MONEY_NAME, target ? "小" : "大");
			move(12, 4);
			if (askyn(genbuf, NA, NA) == NA)
				break;
			move(13, 4);
			if (myInfo->cash < num) {
				showAt(13, 4, "去去去，没那么多钱捣什么乱！",
				       YEA);
				break;
			}
			myInfo->cash -= num;
			myInfo->health--;
			if(!random()%5)
				myInfo->luck--;
			myInfo->Actived += 2;
			for (i = 0; i < 3; i++) {
				getrandomint(&t[i]);
				t[i] = t[i] % 6 + 1;
			}
			sum = t[0] + t[1] + t[2];
			if ((t[0] == t[1]) && (t[1] == t[2])) {
				mcEnv->prize777 += after_tax(num);
				sprintf(genbuf, "\033[1;32m庄家通杀！\033[m");
			} else if (sum <= 10) {
				sprintf(genbuf, "%d 点，\033[1;32m小\033[m",
					sum);
				if (target == 1)
					win = 1;
			} else {
				sprintf(genbuf, "%d 点，\033[1;32m大\033[m",
					sum);
				if (target == 0)
					win = 1;
			}
			sleep(1);
			prints("开了开了~~  %d %d %d  %s", t[0], t[1], t[2],
			       genbuf);
			move(14, 4);
			if (win) {
				myInfo->cash += 2 * num;
				mcEnv->prize777 -= num;
				prints("恭喜您，再来一把吧！");
			} else {
				mcEnv->prize777 += after_tax(num);
				prints("没有关系，先输后赢...");
			}
			pressanykey();
			break;
		case '2':
			money_big();
			break;
		case 'Q':
		case 'q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int
calc777(int t1, int t2, int t3)
{
	if ((t1 == 13) && (t2 == 13) && (t3 == 13))
		return 81;
	if ((t1 % 2 == 0) && (t2 % 2 == 0) && (t3 % 2 == 0))
		return 2;
	if ((t1 % 2 == 0) && (t2 % 2 == 0) && (t3 % 6 == 1) && (t3 !=  13))
		return 3;
	if ((t1 % 2 == 0) && (t2 % 6 == 1) && (t3 % 6 == 1) && (t2 !=  13) && (t3 !=  13))
		return 4;
	if ((t1 % 6 == 1) && (t2 % 6 == 1) && (t3 % 2 == 0) && (t1 !=  13) && (t2 !=  13))
		return 4;
	if ((t1 % 2 == 0) && (t2 % 6 == 3) && (t3 % 6 == 3))
		return 6;
	if ((t1 % 6 == 3) && (t2 % 6 == 3) && (t3 % 2 == 0))
		return 6;
	if ((t1 % 6 == 1) && (t2 % 6 == 1) && (t3 % 6 == 1) && (t1 !=  13) && (t2 !=  13) && (t3 !=  13))
		return 11;
	if ((t1 % 6 == 3) && (t2 % 6 == 3) && (t3 % 6 == 3))
		return 21;
	if ((t1 % 6 == 5) && (t2 % 6 == 5) && (t3 % 6 == 5))
		return 41;
	if ((t1 % 6 == 5) && (t2 == 13) && (t3 == 13))
		return 61;
	return 0;
}

#if 0
static int
get_addedlevel(int level, int base, int num)
{
	if (num - (level + 1) * (level + 1) * base >
	    (level + 2) * (level + 2) * base) {
		level++;
		level = get_addedlevel(level, base, num - level * level * base);
	}
	return level;
}
#endif

static int
money_777()
{
	int i, ch, quit = 0, bid, winrate, num = 0, tax = 0;
	unsigned int t[3];
	char n[15] = "-R-B-6-R-B-6-7";
	char title[STRLEN], buf[256];
	time_t timeA,timeB;

	while (!quit) {
		money_show_stat("赌场777");
		if (mcEnv->prize777 < 0) {
			mcEnv->Treasury += mcEnv->prize777;
			mcEnv->prize777 = 0;
		}
		if (check_health(1, 12, 4, "您的体力不够了！", YEA))
			break;
		/*if (!(random() % (MAX_ONLINE/50)) && myInfo->Actived > MAX_ONLINE/5 ) {
			clear();
			move(4, 4);
			prints("突然之间，大富翁世界剧烈的旋转起来。。。");
			pressanykey();
			randomevent();
		}*/
		if (!(random() % (MAX_ONLINE/10)) || myInfo->luck <= -90) {
			move(5, 8);
			prints("=============== 警 官 巡 视 ============");
			move(8, 4);
			prints
			    ("  最近777赌场劫案频发，所以经常会有警官进来巡视，盘查可疑目标。"
			     "\n  啊呀，不好！警官向你走了过来。");
			pressanykey();
			move(10, 4);
			if (random() % 2) {
				prints
					("还好还好，警官目不斜视的从你身边走过去了。");
			}
/*			
      else {
				prints
				    ("警官上下打量了你一眼，慢条斯理的说：“需要身份确认！”");
				getdata(11, 4, "请输入密码(只有一次机会): ",
					passbuf, PASSLEN, NOECHO, YEA);
				if (passbuf[0] == '\0' || passbuf[0] == '\n'
				    || !checkpasswd(currentuser->passwd,
						    currentuser->salt,
						    passbuf)) {
					prints
					    ("\n    警官怒吼：“身份验证失败！跟我走一趟！”"
					     "\n    555，你被没收所有现金，并且被监禁30分钟。");
					myInfo->freeTime = time(NULL) + 1800;
					mcEnv->Treasury += myInfo->cash;
					myInfo->cash = 0;
					myInfo->mutex = 0;
					unloadData(myInfo,sizeof (mcUserInfo));
					unloadData(mcEnv, sizeof (MC_Env));
					pressreturn();
					Q_Goodbye();
				} 
*/
			  else  {
				    timeA = time(NULL);
					if (show_cake()) {
						prints
						    ("\n    警官怒吼：“身份验证失败！跟我走一趟！”"
						     "\n    555，你被没收所有现金，并且被监禁30分钟。");
						myInfo->freeTime = time(NULL) + 1800;
						mcEnv->Treasury += myInfo->cash;
						myInfo->cash = 0;
						myInfo->mutex = 0;
						unloadData(myInfo,sizeof (mcUserInfo));
						unloadData(mcEnv, sizeof (MC_Env));
						pressreturn();
						Q_Goodbye();
					}
					timeB = time(NULL);
					if (timeA + 120 < timeB) {
						prints
						    ("\n    警官怒吼：“这么简单的问题都想这么久？机器人吧！”"
						     "\n    555，你被没收所有现金，并且被监禁30分钟。");
						myInfo->freeTime = time(NULL) + 1800;
						mcEnv->Treasury += myInfo->cash;
						myInfo->cash = 0;
						myInfo->mutex = 0;
						unloadData(myInfo,sizeof (mcUserInfo));
						unloadData(mcEnv, sizeof (MC_Env));
						pressreturn();
						Q_Goodbye();
					}
					move(20, 0);
			        prints("\n    警官点点头：“嗯，没错，继续玩吧。”");
			        pressanykey();
				}			
			break;
		}

		move(6, 4);
		prints("--R 1:2    -RR 1:3    RR- 1:3    -BB 1:5    BB- 1:5");
		move(7, 4);
		prints("RRR 1:10   BBB 1:20   666 1:40   677 1:60   --- 1:1");
		move(8, 4);
		prints("777 1:80 (有机会赢得当前累计基金的一半，最多不超过100万)");
		move(9, 4);
		prints
		    ("目前累积奖金数: %d  想赢大奖么？压200就有机会喔。",
		     mcEnv->prize777);
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m 选单 \033[1;46m [1] 压50 [2] 压200 [3] 注入奖金 [Q]离开\033[m");
		ch = igetkey();
		if (ch == 'q' || ch == 'Q')
			break;
		if (ch == '1') {
			if (mcEnv->prize777 < 10000) {
				showAt(11, 4,
				       "目前777奖金殆尽，请等待注入奖金。",
				       YEA);
				return 0;
			}
			bid = 50;
		} else if (ch == '2') {
			if (mcEnv->prize777 < 10000) {
				showAt(11, 4,
				       "目前777奖金殆尽，请等待注入奖金。",
				       YEA);
				return 0;
			}
			bid = 200;
		} else if (ch == '3') {
			if (myInfo->cash < 10000) {
				showAt(11, 4, "你带的现金太少了，cmft", YEA);
				continue;
			}
			num =
			    userInputValue(11, 4, "注入", MONEY_NAME, 10000,
					   myInfo->cash);
			if (num <= 0)
				continue;
			myInfo->cash -= num;
			mcEnv->prize777 += after_tax(num);
			myInfo->luck = MIN(100, myInfo->luck + num / 10000);
			showAt(15, 4, "注入奖金成功！", YEA);
			continue;
		} else
			continue;
		if (myInfo->cash < bid) {
			showAt(11, 4, "没钱就别玩了...", YEA);
			continue;
		}
		myInfo->cash -= bid;
		update_health();
		myInfo->health -= 2;
		myInfo->Actived += 2;
		for (i = 0; i < 3; i++) {
			getrandomint(&t[i]);
			t[i] = t[i] % 14;
			move(11, 20 + 2 * i);
			prints("%c", n[t[i]]);
			refresh();
			sleep(1);
		}
		winrate = calc777(t[0], t[1], t[2]);
		if (winrate <= 0) {
			mcEnv->prize777 += after_tax(bid);
			showAt(12, 4,
			       "输了，赌注流入累积基金，造福他人等于造福自己。",
			       YEA);
			continue;
		}
		mcEnv->prize777 -= MAX(bid * (winrate - 1),0);
		myInfo->cash += MAX(bid * winrate, 0);
		move(12, 4);
		prints("您赢了 %d %s", bid * (winrate - 1), MONEY_NAME);
		if (winrate == 81 && bid == 200) {
			num = MIN(mcEnv->prize777 / 2, 1000000) * (myInfo->luck + 100)/200;	//777奖金跟人品挂钩
//			num = MAX(num, random() % (myInfo->luck + 101) * 5000 );
			tax = num - after_tax(num);
			myInfo->cash += num - tax;
			mcEnv->prize777 -= num;
			move(12, 4);
			prints("\033[5;33m恭喜您获得大奖！别忘了要缴税啊！\033[0m");
			sprintf(title, "【赌场】%s 赢得777大奖！",
				currentuser->userid);
			sprintf(buf, "赌场传来消息：%s 赢得777大奖 %d %s!\n扣税后实得 %d %s",
				currentuser->userid, num, MONEY_NAME, 
				num - tax, MONEY_NAME);
			deliverreport(title, buf);
		}
		pressanykey();
	}
	return 0;
}

static int
money_big()
{
	int i, ch, quit = 0, target, win, num, sum;
	unsigned int t[3];
	char buf[256];

	if(mcEnv->Treasury < 15000000){                 //大户室1500w 保底资金
		showAt(13, 4,
		       "国库现金余额过低，大户室暂停营业！”",
		       YEA);
		return 0;
	}
	if (myInfo->cash < 200000) {
		showAt(13, 4,
		       "门卫斜眼瞅你一眼，伸手把你拦住：“一副穷酸相也想进大户密室？大你个头大！！”",
		       YEA);
		return 0;
	}
	move(7, 4);
	sprintf(buf, "入场费\033[1;32m10万\033[m%s，你确定要进去吗？",
		MONEY_NAME);
	if (askyn(buf, NA, NA) == NA) {
		prints("\n拜拜，小气鬼～");
		return 0;
	}
	myInfo->cash -= 100000;
	mcEnv->Treasury += 100000;
	clear();
	money_show_stat("赌场大户密室");
	move(4, 0);
	prints
	    ("    这里是大户密室，这里的赌注比外面的更要大，所以更加的刺激：）\n"
	     "    这地方一般有钱有权的才能进的来，赶紧押吧！\n"
	     "    哦，对了，赢了的话要抽取10％的小费的。");
	pressanykey();

	while (!quit) {
		clear();
		money_show_stat("赌场大户密室");
		move(4, 0);
		prints("    骰宝。简介：分大小两门，4-10点是小，11-17点为大。\n"
		       "    若押小开小，可拿一倍彩金，押大的就全归庄家。\n"
		       "    庄家要是摇出全骰（三个骰子点数一样）则通吃大小家。\n"
		       "    \033[1;31m多买多赚，少买少赔，买定离手，愿赌服输!\033[m");
		move(t_lines - 1, 0);
		prints("\033[1;44m 选单 \033[1;46m [1]下注 [Q]离开\033[m");
		win = 0;
		ch = igetkey();
		switch (ch) {
		case '1':
			update_health();
			if (check_health(3, 12, 4, "您的体力不够了！", YEA))
				break;
			sprintf(buf, "\033[1;32m万\033[m%s", MONEY_NAME);
			num = userInputValue(9, 4, "压", buf, 20, 1000);
			if (num == -1)
				break;
			getdata(11, 4, "压大(L)还是小(S)？[L]", genbuf, 3,
				DOECHO, YEA);
			if (genbuf[0] == 'S' || genbuf[0] == 's')
				target = 1;
			else
				target = 0;
			if(mcEnv->Treasury < 15000000){                 //大户室1500w 保底资金
	        	showAt(13, 4,"国库现金余额过低，大户室暂停营业！”",YEA);
		        return 0;
			}
			if(mcEnv->Treasury - 15000000 < num*10000){
				num = mcEnv->Treasury/10000 - 1500;
				sprintf(genbuf,
					"国库现金不足，你只能买 \033[1;31m%d\033[m \033[1;32m万\033[m%s的 \033[1;31m%s\033[m，确定么？",
					num, MONEY_NAME, target ? "小" : "大");
			}
			else{
				sprintf(genbuf,
					"买 \033[1;31m%d\033[m \033[1;32m万\033[m%s的 \033[1;31m%s\033[m，确定么？",
					num, MONEY_NAME, target ? "小" : "大");
				}
			move(12, 4);
			if (askyn(genbuf, NA, NA) == NA)
				break;
			move(13, 4);
			if (myInfo->cash < (num * 10000)) {
				showAt(13, 4, "去去去，没那么多钱捣什么乱！",
				       YEA);
				break;
			}
			myInfo->cash -= (num * 10000);
			myInfo->health -= 3;
			myInfo->luck--;
			myInfo->Actived += 3;
			for (i = 0; i < 3; i++) {
				getrandomint(&t[i]);
				t[i] = t[i] % 6 + 1;
			}
			sum = t[0] + t[1] + t[2];
			if ((t[0] == t[1]) && (t[1] == t[2])) {
				mcEnv->Treasury += 10000 * num;
				sprintf(genbuf, "\033[1;32m庄家通杀！\033[m");
			} else if (sum <= 10) {
				sprintf(genbuf, "%d 点，\033[1;32m小\033[m",
					sum);
				if (target == 1)
					win = 1;
			} else {
				sprintf(genbuf, "%d 点，\033[1;32m大\033[m",
					sum);
				if (target == 0)
					win = 1;
			}
			sleep(1);
			prints("开了开了~~  %d %d %d  %s", t[0], t[1], t[2],
			       genbuf);
			move(14, 4);
			if (win) {
				myInfo->cash += 19000 * num;
				mcEnv->Treasury -= 9000 * num;
				prints("恭喜您，抽取小费%d%s~~ 再来一把吧！",
				       1000 * num, MONEY_NAME);
				sprintf(buf, "今日赌场快讯，%s刚刚从%s赌场赢走了%d万%s。",
					    currentuser->userid,CENTER_NAME,num,MONEY_NAME);
				deliverreport("【新闻】幸运儿赌场一夜暴富", buf);
			} else{
				mcEnv->Treasury += 10000 * num;
				prints("没有关系，先输后赢，至少不用给小费了...");
				sprintf(buf, "今日赌场快讯，%s刚刚从%s赌场输掉了%d万%s。\n专家提醒：赌博有害健康!",
					    currentuser->userid,CENTER_NAME,num,MONEY_NAME);
				deliverreport("【新闻】倒霉蛋街头落魄", buf);
			}
			pressanykey();
			break;
		case 'Q':
		case 'q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;

}

/*
static int
game_prize()
{
	int quit = 0;
	char ch;
	while (!quit) {
		money_show_stat("抽奖处");
		move (4, 0);
		prints ("如果您玩游戏进入高手排行榜，可以在这里抽奖。一次上榜记录只能抽一次奖。");
		move (t_lines - 1, 0);
		prints
		    ("\033[1;37;44m 选单 \033[1;46m [1]推箱子 [2]扫雷 [3]感应式扫雷 [4]俄罗斯方块 [5]打字 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			choujiang(MY_BBS_HOME "/etc/worker/worker.rec", "推箱子");
			break;
		case '2':
			choujiang(MY_BBS_HOME "/etc/winmine/mine2.rec", "扫雷");
			break;
		case '3':
			choujiang(MY_BBS_HOME "/etc/winmine2/mine3.rec",
				  "感应式扫雷");
			break;
		case '4':
			choujiang(MY_BBS_HOME "/etc/tetris/tetris.rec",
				  "俄罗斯方块");
			break;
		case '5':
			choujiang(MY_BBS_HOME "/etc/tt/tt.dat", "打字练习");
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	return 0;
}

static void
choujiang(char *recfile, char *gametype)
{
	int n, topS[20], topT[20], chou, jiang, tax;
	char topID[20][20], topFROM[20][20], prize[20][3], buf[256],
	    title[STRLEN];
	FILE *fp;

	if ( (fp = fopen(recfile, "r")) == NULL)
		return;
	chou = 0;
	if (!strcmp(gametype, "推箱子")){
		for (n = 0; n <= 19; n++){
			fscanf(fp, "%s %d %d %s %s\n",
				topID[n], &topS[n], &topT[n], topFROM[n], prize[n]);
			if (!strcmp(currentuser->userid, topID[n])
				&& !strcmp(prize[n], "未")){
				chou = n + 1;
			}
		}
	}else{
		for (n = 0; n <= 19; n++) {
			fscanf(fp, "%s %d %s %s\n", topID[n], &topT[n], topFROM[n],
		       		prize[n]);
			if (!strcmp(currentuser->userid, topID[n])
			    && !strcmp(prize[n], "未")) {
				chou = n + 1;
			}
		}
	}
	fclose(fp);
	
	if (chou == 0) {
		showAt(6, 4, "\033[1;37m您目前无法抽奖\033[m", YEA);
		return;
	}
	move(6, 4);
	prints("\033[1;37m您是 %s 第 %d 名\033[m", gametype, chou);
	move(7, 4);
	if (askyn("要抽奖吗？", YEA, NA) == YEA) {
		jiang = 0;
		if (chou == 1)
			jiang = random() % 500 + 500;
		if (chou > 1 && chou <= 5)
			jiang = random() % 500 + 100;
		if (chou > 5 && chou <= 10)
			jiang = random() % 190 + 10;
		if (chou > 10 && chou <= 15)
			jiang = random() % 95 + 5;
		if (chou > 15 && chou <= 20)
			jiang = random() % 49 + 1;
		jiang = jiang * 1000;
		
		if (mcEnv->Treasury > jiang + 10000){
			tax = jiang - after_tax(jiang);
			mcEnv->Treasury -= jiang - tax;
			myInfo->cash += jiang - tax;
			strcpy(prize[chou-1], "抽");
			myInfo->Actived += 2;
			sprintf(buf, "\033[1;32m您抽中了 %d %s，扣税后实得 %d %s\033[m", 
				jiang, MONEY_NAME, jiang - tax, MONEY_NAME);
			showAt(8, 4, buf, YEA);
			sprintf(title, "【抽奖】%s 抽中了 %d %s", 
				currentuser->userid, jiang, MONEY_NAME);
			sprintf(buf, "%s 玩 %s 得了第 %d 名，\n抽奖抽到 %d %s，\n扣税后实得 %d %s",
				currentuser->userid, gametype, chou, jiang, MONEY_NAME, 
				jiang - tax, MONEY_NAME);
			deliverreport(title, buf);
		}else{
			showAt(8, 4, "奖金库没钱了！等会再抽吧。", YEA);
		}
		fp = fopen(recfile, "w");
		if (!strcmp(gametype, "推箱子")){
			for (n = 0; n <= 19; n++)
				fprintf(fp, "%s %d %d %s %s\n", 
					topID[n], topS[n], topT[n], topFROM[n], prize[n]);
		}else{
			for (n = 0; n <= 19; n++){
				fprintf(fp, "%s %d %s %s\n", 
					topID[n], topT[n], topFROM[n], prize[n]);
			}
		}
		fclose(fp);
	} else {
		move(8, 4);
		prints("\033[1;32m您不想抽奖了\033[m");
	}
	return;
}


//先买定义每次游戏花费_UNIT，开始后出现3行5列的图案。
//共有5根线：(0,0)-(0,1)-(0,2)-(0,3)-(0,4)，(0,0)-(1,1)-(2,2)-(1,3)-(0,4)
//           (1,0)-(1,1)-(1,2)-(1,3)-(1,4)
//           (2,0)-(1,1)-(0,2)-(1,3)-(2,4)，(2,0)-(2,1)-(2,2)-(2,3)-(2,4)
//如果前3，4，5个是一样的图案，就会奖励，赔率在DIR_MC "monkey_rate"
//不同图案赔率不一样，○可以代替任何图案，人和猴子不参与这些组合。
//如果前3，4，5列出现人，获得10次免费游戏。
//免费游戏中如果第三列出现猴子，台面奖金×(人数目-1)
static void
monkey_business(void)
{
	int i,j,MONKEY_UNIT, TempMoney=0, FreeGame=0, multi=0, quit=0, total=0, man=0;
	char ch,
	     pic[12][2] ={"Ａ","Ｋ","Ｑ","Ｊ","10","象",
		          "蛇","鸟","狮","○","人","猴"};
	int pattern[3][5], sum[5]={12345,12345,12345,12345,12345}, winrate[100000];
	FILE *fp;

	MONKEY_UNIT = userInputValue(6, 4, "每次游戏花费",MONEY_NAME, 250, 25000);
	if(MONKEY_UNIT < 0) {
		showAt(8, 4, "不玩别捣乱！", YEA);
		return;
	}
	//mcLog("进入MONKEY，每次花费", MONKEY_UNIT, "");
	if (myInfo->cash < MONKEY_UNIT) {
		showAt(8, 4, "没钱就别赌了。", YEA);
		return;
	}

	fp = fopen(DIR_MC "monkey_rate", "r");
	for(i=0;i<100000;i++)
		fscanf(fp,"%d\n",&winrate[i]);
	fclose(fp);									

	while(!quit)
	{
		if (!(random() % MAX_ONLINE/5) && FreeGame <= 0)
			policeCheck();
		
		clear();
		money_show_stat("MONKEY BUSINESS");
		move(5, 0);
		prints("      --== 游戏规则 ==--      |\n");
		prints("                              |\n");
		prints("在以下5条线中：               |\n");
		prints("(1,1)\033[1;31m-\033[0;37m(1,2)\033[1;31m-\033[0;37m(1,3)"
			"\033[1;31m-\033[0;37m(1,4)\033[1;31m-\033[0;37m(1,5) |\033[m\n");
		prints("     \033[1;32m╲\033[0;37m   \033[1;35m╱\033[0;37m     "
			"\033[1;35m╲\033[0;37m   \033[1;32m╱\033[0;37m      |\033[m\n");
		prints("(2,1)\033[1;33m-\033[0;37m(2,2)\033[1;33m-\033[0;37m(2,3)"
			"\033[1;33m-\033[0;37m(2,4)\033[1;33m-\033[0;37m(2,5) |\033[m\n");
		prints("     \033[1;35m╱\033[0;37m   \033[1;32m╲\033[0;37m     "
			"\033[1;32m╱\033[0;37m   \033[1;35m╲\033[0;37m      |\033[m\n");
		prints("(3,1)\033[1;36m-\033[0;37m(3,2)\033[1;36m-\033[0;37m(3,3)"
			"\033[1;36m-\033[0;37m(3,4)\033[1;36m-\033[0;37m(3,5) |\033[m\n");
		prints("如果前3，前4，前5个图案相同： |\n");
		prints("奖金(倍):  \033[1;44;37m 3\033[40m \033[46;37m 4\033[40m "
			"\033[44;37m 5\033[0;40;37m           |\033[m\n");
		prints("\033[1mＡＫＱＪ10\033[32m \033[44;32m 1\033[40m \033[46;32m 2"
			"\033[40m \033[44;32m 4\033[0;40;37m           |\033[m\n");
		prints("\033[1m     象 蛇\033[33m \033[44;33m 4\033[40m \033[46;33m10"
			"\033[40m \033[44;33m20\033[0;40;37m           |\033[m\n");
		prints("\033[1m        鸟\033[35m \033[44;35m 6\033[40m \033[46;35m20"
			"\033[40m \033[44;35m30\033[0;40;37m "
			"\033[5;1;31m○\033[0;37m可以代替|\033[m\n");
		prints("\033[1m     狮 ○\033[31m \033[44;31m10\033[40m \033[46;31m30"
			"\033[40m \033[44;31m50\033[0;40;37m 任何图案。|\033[m\n");
		prints("如果前3，前4，前5列出现\033[1;33m人\033[0;37m，则 |\033[m\n");
		prints("可获得10次免费游戏机会，此时如|\n");
		prints("果第3列出现\033[1;33m猴子\033[0;37m，则台面筹码分 |\033[m\n");
		prints("别×2，×3，×4。             |\n");
		
		move(5, 34);
		prints("筹码：%d\t玩一次需要：%d", myInfo->cash, MONKEY_UNIT);
		if (FreeGame > 0) {
			move(6, 34);
			prints("免费次数：%d\t台面筹码：%d", FreeGame, TempMoney);
		}
		
		showAt(7, 34, "按空格键开始游戏。按\033[1;32mQ\033[m退出。", NA);

		ch=igetkey();
		while ( ch != ' ' && ch != 'q' && ch != 'Q')
			ch=igetkey();

		if (mcEnv->prize777 < 50000){                 //777 50000 保底资金
		    showAt(8, 34,"赌场现金余额过低，暂停营业！",YEA);
		    ch = 'q';
		}

		if(myInfo->cash < MONKEY_UNIT) {
			showAt(8, 34, "你没钱了，只好退出游戏。", YEA);
			ch = 'q';
		}
		if(myInfo->health == 0)	{
			showAt(9, 34, "你没体力了。只好退出游戏。", YEA);
			ch = 'q';
		}
		switch(ch) {
		case 'q':
		case 'Q':
			//mcLog("退出MONKEY，赢了", TempMoney,"");
			myInfo->cash += TempMoney;
			TempMoney = 0;
			quit = 1;
			break;
		default:
			myInfo->health--;
			myInfo->Actived += 2;
			for (i=0;i<3;i++)
				for (j=0;j<5;j++) {
					if (j==2)
						pattern[i][j]=random()%12;
					else
						pattern[i][j]=random()%11;
				}

			for(j=0;j<5;j++) {
				for(i=0;i<3;i++) {
					move(10 + i, 38 + j*3);
					prints("%2s", pic[pattern[i][j]]);
				}
				prints("\n");
				refresh();
				sleep(1);
			}			
			
			sum[0] = 10000 * (pattern[0][0] % 10) +
				  1000 * (pattern[0][1] % 10) +
				   100 * (pattern[0][2] % 10) +
				    10 * (pattern[0][3] % 10) +
				         pattern[0][4] % 10;
			sum[1] = 10000 * (pattern[0][0] % 10) +
				  1000 * (pattern[1][1] % 10) +
				   100 * (pattern[2][2] % 10) +
				    10 * (pattern[1][3] % 10) +
				         pattern[0][4] % 10;
			sum[2] = 10000 * (pattern[1][0] % 10) +
				  1000 * (pattern[1][1] % 10) +
				   100 * (pattern[1][2] % 10) +
				    10 * (pattern[1][3] % 10) +
				         pattern[1][4] % 10;
			sum[3] = 10000 * (pattern[2][0] % 10) +
				  1000 * (pattern[1][1] % 10) +
				   100 * (pattern[0][2] % 10) +
				    10 * (pattern[1][3] % 10) +
				         pattern[2][4] % 10;
			sum[4] = 10000 * (pattern[2][0] % 10) +
				  1000 * (pattern[2][1] % 10) +
				   100 * (pattern[2][2] % 10) +
				    10 * (pattern[2][3] % 10) +
				         pattern[2][4] % 10;
			
			if(pattern[0][0]==10) {
				if(sum[0] != 9999 && 
				   sum[0]/10 != 999 && 
				   sum[0]%1000 != 999)
					sum[0] = 12345;
				if(sum[1] != 9999 &&
				   sum[1]/10 != 999 &&
				   sum[1]%1000 != 999)
					sum[1] = 12345;
			}
                        if(pattern[0][1]==10)
				if(sum[0] != 999)
					sum[0] = 12345;
			if(pattern[0][2]==10 || pattern[0][2]==11) {
				sum[0] = 12345;
				sum[3] = 12345;
			}
			if(pattern[1][0]==10)
				if(sum[2] != 9999 &&
				   sum[2]/10 != 999 &&
				   sum[2]%1000 != 999)
					sum[2] = 12345;
			if(pattern[1][1]==10) {
				if(sum[1] != 999)
					sum[1] = 12345;
				if(sum[2] != 999)
					sum[2] = 12345;
				if(sum[3] != 999)
					sum[3] = 12345;
			}
			if(pattern[1][2]==10 || pattern[1][2]==11)
				sum[2] = 12345;
			if(pattern[2][0]==10) {
				if(sum[3] != 9999 &&
				   sum[3]/10 != 999 &&
				   sum[3]%1000 != 999)
					sum[3] = 12345;
				if(sum[4] != 9999 &&
				   sum[4]/10 != 999 &&
				   sum[4]%1000 != 999)
					sum[4] = 12345;
			}
			if(pattern[2][1]==10)
				if(sum[4] != 999)
					sum[4] = 12345;
			if(pattern[2][2]==10 || pattern[2][2]==11) {
				sum[1] = 12345;
				sum[4] = 12345;
			}
			
			if(pattern[0][3]==10) {
				if(pattern[0][0] != 0)
					sum[0] = sum[0] / 100 * 100;
				else if(pattern[0][1] != 0)
					sum[0] = sum[0] / 100 * 100;
				else if(pattern[0][2] != 0)
					sum[0] = sum[0] / 100 * 100;
				else
					sum[0] = 11;
			}
			if(pattern[0][4]==10) {
				if(pattern[0][0] != 0)
					sum[0] = sum[0] / 10 * 10;
				else if(pattern[0][1] != 0)
					sum[0] = sum[0] / 10 * 10;
				else if(pattern[0][2] != 0)
					sum[0] = sum[0] / 10 * 10;
				else if(pattern[0][3] != 0)
					sum[0] = sum[0] / 10 * 10;
				else
					sum[0] = 1;
				if(pattern[0][0] != 0)
					sum[1] = sum[1] / 10 * 10;
				else if(pattern[1][1] != 0)                                                                 
					sum[1] = sum[1] / 10 * 10;
				else if(pattern[2][2] != 0)                                                                 
					sum[1] = sum[1] / 10 * 10;
				else if(pattern[1][3] != 0)
					sum[1] = sum[1] / 10 * 10;
				else
					sum[1] = 1;
			}
			if(pattern[1][3]==10) {
				if(pattern[0][0] != 0)
					sum[1] = sum[1] / 100 * 100;
				else if(pattern[1][1] != 0)
					sum[1] = sum[1] / 100 * 100;
				else if(pattern[2][2] != 0)
					sum[1] = sum[1] / 100 * 100;
				else
					sum[1] = 11;
				if(pattern[1][0] != 0)
					sum[2] = sum[2] / 100 * 100;
				else if(pattern[1][1] != 0)
					sum[2] = sum[2] / 100 * 100;
				else if(pattern[1][2] != 0)
					sum[2] = sum[2] / 100 * 100;
				else
					sum[2] = 11;
				if(pattern[2][0] != 0)
					sum[3] = sum[3] / 100 * 100;
				else if(pattern[1][1] != 0)
					sum[3] = sum[3] / 100 * 100;
				else if(pattern[0][2] != 0)
					sum[3] = sum[3] / 100 * 100;
				else
					sum[3] = 11;
			}
			if(pattern[1][4]==10) {
				if(pattern[1][0] != 0)
					sum[2] = sum[2] / 10 * 10;
				else if(pattern[1][1] != 0)
					sum[2] = sum[2] / 10 * 10;
				else if(pattern[1][2] != 0)
					sum[2] = sum[2] / 10 * 10;
				else if(pattern[1][3] != 0)
					sum[2] = sum[2] / 10 * 10;
				else
					sum[2] = 1;
			}
			if(pattern[2][3]==10) {
				if(pattern[2][0] != 0)
					sum[4] = sum[4] / 100 * 100;
				else if(pattern[2][1] != 0)
					sum[4] = sum[4] / 100 * 100;
				else if(pattern[2][2] != 0)
					sum[4] = sum[4] / 100 * 100;
				else
					sum[4] = 11;
			}
			if(pattern[2][4]==10) {
				if(pattern[2][0] != 0)
					sum[3] = sum[3] / 10 * 10;
				else if(pattern[1][1] != 0)
					sum[3] = sum[3] / 10 * 10;
				else if(pattern[0][2] != 0)
					sum[3] = sum[3] / 10 * 10;
				else if(pattern[1][3] != 0)
					sum[3] = sum[3] / 10 * 10;
				else
					sum[3] = 1;
				if(pattern[2][0] != 0)
					sum[4] = sum[4] / 10 * 10;
				else if(pattern[2][1] != 0)
					sum[4] = sum[4] / 10 * 10;
				else if(pattern[2][2] != 0)
					sum[4] = sum[4] / 10 * 10;
				else if(pattern[2][3] != 0)
					sum[4] = sum[4] / 10 * 10;
				else
					sum[4] = 1;
			}

			total = 0;
			for(i=0;i<5;i++)
				  total += winrate[sum[i]];
			total *= MONKEY_UNIT;

			move(15, 34);
			if(total > 0)
				prints("你赢了 %d %s", total,  MONEY_NAME);
			
			if(FreeGame > 0) {
				TempMoney += total;
				mcEnv->prize777 -= total;
				//mcLog("玩MONKEY.FreeGame赢了", total, "");
				multi = 0;
				for(i=0;i<3;i++)
					if(pattern[i][2]==11)
						multi = 1;
				if(multi) {
					//mcLog("MONKEY.FreeGame台面有", TempMoney, "");
					mcEnv->prize777 -= TempMoney * (man - 2);
					TempMoney *= man - 1;
					move(16, 34);
					prints("出现了猴子，你台面的奖金×%d。",
							man - 1);
					//mcLog("出现猴子，台面钱×", man-1, "");
				}
				FreeGame--;
				if(FreeGame == 0) {
					//mcLog("MONKEY.FreeGame共赢得", TempMoney, "");
					myInfo->cash += TempMoney;
					TempMoney = 0;
				}
			} else {
				man=0;
				for(j=0;j<5;j++){
					multi = 0;
					for(i=0;i<3;i++)
						if (pattern[i][j]==10)
							multi = 1;
					man += multi;
					if(multi == 0) {
						if(j < 3)
							man = 0;
						goto ENDMAN;
					}
				}
				ENDMAN:
			
				if(man >=3) {
					FreeGame = 10;
					TempMoney = 0;
					move(16, 34);
					prints("出现了%d个人，你获得10次免费游戏机会。\n",
							man);
					//mcLog("MONKEY中出现", man, "个人，10次FreeGame");
					move(17, 34);
					prints("免费游戏中如果出现猴子，"
						"你台面的奖金将会×%d！", man-1);
				}
				myInfo->cash += total - MONKEY_UNIT;
				mcEnv->prize777 -= total - MONKEY_UNIT;
				//mcLog("玩MONKEY赢了", total-MONKEY_UNIT, "");
			}
			move(18, 36);
			prints("\033[1;31m~~~~~  \033[32m~-_-~  \033[33m-----  "
					"\033[35m_-~-_  \033[36m_____\033[m");
			move(19, 34);
			for(i=0;i<5;i++)
				prints("%7d", sum[i]);
			move(20, 33);
			for(i=0;i<5;i++)
				prints("%7d", winrate[sum[i]]);
			pressanykey();
			break;
		}
	}
	return;
}
*/

int
money_gamble()
{
	int ch, quit = 0;
	char uident[IDLEN + 1];

       if (!(myInfo->GetLetter == 1)) {
           clear();
           showAt(5,4,"你已经关闭了金融中心游戏功能，请开启后再来。",YEA);
           return 0;
       }

	while (!quit) {
		money_show_stat("赌场大厅");
		move(6, 4);
		prints("%s赌场最近生意红火，大家尽兴啊！", CENTER_NAME);
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m 选单 \033[1;46m [1]骰宝 [2]777 [3] MONKEY [4] 经理办公室 [Q] 离开\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			money_dice();
			break;
		case '2':
			money_777();
			break;
		case '3':
			//monkey_business();  //陪率设定有问题，暂时关闭  
			break;
		case '4':
			whoTakeCharge(3, uident);
			if (strcmp(currentuser->userid, uident))
				break;
			nomoney_show_stat("赌场经理办公室");
			showAt(12, 4, "\033[1;32m正在建设中。\033[m", YEA);
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	return 0;
}

