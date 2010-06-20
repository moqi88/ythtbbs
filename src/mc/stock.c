#include "mc.h"

//---------------------------------  股市  ---------------------------------// 
static int
userSelectStock(int n, int x, int y)
{
	int stockid;
	char buf[8];

	sprintf(genbuf, "请输入股票的代号[0-%d  \033[1;33mENTER\033[m放弃]:",
		n - 1);
	while (1) {
		move(x, y);
		getdata(x, y, genbuf, buf, 3, DOECHO, YEA);
		if (buf[0] == '\0')
			return -1;
		stockid = atoi(buf);
		if (stockid >= 0 && stockid < n)
			return stockid;
	}
}

static int
trendInfo(void *stockMem, int n)
{				//显示大盘走势 
	int i, yeste, today, change, stockid;
	char buf[STRLEN];
	struct BoardStock *bs;
	time_t currTime = time(NULL);
	struct tm *thist = localtime(&currTime);

	today = thist->tm_wday;
	yeste = (today <= 1) ? 6 : (today - 1);	//周日休市, 故周一的昨日算周六 
	change = mcEnv->stockPoint[today] - mcEnv->stockPoint[yeste];
	nomoney_show_stat("即时电子公告牌");
	sprintf(buf, "当前指数: %d        %s: %d点", mcEnv->stockPoint[today],
		change > 0 ? "涨" : "跌", abs(change));
	showAt(4, 4, buf, NA);
	showAt(6, 0,
	       "                  周一\t  周二\t  周三\t  周四\t  周五\t  周六\n\n"
	       "    \033[1;36m大盘走势\033[0m    ", NA);
	showAt(11, 0, "    \033[1;36m  成交手\033[0m    ", NA);
	move(8, 16);
	for (i = 1; i < 7; i++)
		prints("%6d\t", mcEnv->stockPoint[i]);
	move(11, 16);
	for (i = 1; i < 7; i++)
		prints("%6d\t", mcEnv->tradeNum[i] / 100);
	showAt(14, 0, "    \033[1;33m一周收盘\033[0m    ", NA);
	while (1) {
		if ((stockid = userSelectStock(n, 20, 4)) == -1)
			break;
		bs = stockMem + stockid * sizeof (struct BoardStock);
		move(14, 16);
		for (i = 1; i < 7; i++) {
			sprintf(genbuf, "%6.2f\t", bs->weekPrice[i]);
			prints("%s", genbuf);
		}
		pressanykey();
	}
	return 0;
}

static int
tryPutStock(void *stockMem, int n, struct BoardStock *new_bs)
{				//试图将一支新股票加入空位，成功返回下标值 
//若股票已存在或没有空位, 返回-1 
	int i, slot = -1;
	struct BoardStock *bs;

	for (i = 0; i < n; i++) {
		bs = stockMem + i * sizeof (struct BoardStock);
		if (!strcmp(bs->boardname, new_bs->boardname) && bs->timeStamp)
			return -1;
		if (bs->timeStamp == 0 && slot == -1)	//放到第一个空位 
			slot = i;
	}
	if (slot >= 0) {
		bs = stockMem + slot * sizeof (struct BoardStock);
		memcpy(bs, new_bs, sizeof (struct BoardStock));
	}
	return slot;
}

static int
newStock(void *stockMem, int n)
{				//股票上市 
	int i, money, num;
	float price;
	char boardname[24], buf[256], title[STRLEN];
	struct boardmem *bp;
	struct BoardStock bs;

	make_blist_full();
	move(10, 4);
	namecomplete("请输入将要上市的讨论区: ", boardname);
	FreeNameList();
	if (boardname[0] == '\0') {
		showAt(11, 4, "错误的讨论区名称...", YEA);
		return 0;
	}
	bp = getbcache(boardname);
	if (bp == NULL) {
		showAt(11, 4, "错误的讨论区名称...", YEA);
		return 0;
	}
	while (1) {
		getdata(11, 4, "请输入市值[单位:千万, [1-200]]:", genbuf, 4,
			DOECHO, YEA);
		money = atoi(genbuf);
		if (money >= 1 && money <= 200)
			break;
	}
	price = bp->score / 1000.0;
	money *= 10000000;
	num = money / price;
	sprintf(buf, "    该讨论区将发行 %d %s的股票。\n"
		"    共发行 %d 股, 每股价格 %.2f %s。", money, MONEY_NAME, num,
		price, MONEY_NAME);
	showAt(12, 0, buf, NA);
	move(15, 4);
	if (askyn("确定发行吗", NA, NA) == NA)
		return 0;
//初始化 
	bzero(&bs, sizeof (struct BoardStock));
	strcpy(bs.boardname, boardname);
	bs.totalStockNum = num;
	bs.remainNum = num;
	for (i = 0; i < 7; i++)
		bs.weekPrice[i] = price;
	for (i = 0; i < 4; i++)
		bs.todayPrice[i] = price;
	bs.high = price;
	bs.low = price;
	bs.boardscore = bp->score;
	bs.timeStamp = time(NULL);
	if (tryPutStock(stockMem, n, &bs) == -1)
		append_record(DIR_STOCK "stock", &bs,
			      sizeof (struct BoardStock));
	sprintf(title, "【股市】%s版今日上市", boardname);
	deliverreport(title, buf);
	showAt(16, 4, "操作完成。", YEA);
	return 1;
}

static void
deleteList(short stockid)
{				//删除一支股票的报价队列 
	char listpath[256];
	sprintf(listpath, "%s%d.sell", DIR_STOCK, stockid);
	unlink(listpath);
	sprintf(listpath, "%s%d.buy", DIR_STOCK, stockid);
	unlink(listpath);
}

static void
stockNews(struct BoardStock *bs)
{
	int n, rate;
	char newsbuf[STRLEN], titlebuf[STRLEN];

	n = random() % 1000;
	if (n < 50) {
		rate = random() % 6 + 5;
		bs->boardscore *= (1 + rate / 100.0);
		sprintf(newsbuf, "股票%s今日在利好消息刺激下股价有望上扬%d%%.",
			bs->boardname, rate);
	} else if (n < 100) {
		rate = random() % 6 + 5;
		bs->boardscore *= (1 - rate / 100.0);
		sprintf(newsbuf, "股票%s据传盈利悲观, 有分析指股价将下挫%d%%.",
			bs->boardname, rate);
	}
	if (n < 100) {
		sprintf(titlebuf, "【股市】%s股市最新消息", CENTER_NAME);
		deliverreport(titlebuf, newsbuf);
	}
}

static void
UpdateBSStatus(struct BoardStock *bs, short today, int newday, int stockid)
{				//更新一支股票的状态 

	int yeste = (today <= 1) ? 6 : (today - 1);
	float delta = bs->todayPrice[3] / bs->weekPrice[yeste];
	struct boardmem *bp;
	char buf[80];

	if (bs->status == 3)
		return;
	if (delta >= 1.1)
		bs->status = 1;	//涨停 
	else if (delta <= 0.9)
		bs->status = 2;	//跌停 
	else
		bs->status = 0;	//恢复正常 
	if (newday) {
		deleteList(stockid);
		bs->tradeNum = 0;
		bs->sellerNum = 0;
		bs->buyerNum = 0;
		bs->weekPrice[yeste] = bs->todayPrice[3];
		memset(bs->todayPrice, 0, sizeof (float) * 4);
		bs->todayPrice[3] = bs->weekPrice[yeste];
		bp = getbcache(bs->boardname);
		if (bp != NULL) {
			bs->boardscore =
			    (bp->score + bs->boardscore * 9) / 10.0;
			stockNews(bs);
		} else {
			sprintf(buf, "没有找到%s版数据，无法更新股票",
				bs->boardname);
			deliverreport(buf, "");
		}
	}
}

static int
updateStockEnv(void *stockMem, int n, int flag)
{				//更新股市统计信息 

	int i, hour, newday = 0, totalTradeNum = 0;
	float megaStockNum = 0, megaTotalValue = 0, avgPrice;
	struct BoardStock *bs;
	struct tm thist, lastt;
	time_t currTime = time(NULL);

	if (currTime < mcEnv->lastUpdateStock + 60)	//不用频繁更新 
		return 0;
	localtime_r(&currTime, &thist);
	hour = thist.tm_hour;
	mcEnv->stockTime = (hour >= 5 && hour < 23) && !(hour >= 13
							 && hour < 15)
	    && thist.tm_wday;
	if (!mcEnv->stockTime)	//休市状态下不更新 
		return 0;
	localtime_r(&mcEnv->lastUpdateStock, &lastt);
	mcEnv->lastUpdateStock = currTime;
	if (thist.tm_wday != lastt.tm_wday || flag)	//自动删除昨日交易单 
		newday = 1;
	for (i = 0; i < n; i++) {
		bs = stockMem + i * sizeof (struct BoardStock);
		if (!bs->timeStamp)	//无效股票 
			continue;
		megaStockNum += bs->totalStockNum / 1000000.0;
		megaTotalValue += megaStockNum * bs->todayPrice[3];
		UpdateBSStatus(bs, thist.tm_wday, newday, i);	//更新股票状态 
		totalTradeNum += bs->tradeNum;
	}
// 更新股市指数 
	avgPrice = megaTotalValue / megaStockNum;
	mcEnv->stockPoint[thist.tm_wday] = avgPrice * n;
// 更新成交量 
	mcEnv->tradeNum[thist.tm_wday] = totalTradeNum;
	return 0;
}

static void
showTradeInfo(void *stockMem, int n)
{				//显示一支股票的当前交易信息 
	int i, j, m, stockid, col, line;
	int buyerNum = 0, sellerNum = 0;
	size_t filesize;
	char listpath[2][256];
	void *listMem;
	struct BoardStock *bs;
	struct TradeRecord *tr;

	money_show_stat("交易信息中心");
	showAt(4, 4, "这里可以查询股票的当前报价信息。", NA);
	if ((stockid = userSelectStock(n, 6, 4)) == -1)
		return;
	bs = stockMem + stockid * sizeof (struct BoardStock);
	sprintf(genbuf, "确定要查询股票 %s 的交易信息么", bs->boardname);
	move(7, 4);
	if (askyn(genbuf, NA, NA) == NA)
		return;
	showAt(9, 0, "    查询结果如下[前10条] ：\n"
	       "    [序号  买入报价    余交易量]  ========  [序号  卖出报价    余交易量]",
	       NA);
	sprintf(listpath[0], "%s%d.buy", DIR_STOCK, stockid);
	sprintf(listpath[1], "%s%d.sell", DIR_STOCK, stockid);
	for (i = 0; i < 2; i++) {
		col = (i == 0) ? 5 : 45;
		m = get_num_records(listpath[i], sizeof (struct TradeRecord));
		if (m <= 0) {
			showAt(11, col, "没有交易报价。", NA);
			continue;
		}
		filesize = sizeof (struct TradeRecord) * m;
		if((listMem = loadData(listpath[i], filesize)) == (void*)-1)
			continue;
		for (j = 0, line = 11; j < m && j < 10; j++) {	//仅显示前10 
			tr = listMem + j * sizeof (struct TradeRecord);
			if (tr->num <= 0)
				continue;
			sprintf(genbuf, "%4d  %8.2f    %8d", line - 11,
				tr->price, tr->num);
			showAt(line++, col, genbuf, NA);
			if (i == 0)
				buyerNum++;
			else
				sellerNum++;
		}
		unloadData(listMem, filesize);
	}
	bs->buyerNum = buyerNum;	//顺便统计买卖家数, 但不准确 
	bs->sellerNum = sellerNum;	//因为只scan了前10个记录 
	sleep(1);
	pressanykey();
}

static void
showStockDetail(struct BoardStock *bs)
{				//显示一支股票的详细信息 
	char strTime[16];
	struct tm *timeStamp = localtime(&(bs->timeStamp));
	char *status[] = { "\033[1;37m正常\033[0m", "\033[1;31m涨停\033[0m",
		"\033[1;32m跌停\033[0m", "\033[1;34m暂停\033[0m"
	};

	sprintf(strTime, "%4d-%2d-%2d", timeStamp->tm_year + 1900,
		timeStamp->tm_mon + 1, timeStamp->tm_mday);
	money_show_stat("交易大厅");
	move(4, 4);
	prints("以下是股票 %s 的详细信息:", bs->boardname);
	move(6, 0);
	prints("      上市日期: %10s\t  发行量: %10d\n"
	       "      系统控股: %10d\t  散户数: %10d\n"
	       "      当前卖家: %10d\t当前买家: %10d\n"
	       "        交易量: %10d\t    状态:   %s\n",
	       strTime, bs->totalStockNum, bs->remainNum,
	       bs->holderNum, bs->sellerNum, bs->buyerNum, bs->tradeNum,
	       status[bs->status]);
	sprintf(genbuf,
		"       今日开盘: %7.2f\t最高: %7.2f\t最低: %7.2f\t平均: %7.2f\n"
		"       历史最高: %7.2f\t最低: %7.2f\n",
		bs->todayPrice[0], bs->todayPrice[1], bs->todayPrice[2],
		bs->todayPrice[3], bs->high, bs->low);
	move(11, 0);
	prints("%s", genbuf);
}

static int
getMyStock(mcUserInfo * mcuInfo, int stockid)
{
//按给定的stockid搜索一支股票 
//如果没有该股票, 那么找到一个空位, 初始化 
//返回值是股票的位置下标, 如果已经满了, 那么返回 -1 
	int i, slot = -1;

	for (i = 0; i < STOCK_NUM; i++) {
		if (mcuInfo->stock[i].stockid == stockid)
			return i;
		if (mcuInfo->stock[i].num <= 0 && slot == -1)
			slot = i;
	}
	if (slot >= 0) {
		bzero(&(mcuInfo->stock[slot]), sizeof (struct myStockUnit));
		mcuInfo->stock[slot].stockid = stockid;
	}
	return slot;
}

static void
newRecord(struct BoardStock *bs, int tradeNum, float tradePrice, int tradeType)
{				//更新一支股票的历史记录 
	time_t currTime = time(NULL);
	struct tm *thist = localtime(&currTime);
	int today = thist->tm_wday;
	int yeste = (today <= 1) ? 6 : (today - 1);
	float UsefulPrice;
	float sup = bs->weekPrice[yeste] * 1.1;
	float inf = bs->weekPrice[yeste] * 0.9;

	if (tradeNum == 0)
		return;
	UsefulPrice = MAX(MIN(tradePrice, sup), inf);

	bs->high = MAX(tradePrice, bs->high);
	bs->low = MIN(tradePrice, bs->low);
	if (bs->todayPrice[0] == 0)	//每日开盘价 
		bs->todayPrice[0] = tradePrice;
	bs->todayPrice[1] = MAX(tradePrice, bs->todayPrice[1]);	//每日最高价 
	bs->todayPrice[2] =
	    MIN(tradePrice, (bs->todayPrice[2] == 0) ? 999 : bs->todayPrice[2]);
	bs->todayPrice[3] =
	    (tradeNum * UsefulPrice +
	     bs->todayPrice[3] * bs->tradeNum) / (bs->tradeNum + tradeNum);
	bs->tradeNum += tradeNum;
	mcEnv->tradeNum[thist->tm_wday] += tradeNum;
}

static int
addToWaitingList(struct TradeRecord *mytr, int stockid)
{				//将一个待交易单加入队列 
	int i, n, slot = -1;
	char filepath[256];
	void *listMem;
	struct TradeRecord *tr;

	if (mytr->tradeType == 1)
		sprintf(filepath, "%s%d.sell", DIR_STOCK, stockid);
	else
		sprintf(filepath, "%s%d.buy", DIR_STOCK, stockid);
	n = get_num_records(filepath, sizeof (struct TradeRecord));
	if (n > 128)		//filesize <= 4k 
		return 0;
	if (n <= 0) {		//直接append_record 
		append_record(filepath, mytr, sizeof (struct TradeRecord));
		return 1;
	}
	if ((listMem = loadData(filepath, sizeof (struct TradeRecord)*n)) ==(void*) -1)
		return 0;
	for (i = 0; i < n; i++) {
		tr = listMem + i * sizeof (struct TradeRecord);
		if (!strncmp(mytr->userid, tr->userid, IDLEN)) {
			slot = i;
			break;
		}
		if (slot == -1 && tr->num <= 0)	//找到第一个无效的交易请求 
			slot = i;
	}
	if (slot >= 0) {	//替换 
		tr = listMem + slot * sizeof (struct TradeRecord);
		memcpy(tr, mytr, sizeof (struct TradeRecord));
		unloadData(listMem, sizeof (struct TradeRecord) * n);
	} else {
		unloadData(listMem, sizeof (struct TradeRecord) * n);
		append_record(filepath, mytr, sizeof (struct TradeRecord));
	}
	return 1;
}

static int
tradeArithmetic(struct BoardStock *bs, struct TradeRecord *trbuy,
		struct TradeRecord *trsell, mcUserInfo * buyer,
		mcUserInfo * seller)
{				//股票交易的核心函数 
	int idx, slot, iCanBuyNum, tradeNum, tradeMoney;

	if (buyer->cash < trsell->price || trbuy->price < trsell->price)
		return 0;	//没钱买或开价不够 
	idx = getMyStock(seller, trsell->stockid);
	if (idx == -1)		//卖方已经没有这支股票了 
		return 0;
	tradeNum = MIN(trbuy->num, trsell->num);
	iCanBuyNum = buyer->cash / trsell->price;
	tradeNum = MIN(tradeNum, iCanBuyNum);
	tradeNum = MIN(tradeNum, seller->stock[idx].num);
	if (tradeNum <= 0)	//上面确定最终双方可以交易的数量 
		return 0;
	tradeMoney = tradeNum * trsell->price;
	if ((slot = getMyStock(buyer, trbuy->stockid)) == -1)
		return 0;
	buyer->cash -= tradeMoney;	//买方扣钱 
	seller->stock[idx].num -= tradeNum;	//卖方持有股数要减 
	if (seller->stock[idx].num <= 0)	//卖方卖完,持股人-- 
		bs->holderNum--;
	if (buyer->stock[slot].num <= 0)	//新买主,持股人++ 
		bs->holderNum++;
	buyer->stock[slot].num += tradeNum;	//买方买入 
	buyer->stock[slot].stockid = trbuy->stockid;
	seller->cash += after_tax(tradeMoney);	//卖方得钱 
	trsell->num -= tradeNum;	//卖方卖出单数量减少 
	trbuy->num -= tradeNum;	//买方购买需求也要减 
	return tradeNum;
}

static int
tradeWithSys(struct BoardStock *bs, struct TradeRecord *mytr, float sysPrice)
{				//用户与系统交易 
	int sysTradeNum;
	mcUserInfo mcuInfo;
	struct TradeRecord systr;

	if (mytr->num <= 0)	//如果没有量 
		return 0;

	myInfo->health -= 10;
	bzero(&systr, sizeof (struct TradeRecord));
	if (mytr->tradeType == 0) {	//用户买入 
		systr.price = MAX(mytr->price, sysPrice);
		systr.num = bs->remainNum;
	} else {
		systr.price = MIN(sysPrice, mytr->price);
		systr.num = mytr->num;	//系统购买总是有购买欲望 
	}
//下面构造系统的交易单和交易者 
	systr.stockid = mytr->stockid;
	systr.tradeType = 1 - mytr->tradeType;
	strcpy(systr.userid, "_SYS_");

	bzero(&mcuInfo, sizeof (mcUserInfo));
	mcuInfo.cash = MAX((mcEnv->Treasury - 20000000) / 2, 0);	//取国库一半的钱,保底 2000w
	if (mcuInfo.cash <= 0)
		return 0;
	mcEnv->Treasury -= mcuInfo.cash;
	mcuInfo.stock[0].stockid = mytr->stockid;
	mcuInfo.stock[0].num = bs->remainNum;	//! 
	if (mytr->tradeType == 0)	//用户买入 
		sysTradeNum =
		    tradeArithmetic(bs, mytr, &systr, myInfo, &mcuInfo);
	else
		sysTradeNum =
		    tradeArithmetic(bs, &systr, mytr, &mcuInfo, myInfo);
	mcEnv->Treasury += mcuInfo.cash;
	if (sysTradeNum > 0) {
		bs->remainNum += mytr->tradeType ? sysTradeNum : (-sysTradeNum);
		newRecord(bs, sysTradeNum, systr.price, mytr->tradeType);
	}
	return sysTradeNum * systr.price;
}

static void
sendTradeMail(struct TradeRecord *utr, int tradeNum, float tradePrice)
{				//信件通知交易对方 
	char content[STRLEN], title[STRLEN];
	char *actionDesc[] = { "买入", "卖出" };
	char *moneyDesc[] = { "支付", "获取" };
	int type = utr->tradeType;

	sprintf(title, "你%s股票 %d 的交易完成", actionDesc[type],
		utr->stockid);
	sprintf(content, "此次交易量为 %d 股, 成交价 %.2f, %s%s %d", tradeNum,
		tradePrice, moneyDesc[type], MONEY_NAME,
		(int) (tradeNum * tradePrice));
	system_mail_buf(content, strlen(content), utr->userid, title,
			currentuser->userid);
}

static int
tradeWithUser(struct BoardStock *bs, struct TradeRecord *mytr)
{				//用户间的交易 
	int i, n, tradeNum, tradeMoney = 0;
	float tradePrice;
	char mcu_path[256], list_path[256];
	void *tradeList;
	mcUserInfo *mcuInfo;
	struct TradeRecord *utr;

	if (mytr->tradeType == 0)	//currentuser买入 
		sprintf(list_path, "%s%d.sell", DIR_STOCK, mytr->stockid);
	else
		sprintf(list_path, "%s%d.buy", DIR_STOCK, mytr->stockid);
	n = get_num_records(list_path, sizeof (struct TradeRecord));
	if (n <= 0)
		return 0;
	if ((tradeList = loadData(list_path, sizeof (struct TradeRecord)*n)) == (void*) -1)
		return 0;
	for (i = 0; i < n && mytr->num > 0; i++) {	//报价队列 
		utr = tradeList + i * sizeof (struct TradeRecord);
		if (utr->userid[0] == '\0' || utr->num <= 0)
			continue;
		sethomefile(mcu_path, utr->userid, "mc.save");
	        if((mcuInfo = loadData(mcu_path, sizeof (mcUserInfo))) == (void*)-1)
			continue;
		if (mytr->tradeType == 0)	//currentuser买入 
			tradeNum =
			    tradeArithmetic(bs, mytr, utr, myInfo, mcuInfo);
		else		//currentuser卖出, 相当于对方买入 
			tradeNum =
			    tradeArithmetic(bs, utr, mytr, mcuInfo, myInfo);
		if (tradeNum > 0) {	//更新记录和通知交易方 
			tradePrice = mytr->tradeType ? mytr->price : utr->price;
			newRecord(bs, tradeNum, tradePrice, mytr->tradeType);
			tradeMoney += tradeNum * tradePrice;
			sendTradeMail(utr, tradeNum, tradePrice);
		}
		unloadData(mcuInfo, sizeof (mcUserInfo));
	}
	unloadData(tradeList, sizeof (struct TradeRecord));
	return tradeMoney;
}

static int
tradeInteractive(struct BoardStock *bs, int stockid, int today, int type)
{				//交易的交互过程 
	int idx, inList = 0;
	int iHaveNum, inputNum, userTradeNum, sysTradeNum;
	int userTradeMoney, sysTradeMoney, totalMoney;
	float inputPrice, sysPrice, guidePrice;
	char buf[256];
	char *actionDesc[] = { "买入", "卖出", NULL };
	struct TradeRecord mytr;
	int yeste = (today <= 1) ? 6 : (today - 1);
	float sup = bs->weekPrice[yeste] * 1.1;
	float inf = bs->weekPrice[yeste] * 0.9;

	idx = getMyStock(myInfo, stockid);
	if (idx == -1 && type == 0) {
		showAt(14, 4, "你已经买了10支其它股票了, 不能再多买了", YEA);
		return 0;
	}
	if (myInfo->stock[idx].num <= 0 && type == 1) {
		showAt(14, 4, "你没有持这支股票", YEA);
		return 0;
	}
	if (bs->status == 3) {
		showAt(14, 4, "这支股票已停止交易", YEA);
		return 0;
	}
	iHaveNum = myInfo->stock[idx].num;
	move(14, 4);
	sprintf(genbuf, "目前您持有 %d 股这支股票, 控股率 %.4f％.",
		iHaveNum, iHaveNum / (bs->totalStockNum / 100.0));
	prints("%s", genbuf);
	sprintf(genbuf, "请输入要%s的股数[100]: ", actionDesc[type]);
	while (1) {
		inputNum = 100;
		getdata(15, 4, genbuf, buf, 8, DOECHO, YEA);
		if (buf[0] == '\0' || (inputNum = atoi(buf)) >= 100)
			break;
	}
	guidePrice = bs->boardscore / 1000.0;
	if (type == 0)		//currentuser买入, 系统要取高价卖 
		sysPrice = MAX(guidePrice, bs->todayPrice[3]);
	else			//currentuser卖出, 系统低价买入 
		sysPrice = MIN(0.95 * guidePrice, bs->todayPrice[3]);
	sprintf(genbuf, "请输入%s报价[%.2f]: ", actionDesc[type], sysPrice);
	while (1) {
		inputPrice = sysPrice;
		getdata(16, 4, genbuf, buf, 7, DOECHO, YEA);
		if (buf[0] != '\0')
			inputPrice = atof(buf);
		if (inputPrice < inf - 0.01 || inputPrice > sup + 0.01) {
			move(17, 4);
			if (askyn
			    ("你的报价超过涨停(跌停)价位，你确定要交易吗？", NA,
			     NA) == YEA)
				break;
		} else
			break;
	}
	sprintf(buf, "你确定以 %.2f 的报价%s %d 股的 %s 股票吗?", inputPrice,
		actionDesc[type], inputNum, bs->boardname);
	move(17, 4);
	if (askyn(buf, NA, NA) == NA)
		return 0;
	if (type == 0 && myInfo->cash < inputNum * inputPrice) {
		showAt(18, 4, "你的现金不够此次交易...", YEA);
		return 0;
	}
	if (type == 1 && myInfo->stock[idx].num < inputNum) {
		showAt(18, 4, "你没有这么多股票...", YEA);
		return 0;
	}
//构造我的交易单 
	bzero(&mytr, sizeof (struct TradeRecord));
	mytr.num = inputNum;
	mytr.tradeType = type;
	mytr.price = inputPrice;
	mytr.stockid = stockid;
	strcpy(mytr.userid, currentuser->userid);
//用户间交易 
	userTradeMoney = tradeWithUser(bs, &mytr);
	userTradeNum = abs(inputNum - mytr.num);
	if (userTradeNum > 0) {
		sprintf(buf, "用户%s了 %d 股, 成交价每股 %.2f %s",
			actionDesc[1 - type], userTradeNum,
			1.0 * userTradeMoney / userTradeNum, MONEY_NAME);
		showAt(18, 4, buf, YEA);
	}
//如果还有剩余量未完成, 那么与系统交易 
	sysTradeMoney = tradeWithSys(bs, &mytr, sysPrice);
	sysTradeNum = abs(inputNum - mytr.num - userTradeNum);
	if (sysTradeNum > 0) {
		sprintf(buf, "系统%s了 %d 股, 成交价每股 %.2f %s",
			actionDesc[1 - type], sysTradeNum,
			1.0 * sysTradeMoney / sysTradeNum, MONEY_NAME);
		showAt(19, 4, buf, YEA);
	}
//如果还有较多剩余量未交易, 那么加入交易队列 
	if (mytr.num >= 100)
		inList = addToWaitingList(&mytr, stockid);

	move(20, 4);
	if (mytr.num < inputNum) {	//如果有交易量 
		totalMoney = userTradeMoney + sysTradeMoney;
		prints("交易成功! 共%s %d股, 金额%d %s,国库自动抽税。",
		       actionDesc[type], abs(inputNum - mytr.num), totalMoney,
		       MONEY_NAME);
		//, mytr.tradeType ? taxMoney : 0);
	}

	if (mytr.num > 0) {	//交易未完成 
		myInfo->stock[idx].status = type + 1;
		move(21, 4);
		prints("剩余 %d 股未交易%s", mytr.num,
		       inList ? ", 已经加入报价队列" : ".");
	}
	pressanykey();
	return 0;
}

static int
stockTrade(void *stockMem, int n, int type)
{				//交易大厅界面 
	int i, j, yeste, today, pages, offset, stockid;
	float delta;
	char buf[256];
	struct BoardStock *bs;
	struct tm *thist;
	time_t currTime = time(NULL);
	char *status[] = { "\033[1;37m正常\033[0m", "\033[1;31m涨停\033[0m",
		"\033[1;32m跌停\033[0m", "\033[1;34m暂停\033[0m"
	};

	if (myInfo->health < 10) {
		showAt(4, 4, "你太疲惫了", YEA);
		return 0;
	}
	thist = localtime(&currTime);
	today = thist->tm_wday;
	yeste = (today <= 1) ? 6 : (today - 1);
	move(4, 4);
	prints("目前有 %d 支股票, 昨日收盘指数 %d 点, 当前指数 %d 点  状态: %s",
	       n, mcEnv->stockPoint[yeste], mcEnv->stockPoint[today],
	       mcEnv->stockTime ? "交易中" : "\033[1;31m休市中\033[0m");
	sprintf(buf, "\033[1;36m代号  %20s  %8s  %8s  %8s \t%s\033[0m",
		"版名", "昨日收盘", "今日成交", "涨幅", "状态");
	showAt(6, 4, buf, NA);
	move(7, 4);
	prints
	    ("----------------------------------------------------------------------------------");
	pages = n / 10 + 1;
	for (i = 0;; i++) {	//i用于控制页数 
		for (j = 0; j < 10; j++) {	//每屏显示最多10支股票 
			offset = i * 10 + j;
			move(8 + j, 4);
			if (offset >= n || offset < 0) {
				clrtoeol();
				continue;
			}
			bs = stockMem + offset * sizeof (struct BoardStock);
			delta = bs->todayPrice[3] / bs->weekPrice[yeste] - 1;
			sprintf(buf, "[%2d]  %20s  %8.2f  %8.2f  %8.2f%%\t%s",
				offset, bs->boardname, bs->weekPrice[yeste],
				bs->todayPrice[3], delta * 100,
				status[bs->status]);
			prints("%s", buf);
			offset++;
		}
		getdata(19, 4, "[B]前页 [C]下页 [T]交易 [Q]退出: [C]", buf, 2,
			DOECHO, YEA);
		if (toupper(buf[0]) == 'Q')
			return 0;
		if (toupper(buf[0]) == 'T' && mcEnv->stockTime)
			break;
		if (toupper(buf[0]) == 'B')
			i = (i == 0) ? (i - 1) : (i - 2);
		else
			i = (i == pages - 1) ? (i - 1) : i;
	}
	if ((stockid = userSelectStock(n, 19, 4)) == -1)
		return 0;
	bs = stockMem + stockid * sizeof (struct BoardStock);
	showStockDetail(bs);
	tradeInteractive(bs, stockid, today, type);
	return 1;
}

static void
myStockInfo()
{				//显示个人持有股票 
	int i, count = 0;
	char *status[] = { "持有", "报价买入", "报价卖出", NULL };

	nomoney_show_stat("股市个人服务中心");

	showAt(4, 0, "    以下是你个人持有股票情况: \n"
	       "    [代号]\t\t持  有  量\t\t状  态", NA);
	for (i = 0; i < STOCK_NUM; i++) {
		if (myInfo->stock[i].num <= 0)
			continue;
		move(6 + count++, 0);
		prints("    [%4d]\t\t%10d\t\t%-8s", myInfo->stock[i].stockid,
		       myInfo->stock[i].num, status[myInfo->stock[i].status]);
	}
	pressreturn();
}

static int
stockAdmin(void *stockMem, int n)
{				//管理股市 
	int ch, quit = 0, stockid;
	struct BoardStock *bs;

	while (!quit) {
		nomoney_show_stat("证监会");
		move(4, 4);
		prints("这里是管理股市的机构。请谨慎操作，对广大股民负责。");
		showAt(6, 0, "        1. 股票上市        2. 开启/关闭股市\n"
		       "        3. 股票退市        4. 暂停/恢复股票 Q. 退出\n\n"
		       "    请选择操作代号: ", NA);
		ch = igetkey();
		update_health();
		if (check_health
		    (1, 12, 4, "不要太辛苦，喝杯咖啡歇一会吧！", YEA))
			continue;
		switch (ch) {
		case '1':
			myInfo->health--;
			myInfo->Actived++;
			newStock(stockMem, n);
			break;
		case '2':
			if (!mcEnv->stockOpen)
				sprintf(genbuf, "%s", "确定要开启股市吗");
			else
				sprintf(genbuf, "%s", "确定要关闭股市吗");
			move(10, 4);
			if (askyn(genbuf, NA, NA) == NA)
				break;
			mcEnv->stockOpen = !mcEnv->stockOpen;
			update_health();
			myInfo->health--;
			myInfo->Actived++;
			showAt(11, 4, "操作完成。", YEA);
			break;
		case '3':
			if ((stockid = userSelectStock(n, 10, 4)) == -1)
				break;
			bs = stockMem + stockid * sizeof (struct BoardStock);
			sprintf(genbuf,
				"\033[5;31m警告：\033[0m确定要将股票 %s 退市么",
				bs->boardname);
			move(11, 4);
			if (askyn(genbuf, NA, NA) == YEA) {
				bs->timeStamp = 0;	//股票废弃标志 
				bs->status = 3;
				deleteList(stockid);
				showAt(12, 4, "操作完成。", YEA);
			}
			update_health();
			myInfo->health--;
			myInfo->Actived++;
			break;
		case '4':
			if ((stockid = userSelectStock(n, 10, 4)) == -1)
				break;
			bs = stockMem + stockid * sizeof (struct BoardStock);
			sprintf(genbuf, "确定要\033[1;32m%s\033[0m股票 %s 吗",
				(bs->status == 3) ? "恢复" : "暂停",
				bs->boardname);
			move(11, 4);
			if (askyn(genbuf, NA, NA) == YEA) {
				bs->status = (bs->status == 3) ? 0 : 3;
				showAt(12, 4, "操作完成。", YEA);
			}
			update_health();
			myInfo->health--;
			myInfo->Actived++;
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	return 0;
}

int
money_stock()
{
	int n, ch, quit = 0;
	size_t filesize;
	void *stockMem;
	char buf[256], uident[IDLEN + 1];

	if (!(myInfo->GetLetter == 1)) {
		clear();
		showAt(5, 4, "你已经关闭了金融中心游戏功能，请开启后再来。",
		       YEA);
		return 0;
	}

	clear();
	whoTakeCharge(6, uident);
	if (!mcEnv->stockOpen && !USERPERM(currentuser, PERM_SYSOP)
	    && (strcmp(currentuser->userid, uident))) {
		showAt(6, 16, "\033[1;31m股市暂停交易  请稍后再来\033[0m", YEA);
		return 0;
	}
	if (!file_exist(DIR_STOCK "stock"))
		initData(2, DIR_STOCK "stock");
	n = get_num_records(DIR_STOCK "stock", sizeof (struct BoardStock));
	if (n <= 0)
		return 0;
	filesize = sizeof (struct BoardStock) * n;
//加载股市信息 
	if ((stockMem = loadData(DIR_STOCK "stock", filesize)) == (void*)-1)
		return -1;
	while (!quit) {
		limit_cpu();
		sprintf(buf, "%s股市", CENTER_NAME);
		money_show_stat(buf);
		showAt(4, 4, "\033[1;33m股票有风险，涨跌难预料。\n"
		       "    入市需谨慎，破产责自负!\033[0m\n\n"
		       "        股市开盘时间\n\n"
		       "    周一到六上午 5点到13点\n"
		       "            下午15点到23点", NA);
		move(t_lines - 1, 0);
		prints("\033[1;44m 选单 \033[1;46m [1]买入 [2]卖出 [3]信息 "
		       "[4]走势 [5]个人 [9]证监会 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
		case '2':
			money_show_stat("交易大厅");
			stockTrade(stockMem, n, ch - '1');
			break;
		case '3':
			showTradeInfo(stockMem, n);
			break;
		case '4':
			trendInfo(stockMem, n);
			break;
		case '5':
			myStockInfo();
			break;
		case '9':
			whoTakeCharge(6, uident);
			if (strcmp(currentuser->userid, uident)
			    && !(currentuser->userlevel & PERM_SYSOP))
				break;
			stockAdmin(stockMem, n);
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		updateStockEnv(stockMem, n, 0);
		if (!mcEnv->stockOpen)	//踢掉呆在里面的人 
			break;
	}
	unloadData(stockMem, filesize);
	return 0;
}
