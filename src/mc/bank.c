#include "mc.h"

// ------------------------   银行   ----------------------- //
static int
makeInterest(int basicMoney, time_t lastTime, float rate)
{				// 计算利息 
	int calHour;
	time_t currTime = time(NULL);

	if (lastTime > 0 && currTime > lastTime) {
		calHour = (currTime - lastTime) / 3600;
		calHour = MIN(calHour, 87600);
		return basicMoney * rate * calHour / 24;
	}
	return 0;
}

static int
setBankRate(int rateType)
{
	char buf[STRLEN], strRate[10];
	char rateDesc[][10] = { "存款利", "贷款利", "转帐费" };
	unsigned char rate;

	sprintf(buf, "设定新的%s率[10-250]: ", rateDesc[rateType]);
	getdata(12, 4, buf, strRate, 4, DOECHO, YEA);
	rate = atoi(strRate);
	if (rate < 10 || rate > 250) {
		showAt(13, 4, "超出浮动范围!", YEA);
		return 1;
	}
	move(13, 4);
	sprintf(buf, "新的%s率是%.2f％，确定吗", rateDesc[rateType],
		rate / 100.0);
	if (askyn(buf, NA, NA) == NA)
		return 1;
	if (rateType == 0)
		mcEnv->depositRate = rate;
	else if (rateType == 1)
		mcEnv->loanRate = rate;
	else
		mcEnv->transferRate = rate;
	update_health();
	myInfo->health--;
	myInfo->Actived++;
	sprintf(genbuf, "新的%s率为 %.2f％ 。", rateDesc[rateType],
		rate / 100.0);
	sprintf(buf, "【银行】%s银行调整%s率", CENTER_NAME, rateDesc[rateType]);
	deliverreport(buf, genbuf);
	showAt(14, 4, "设置完毕", YEA);
	return 0;
}

static int
bank_saving()
{
	char ch, quit = 0, buf[STRLEN], getInterest;
	float rate = mcEnv->depositRate / 10000.0;
	int num, total_num;

	money_show_stat("银行储蓄窗口");
	sprintf(buf, "存款利率（日）为 %.2f％", rate * 100);
	showAt(4, 4, buf, NA);
	move(t_lines - 1, 0);
	prints("\033[1;44m 选单 \033[1;46m [1]存款 [2]取款 [Q]离开\033[m");
	ch = igetkey();
	switch (ch) {
	case '1':
		if (check_health(1, 12, 4, "您的体力不够了！", YEA))
			break;
		num =
		    userInputValue(6, 4, "存", MONEY_NAME, 1000, MAX_MONEY_NUM);
		if (num == -1)
			break;
		if (myInfo->cash < num) {
			showAt(8, 4, "您没有这么多钱可以存。", YEA);
			break;
		}
		myInfo->cash -= num;
/* 加上原先存款的利息 */
		myInfo->interest +=
		    makeInterest(myInfo->credit, myInfo->depositTime, rate);
/* 新的存款开始时间 */
		myInfo->depositTime = time(NULL);
		myInfo->credit += num;
		update_health();
		myInfo->health--;
		myInfo->Actived++;
		move(8, 4);
		prints("交易成功，您现在存有 %d %s，利息共计 %d %s。",
		       myInfo->credit, MONEY_NAME, myInfo->interest,
		       MONEY_NAME);
		pressanykey();
		break;
	case '2':
		if (check_health(1, 12, 4, "您的体力不够了！", YEA))
			break;
		num =
		    userInputValue(6, 4, "取", MONEY_NAME, 1000, MAX_MONEY_NUM);
		if (num == -1)
			break;
		if (num > myInfo->credit) {
			showAt(8, 4, "您没有那么多存款。", YEA);
			break;
		}
		myInfo->interest += makeInterest(num,
						 myInfo->depositTime, rate);
		move(8, 4);
		sprintf(genbuf, "是否取出 %d %s的存款利息",
			myInfo->interest, MONEY_NAME);
		if (askyn(genbuf, NA, NA) == YEA) {
/* 存款加利息 */
			total_num = num + MAX(MIN(myInfo->interest,mcEnv->Treasury-2000000),0);
			mcEnv->Treasury -= MAX(MIN(myInfo->interest,mcEnv->Treasury-2000000),0);
			myInfo->interest = 0;
			getInterest = 1;
		} else {
			total_num = num;
			getInterest = 0;
		}
		myInfo->credit -= num;
		myInfo->cash += total_num;
		update_health();
		myInfo->health--;
		myInfo->Actived++;
		move(9, 4);
		prints("交易成功，您现在存有 %d %s，存款利息共计 %d %s。",
		       myInfo->credit, MONEY_NAME, myInfo->interest,
		       MONEY_NAME);
		pressanykey();
		break;
	case 'Q':
	case 'q':
		quit = 1;
		break;
	}
	return quit;
}

static int
bank_loan()
{
	char ch, quit = 0, buf[STRLEN];
	float rate = mcEnv->loanRate / 10000.0;
	int num, total_num, hour, maxLoanMoney;
	time_t currTime = time(NULL);

	money_show_stat("银行贷款窗口");
	sprintf(buf, "贷款利率（日）为 %.2f％", rate * 100);
	showAt(4, 4, buf, NA);
	move(5, 4);
	hour = (myInfo->backTime - currTime) / 3600 + 1;
	total_num =
	    myInfo->loan + makeInterest(myInfo->loan, myInfo->loanTime, rate);
	if (myInfo->GetLetter == 0)
		total_num /= 10;
	if (myInfo->loan > 0) {
		prints("您贷款 %d %s，当前本息共计 %d %s，距到期 %d 小时。",
		       myInfo->loan, MONEY_NAME, total_num, MONEY_NAME, hour);
	} else
		prints("您目前没有贷款。");
	move(t_lines - 1, 0);
	prints("\033[1;44m 选单 \033[1;46m [1]贷款 [2]还贷 [Q]离开\033[m");
	ch = igetkey();
	switch (ch) {
	case '1':

		if (!(myInfo->GetLetter == 1)) {
			clear();
			showAt(5, 4,
			       "你已经关闭了金融中心游戏功能，请开启后再来。",
			       YEA);
			return 1;
		}

		if (check_health(1, 12, 4, "您的体力不够了！", YEA))
			break;
		maxLoanMoney = MIN(countexp(currentuser, 2) * 250, 10000000);
		move(6, 4);
		if (maxLoanMoney < 1000) {
			showAt(8, 4, "对不起，您还没有贷款的资格。", YEA);
			break;
		}
		prints("按照银行的规定，您目前最多可以申请贷款 %d %s。",
		       maxLoanMoney, MONEY_NAME);
		num =
		    userInputValue(7, 4, "贷", MONEY_NAME, 1000, MAX_MONEY_NUM);
		if (num == -1)
			break;
		if (myInfo->loan > 0) {
			showAt(8, 4, "请先还清贷款。", YEA);
			break;
		}
		if (num > maxLoanMoney) {
			showAt(8, 4, "对不起，您要求贷款的金额超过银行规定。",
			       YEA);
			break;
		}
		while (1) {
			getdata(8, 4, "您要贷款多少天？[3-30]: ", buf,
				3, DOECHO, YEA);
			if (atoi(buf) >= 3 && atoi(buf) <= 30)
				break;
		}
		if (mcEnv->Treasury - 10000000 < num) {	//保持1000万国库
			showAt(8, 4, "对不起，国库空虚，无法贷款。", YEA);
			break;
		}
		myInfo->loanTime = currTime;
		myInfo->backTime = currTime + atoi(buf) * 86400;
		mcEnv->Treasury -= num;
		myInfo->loan += num;
		myInfo->cash += num;
		update_health();
		myInfo->health--;
		myInfo->Actived++;
		showAt(9, 4, "您的贷款手续已经完成。请到期还款。", YEA);
		break;
	case '2':
		if (check_health(1, 12, 4, "您的体力不够了！", YEA))
			break;
		if (myInfo->loan == 0) {
			showAt(6, 4, "您记错了吧？没有找到您的贷款记录啊。",
			       YEA);
			break;
		}
		if (time(NULL) < myInfo->loanTime + 86400 * 3) {
			move(6, 4);
			if (askyn
			    ("您要提前偿还贷款吗？(会收取1%手续费)", NA,
			     NA) == NA)
				break;
			total_num *= 1.01;
			move(5, 4);
			prints
			    ("您贷款 %d %s，当前本息共计 %d %s，距到期 %d 小时。",
			     myInfo->loan, MONEY_NAME, total_num, MONEY_NAME,
			     hour);
		} else {
			move(6, 4);
			if (askyn("您要现在偿还贷款吗？", NA, NA) == NA)
				break;
		}
		if (myInfo->cash < total_num) {
			showAt(7, 4, "对不起，您的钱不够偿还贷款。", YEA);
			break;
		}
		myInfo->cash -= total_num;
		mcEnv->Treasury += total_num;
		myInfo->loan = 0;
		myInfo->loanTime = 0;
		myInfo->backTime = 0;
		update_health();
		myInfo->health--;
		myInfo->Actived++;
		showAt(7, 4, "您的贷款已经还清。银行乐见并铭记您的诚信。", YEA);
		break;
	case 'q':
	case 'Q':
		quit = 1;
		break;
	}
	return quit;
}

#if 0
static int
bank_sploan()
{
	money_show_stat("银行商业贷款洽谈室");
	showAt(6, 4,
	       "\033[1;33m商业贷款是为了缓解暂时性资金周转不灵而设置的。\n    其相对于普通贷款的特点是金额大，利息高，欠贷惩罚重。\n\n    商业贷款的客户是赌场，商店等经营性业务承包人，黑帮丐帮等社团负责人。\n\n\033[1;32m    本行正在详细筹划中。。。\033[m",
	       YEA);
	return 1;
}
#endif

static void
moneytransfer()
{
	int ch, num, total_num;
	char uident[IDLEN + 1], title[STRLEN], buf[256], reason[256];
	float transferRate = mcEnv->transferRate / 10000.0;
	float rate = mcEnv->depositRate / 10000.0;
	mcUserInfo *mcuInfo;

	money_show_stat("银行转账窗口");
	if (check_health(1, 12, 4, "您的体力不够了！", YEA))
		return;
	move(4, 4);
	sprintf(genbuf,
		"最小转账金额 1000 %s。手续费 %.2f％ ",
		MONEY_NAME, transferRate * 100);
	prints("%s", genbuf);
	if (!getOkUser("转账给谁？", uident, 5, 4)) {
		showAt(12, 4, "查无此人", YEA);
		return;
	}
	if (!strcmp(uident, currentuser->userid)) {
		showAt(12, 4, "嗨，哥们～玩啥呢？”", YEA);
		return;
	}
	num = userInputValue(6, 4, "转账", MONEY_NAME, 1000, MAX_MONEY_NUM);
	if (num == -1)
		return;
	total_num = num + num * transferRate;
	getdata(7, 4, "附言：", reason, 40, DOECHO, YEA);
	prints("\n    请选择：从[1]现金 [2]存款 转帐？");
	ch = igetkey();
	sethomefile(buf, uident, "mc.save");
	if (!file_exist(buf))
		initData(1, buf);
	if ((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
		return;

	switch (ch) {
	case '1':
		if (myInfo->cash < total_num) {
			move(12, 4);
			prints("您的现金不够，加手续费此次交易共需 %d %s",
			       total_num, MONEY_NAME);
			pressanykey();
			return;
		}
		myInfo->cash -= total_num;
		mcuInfo->interest +=
		    makeInterest(mcuInfo->credit, mcuInfo->depositTime, rate);
		mcuInfo->depositTime = time(NULL);
		mcEnv->Treasury += num * transferRate;

		if (!(myInfo->GetLetter == 0) && (mcuInfo->GetLetter == 0)) {
			move(12, 4);
			prints
			    ("对方持有的是虚拟货币，您的转帐的游戏币自动按照20:1兑换为虚拟货币。");
			num /= 20;
			mcEnv->Treasury += num * 19;
		}
		if ((myInfo->GetLetter == 0) && (mcuInfo->GetLetter == 1)) {
			move(12, 4);
			prints
			    ("对方持有的是游戏币，您的转帐的虚拟货币币自动按照1:10兑换为游戏币。");
			mcEnv->Treasury -= num * 9;
			num *= 10;
		}

		mcuInfo->credit += num;
		myInfo->Actived++;
		break;
	default:
		if (myInfo->credit < total_num) {
			move(12, 4);
			prints("您的存款不够，加手续费此次交易共需 %d %s",
			       total_num, MONEY_NAME);
			pressanykey();
			return;
		}
		myInfo->credit -= total_num;
		myInfo->interest +=
		    makeInterest(total_num, myInfo->depositTime, rate);
		mcuInfo->interest +=
		    makeInterest(mcuInfo->credit, mcuInfo->depositTime, rate);
		mcuInfo->depositTime = time(NULL);
		mcEnv->Treasury += num * transferRate;

		if (!(myInfo->GetLetter == 0) && (mcuInfo->GetLetter == 0)) {
			move(12, 4);
			prints
			    ("对方持有的是虚拟货币，您的转帐的游戏币自动按照20:1兑换为虚拟货币。");
			num /= 20;
			mcEnv->Treasury += num * 19;
		}
		if ((myInfo->GetLetter == 0) && (mcuInfo->GetLetter == 1)) {
			move(12, 4);
			prints
			    ("对方持有的是游戏币，您的转帐的虚拟货币币自动按照1:10兑换为游戏币。");
			mcEnv->Treasury -= num * 9;
			num *= 10;
		}

		mcuInfo->credit += num;
		myInfo->Actived++;
		break;
	}

	update_health();
	myInfo->health--;
	sprintf(title, "您的朋友 %s 给您送钱来了", currentuser->userid);
	sprintf(buf, "%s 通过%s银行给您转帐了 %d %s，请查收。\n 附言:%s",
		currentuser->userid, CENTER_NAME, num, MONEY_NAME, reason);
//      if (mcuInfo->GetLetter == 1)
	system_mail_buf(buf, strlen(buf), uident, title, currentuser->userid);
	if (!(random() % 10) || num >= 10000000) {
		sprintf(title, "【银行】%s资金转移", currentuser->userid);
		sprintf(buf, "%s 通过%s银行转帐了 %d %s 给%s。",
			currentuser->userid, CENTER_NAME, num, MONEY_NAME,
			uident);
		deliverreport(title, buf);
	}
	sprintf(buf, "从%s转帐%d%s成功，我们已经通知了您的朋友。",
		(ch == '1') ? "现金" : "存款", num, MONEY_NAME);
	showAt(14, 4, buf, YEA);
	unloadData(mcuInfo, sizeof (mcUserInfo));
	return;
}

int
newSalary()
{
	time_t currTime = time(NULL);

	if (currTime > mcEnv->salaryEnd) {
		mcEnv->salaryStart = currTime;
		mcEnv->salaryEnd = currTime + PAYDAY * 86400;
		return 1;
	}
	return 0;
}

static int
makeSalary(int *salary, int *exp)
{

	if (currentuser->userlevel & PERM_SYSOP ||
	    currentuser->userlevel & PERM_EXT_IDLE ||
	    currentuser->userlevel & PERM_SPECIAL4) {
		*salary += 150000;
		*exp += 50;
	}
	if (currentuser->userlevel & PERM_OBOARDS ||
	    currentuser->userlevel & PERM_ACCOUNTS ||
	    currentuser->userlevel & PERM_ARBITRATE ||
	    currentuser->userlevel & PERM_SPECIAL5 ||
	    currentuser->userlevel & PERM_SPECIAL6 ||
	    currentuser->userlevel & PERM_ACBOARD) {
		*salary += 100000;
		*exp += 50;
	}
	if (currentuser->userlevel & PERM_BOARDS) {
		*salary += MIN(getbmnum(currentuser->userid) * 50000, 200000);
		*exp += getbmnum(currentuser->userid) * 50;
	}
	*salary = MIN(300000, *salary);
	*exp = MIN(200, *exp);
	return *salary;

}

int
after_tax(int income)
{
	int after;
	after = income;
	if (income > 10000)
		after = MAX(income * 0.95, 10000);
	if (income > 100000)
		after = MAX(income * 0.9, 95000);
	if (income > 1000000)
		after = MAX(income * 0.85, 900000);
	if (income > 10000000)
		after = MAX(income * 0.8, 8500000);
	if (income > 50000000)
		after = MAX(income * 0.7, 40000000);
	mcEnv->Treasury += income - after;
	return after;
}

void
forcetax()
{
	int tax_paid, cash, credit;

	cash = myInfo->cash;
	credit = myInfo->credit;
	myInfo->cash = after_tax(myInfo->cash);
	myInfo->credit = after_tax(myInfo->credit);
	tax_paid = cash + credit - myInfo->cash - myInfo->credit;

	clear();
	move(4, 4);
	if (askyn("依法纳税是一个公民应尽的义务。你要交吗？", YEA, NA) == NA) {
		prints
		    ("\n    你偷税漏税，被罚款%d偿还所欠税款，并被监禁10分钟！",
		     tax_paid);
		myInfo->freeTime = time(NULL) + 600;
		myInfo->mutex = 0;
		unloadData(myInfo, sizeof (mcUserInfo));
		unloadData(mcEnv, sizeof (MC_Env));
		pressreturn();
		Q_Goodbye();
	}
	myInfo->health = 100 + myInfo->con * 2;
	myInfo->robExp += tax_paid / 100000;
	myInfo->begExp += tax_paid / 100000;
	myInfo->luck += tax_paid / 100000;
	move(6, 4);
	prints("您交了%d的税，人品爆发！", tax_paid);
	pressanykey();

	return;
}

int
money_bank()
{
	int ch, quit = 0, salary = 0, exp = 0;
	char uident[IDLEN + 1], admin[IDLEN + 1], buf[256];
	struct userdata data;

	while (!quit) {
		sprintf(buf, "%s银行", CENTER_NAME);
		money_show_stat(buf);
		move(8, 16);
		prints("%s银行欢迎您的光临！", CENTER_NAME);
		move(t_lines - 1, 0);
		prints("\033[1;44m 选单 \033[1;46m [1]转账 [2]储蓄"
		       " [3]贷款 [4]工资 [5]行长办公室 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			moneytransfer();
			break;
		case '2':
			while (1) {
				if (bank_saving() == 1)
					break;
			}
			break;
		case '3':
			while (1) {
				if (bank_loan() == 1)
					break;
			}
			break;
#if 0
		case '4':
			while (1) {
				if (bank_sploan() == 1)
					break;
			}
			break;
#endif
		case '4':
			money_show_stat("银行工资代办窗口");
			if (check_health(1, 12, 4, "您的体力不够了！", YEA))
				break;
			salary = 0;
			exp = 0;
			makeSalary(&salary, &exp);
			if (salary == 0) {
				showAt(10, 10, "您不是本站公务员，没有工资。",
				       YEA);
				break;
			}

			if (mcEnv->salaryStart == 0
			    || time(NULL) > mcEnv->salaryEnd) {
				showAt(10, 10,
				       "对不起，银行还没有收到工资划款。", YEA);
				break;
			}

			if (myInfo->lastSalary >= mcEnv->salaryStart) {
				sprintf(genbuf,
					"您已经领过工资啦。还是勤奋工作吧！\n"
					"          下次发工资日期：%16s",
					ctime(&mcEnv->salaryEnd));
				showAt(10, 10, genbuf, YEA);
				break;
			}

			move(6, 4);
			sprintf(genbuf,
				"您本周的工资 %d %s及 %d精华值已经到帐。现在领取吗？",
				salary, MONEY_NAME, exp);
			if (askyn(genbuf, NA, NA) == NA)
				break;
			if (mcEnv->Treasury < salary) {
				showAt(10, 10,
				       "对不起，现在银行没有现金，请您稍后再来。",
				       YEA);
				break;
			}
			myInfo->lastSalary = mcEnv->salaryEnd;
			myInfo->cash += salary;
			if (myInfo->GetLetter == 1)
				mcEnv->Treasury -= salary;
			else
				mcEnv->Treasury -= salary * 10;
			update_health();
			myInfo->health--;
			myInfo->Actived++;
			loaduserdata(currentuser->userid, &data);
			data.extraexp += exp;
			saveuserdata(currentuser->userid, &data);
			showAt(8, 4, "这里是您的工资。感谢您所付出的工作!",
			       YEA);
			break;
		case '5':
			money_show_stat("行长办公室");
			move(6, 4);
			whoTakeCharge(1, uident);
			whoTakeCharge(0, admin);
			if ((strcmp(currentuser->userid, uident))
			    && (strcmp(currentuser->userid, admin)))
				break;
			prints("请选择操作代号:");
			move(7, 0);
			sprintf(genbuf,
				"      1. 调整存款利率。目前利率: %.2f％\n"
				"      2. 调整贷款利率。目前利率: %.2f％\n"
				"      3. 调整转帐费率。目前利率: %.2f％\n"
				"      4. 发工资。下次发工资日期: \033[1;33m%16s\033[m\n"
				"      Q. 退出", mcEnv->depositRate / 100.0,
				mcEnv->loanRate / 100.0,
				mcEnv->transferRate / 100.0,
				ctime(&mcEnv->salaryEnd));
			prints(genbuf);
			ch = igetkey();
			switch (ch) {
			case '1':
			case '2':
			case '3':
				update_health();
				if (check_health
				    (1, 12, 4, "您工作太辛苦了，休息一下吧！",
				     YEA))
					break;
				setBankRate(ch - '1');
				break;
			case '4':
				update_health();
				if (check_health
				    (1, 12, 4, "您工作太辛苦了，休息一下吧！",
				     YEA))
					break;
				move(12, 4);
				if (!newSalary()) {
					showAt(12, 4, "还未到发放时间。", YEA);
					break;
				}
				if (askyn("确定发放工资吗？", NA, NA) == NA)
					break;
				update_health();
				myInfo->health--;
				myInfo->Actived++;
				sprintf(buf,
					"请于%d天内到%s银行领取，过期视为放弃。",
					PAYDAY, CENTER_NAME);
				deliverreport("【银行】本站公务员领取工资",
					      buf);
				showAt(14, 4, "操作完成。", YEA);
				break;
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
