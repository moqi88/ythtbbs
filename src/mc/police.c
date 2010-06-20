#include "mc.h"

//  ---------------------------   警署     --------------------------  // 
static int
cop_accuse()
{
	char uident[IDLEN + 1], buf[256];
	mcUserInfo *mcuInfo;
	time_t currTime;

	move(4, 4);
	prints("如果您遭遇抢劫或偷窃，如果您有任何线索，请向警方报告。");
	move(5, 4);
	prints("\033[1;32m警民合作，共创安定大好局面！\033[0m");
	if (!getOkUser("举报谁？", uident, 7, 4))
		return 0;
	move(8, 4);
	sethomefile(buf, uident, "mc.save");
	if (!file_exist(buf))
		initData(1, buf);
	if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
		return 0;
	if (seek_in_file(DIR_MC "policemen", uident)) {
		showAt(8, 4, "大胆！想诬陷警务人员吗？！", YEA);
		goto UNMAP;
	}
	if (mcuInfo->freeTime > 0) {
		showAt(8, 4, "这个人已经被警署监禁了。", YEA);
		goto UNMAP;
	}
	currTime = time(NULL);
	if ((mcuInfo->robExp == 0) || (mcuInfo->begExp == 0)
	    || (mcuInfo->lastActiveTime + 7200 < currTime) || !(mcuInfo->BeIn)) {
		showAt(8, 4, "这个人最近很安分啊！你不要诽谤别人哦！", YEA);
		goto UNMAP;
	}
	if (seek_in_file(DIR_MC "criminals_list", uident)) {
		showAt(8, 4, "此人已经被警署通缉了，警署仍然向你表示感谢。",
		       YEA);
		goto UNMAP;
	}
	getdata(8, 4, "简述案情[\033[1;33mENTER\033[0m放弃]：",
		genbuf, 40, DOECHO, YEA);
	if (genbuf[0] == '\0')
		goto UNMAP;
	move(9, 4);
	if (askyn("\033[1;33m你向警方提供的上述信息真实吗\033[0m", NA, NA) ==
	    NA)
		goto UNMAP;
	snprintf(buf, STRLEN - 1, "%s %s", uident, genbuf);
	addtofile(DIR_MC "criminals_list", buf);
	showAt(10, 4, "警方非常感谢您提供的线索，我们将尽力尽快破案。", YEA);
	return 1;

      UNMAP:unloadData(mcuInfo, 1);
	return 0;
}

static int
cop_arrange(int type)
{
	int found;
	char uident[IDLEN + 1], buf[STRLEN], title[STRLEN];
	char *actionDesc[] = { "-", "任命", "解职", NULL };
	mcUserInfo *mcuInfo;

	if (!getOkUser("请输入ID: ", uident, 12, 4))
		return 0;
	found = seek_in_file(DIR_MC "policemen", uident);
	move(13, 4);
	if (type == 1 && found) {
		showAt(13, 4, "该ID已经是警员了。", YEA);
		return 0;
	} else if (type == 2 && !found) {
		showAt(13, 4, "该ID不是警署警员。", YEA);
		return 0;
	}
	if (type == 1 && (clubtest(ROBUNION) || clubtest(BEGGAR))) {
		showAt(13, 4, "此人社会关系不明，不宜雇用为警员。", YEA);
		return 0;
	}

	sprintf(buf, "%s确定%s吗？",
		type == 2 ? "被解雇警员会损失一半胆识跟身法，" : "",
		actionDesc[type]);
	if (askyn(buf, NA, NA) == NA)
		return 0;
	if (type == 1)
		addtofile(DIR_MC "policemen", uident);
	else {
		sethomefile(buf, uident, "mc.save");
		if (!file_exist(buf))
			initData(1, buf);
		if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
			return 0;
		mcuInfo->robExp /= 2;
		mcuInfo->begExp /= 2;
		mcuInfo->luck = -100;
		del_from_file(DIR_MC "policemen", uident);
		unloadData(mcuInfo, sizeof (mcUserInfo));
	}
	sprintf(title, "【警署】%s警员 %s", actionDesc[type], uident);
	sprintf(buf, "警署署长%s %s警员 %s", currentuser->userid,
		actionDesc[type], uident);
	deliverreport(title, buf);
	system_mail_buf(buf, strlen(buf), uident, title, currentuser->userid);
	showAt(14, 4, "操作成功。", YEA);
	return 1;
}

static void
cop_Arrest(void)
{
	char uident[IDLEN + 1], police[IDLEN + 1], buf[256], title[STRLEN];
	char *CrimeName[] =
	    { "莫须有", "有伤风化", "小偷小摸", "破坏治安", "卖淫嫖娼",
		"杀人放火", "颠覆政府", "反社会反人类", "种族灭绝", NULL
	};
	int CrimeTime[] = { 1, 2, 3, 4, 6, 8, 12, 16, 24 };
	time_t currtime;
	mcUserInfo *mcuInfo;
	int num, tranum, crime = 0, bonus = 100;

	nomoney_show_stat("刑警队");
	if (!seek_in_file(DIR_MC "policemen", currentuser->userid)) {
		showAt(12, 4, "警察执行任务，闲杂人等请回避，以免误伤。", YEA);
		return;
	}
	if (clubtest(ROBUNION) || clubtest(BEGGAR)) {
		showAt(12, 4, "《无间道》看多了吧你？", YEA);
		return;
	}

	if (myInfo->robExp < 50 || myInfo->begExp < 50) {
		showAt(12, 4,
		       "新警察吧？先去练练基本功，不然就是送死了。党和国家培养你不容易啊！",
		       YEA);
		return;
	}
	if (check_health(100, 12, 4, "你没有充沛的体力来执行任务。", YEA))
		return;

	currtime = time(NULL);
	if (myInfo->WorkPoint < 7200) {
		showAt(12, 4, "不要着急，上级还没有行动指示。", YEA);
		return;
	}

	move(5, 4);
	prints("刚刚接到上级指示，马上要进行一次抓捕行动。");

	if (!getOkUser("\n请选择你的目标：", uident, 6, 4)) {
		move(8, 4);
		prints("查无此人");
		pressanykey();
		return;
	}
	if (!strcmp(uident, currentuser->userid)) {
		showAt(8, 4, "牛魔王：“老婆～快来看神经病啦～”", YEA);
		return;
	}
	if (!seek_in_file(DIR_MC "criminals_list", uident)) {
		showAt(8, 4, "此人不在通缉名单上面。”", YEA);
		return;
	}
	myInfo->Actived += 10;
	move(10, 4);
	prints("你埋伏在大富翁世界的必经之路，等待目标的出现。");
	sleep(1);

	sethomefile(buf, uident, "mc.save");
	if (!file_exist(buf))
		initData(1, buf);
	if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
		return;

	myInfo->lastActiveTime = currtime;
	myInfo->WorkPoint -= 7200;
	sleep(1);
	if (currtime > 3600 + mcuInfo->lastActiveTime) {
		myInfo->health -= 10;
		showAt(12, 4, "你等了整整一天，目标还是没有出现，只好放弃了。",
		       YEA);
		return;
	}

/*	if (!(random() % 3)) {
		myInfo->health -= 10;
		showAt(12, 4, "你眼看老远处人影一晃，目标得到风声跑掉了。",
		       YEA);
		return;
	}

	if ((myInfo->robExp + myInfo->begExp >=
	     (mcuInfo->robExp + mcuInfo->begExp)
	     * 5 + 50 || myInfo->luck - mcuInfo->luck >= 100 || (random() % 3))
	    && !(mcuInfo->robExp + mcuInfo->begExp >= (myInfo->
						       robExp +
						       myInfo->begExp) * 5 +
		 50)) {
*/
	num =
	    myInfo->robExp + myInfo->begExp + (mcuInfo->robExp +
					       mcuInfo->begExp) * 2;
	if (num > 150000)
		crime = 8;
	else if (num > 100000)
		crime = 7;
	else if (num > 50000)
		crime = 6;
	else if (num > 10000)
		crime = 5;
	else if (num > 5000)
		crime = 4;
	else if (num > 2000)
		crime = 3;
	else if (num > 1000)
		crime = 2;
	else if (num > 500)
		crime = 1;
	else
		crime = 0;
	tranum =
	    MIN((myInfo->robExp + myInfo->begExp + mcuInfo->robExp +
		 mcuInfo->begExp) / 250,
		(mcuInfo->robExp + mcuInfo->begExp) / 50);
	whoTakeCharge(8, police);
	if (!(strcmp(currentuser->userid, police)))
		bonus = 300;
	if (check_chance
	    (myInfo->robExp + myInfo->begExp, mcuInfo->robExp + mcuInfo->begExp,
	     myInfo->weapon, mcuInfo->armor, 200, bonus)) {
		myInfo->begExp += tranum + 10;
		myInfo->robExp += tranum + 10;
		myInfo->health = 0;
		myInfo->cash += mcuInfo->cash;
		mcuInfo->cash = 0;
		mcuInfo->begExp = MAX(0, mcuInfo->begExp - tranum - 10);
		mcuInfo->robExp = MAX(0, mcuInfo->robExp - tranum - 10);
		mcuInfo->luck = MAX(-100, mcuInfo->luck - 10);
//              mcuInfo->freeTime += (currtime + 10800);
		mcuInfo->freeTime = time(NULL) + CrimeTime[crime] * 3600;

		myInfo->luck = MIN(100, myInfo->luck + 10);
		del_from_file(DIR_MC "criminals_list", uident);
		prints("\n    远处有人走了过来，你定睛一看，正是你要抓的%s。"
		       "\n    你跳出来拔出手枪大吼道：“不许动！你被捕了！”"
		       "\n    %s目瞪口呆，只好束手就擒。"
		       "\n    你的胆识增加了！	你的身法增加了！"
		       "\n    你的体力耗尽了！"
		       "\n    你的人品增加了！"
		       "\n    没收罪犯所有现金做为补助！", uident, uident);
		sprintf(title, "【警署】警方抓获通缉犯%s", uident);
		sprintf(buf,
			"警员%s耐心守候，孤身勇擒在榜通缉犯%s，特此公告，以示表彰。\n\n"
			"经%s法庭判决，被告%s%s罪名成立，判处%d小时有期徒刑。",
			currentuser->userid, uident, CENTER_NAME, uident,
			CrimeName[crime], CrimeTime[crime]);
		deliverreport(title, buf);
		sprintf(buf,
			"你被警员%s设伏抓获，没收所有现金，胆识身法下降。\n"
			"还被法庭判决%s罪，关押%d小时。\n"
			"不报此仇，誓不为人！",
			currentuser->userid, CrimeName[crime],
			CrimeTime[crime]);
		if (mcuInfo->GetLetter == 1)
			system_mail_buf(buf, strlen(buf), uident,
					"你被警方抓获", currentuser->userid);
		unloadData(mcuInfo, sizeof (mcUserInfo));
	} else {
		myInfo->begExp = MAX(0, myInfo->begExp - tranum / 2);
		myInfo->robExp = MAX(0, myInfo->robExp - tranum / 2);
		myInfo->health = 0;
		mcuInfo->cash += myInfo->cash;
		myInfo->cash = 0;
		mcuInfo->robExp += tranum / 2;
		mcuInfo->begExp += tranum / 2;
		myInfo->freeTime = time(NULL) + CrimeTime[crime] * 1800 + 1800;
		prints("\n    远处有人走了过来，你定睛一看，正是你要抓的%s。"
		       "\n    你跳出来拔出手枪大吼道：“不许动！你被捕了！”"
		       "\n    正得意时，不料背后有人打了你一个闷棍。"
		       "\n    你的胆识减少了！"
		       "\n    你的身法减少了！" "\n    你昏了过去！", uident);
		sprintf(title, "【警署】警员%s受伤住院", currentuser->userid);
		sprintf(buf,
			"警员%s在执行任务的时候，被罪犯打伤住院。目前情况稳"
			"定，%d小时以后即可出院。", currentuser->userid,
			(CrimeTime[crime] + 1) / 2);
		deliverreport(title, buf);
		sprintf(buf,
			"条子%s设伏抓你，还好你艺高人胆大将其干掉，自己安然"
			"无恙。\n 顺手捞走了他身上的现金，胆识身法上升，太爽了!",
			currentuser->userid);
		if (mcuInfo->GetLetter == 1)
			system_mail_buf(buf, strlen(buf), uident, "你金蝉脱壳",
					currentuser->userid);
		unloadData(mcuInfo, sizeof (mcUserInfo));
		unloadData(myInfo, sizeof (mcUserInfo));
		unloadData(mcEnv, sizeof (MC_Env));
		pressreturn();
		Q_Goodbye();
	}

	pressreturn();
//      Q_Goodbye();
	return;

}

static void
cop_tax()
{
	char uident[IDLEN + 1], buf[256], title[STRLEN];
	time_t currtime;
	mcUserInfo *mcuInfo;
	int num;

	nomoney_show_stat("税局");

	if (clubtest(ROBUNION) || clubtest(BEGGAR)) {
		showAt(12, 4, "《无间道》看多了吧你？", YEA);
		return;
	}

	if (myInfo->robExp < 500 || myInfo->begExp < 500) {
		showAt(12, 4,
		       "新警察吧？先去练练基本功，不然就是送死了。党和国家培养你不容易啊！",
		       YEA);
		return;
	}
	if (check_health(100, 12, 4, "你没有充沛的体力来执行任务。", YEA))
		return;

	currtime = time(NULL);
	if (myInfo->WorkPoint < 9000) {
		showAt(12, 4, "不要着急，上级还没有行动指示。", YEA);
		return;
	}

	move(4, 4);
	prints("最近偷税漏税严重，需要严厉打击这种抗税风潮。");

	if (!getOkUser("\n请选择你的目标：", uident, 6, 4)) {
		move(8, 4);
		prints("查无此人");
		pressanykey();
		return;
	}
	if (!strcmp(uident, currentuser->userid)) {
		showAt(8, 4, "牛魔王：“老婆～快来看神经病啦～”", YEA);
		return;
	}

	myInfo->Actived += 10;
	move(10, 4);
	prints("你敲开了%s的家门。", uident);
	sleep(1);

	sethomefile(buf, uident, "mc.save");
	if (!file_exist(buf))
		initData(1, buf);
	if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
		return;

	if (mcuInfo->GetLetter == 0) {
		clear();
		showAt(6, 4, "虚拟货币持有者，无法征税", YEA);
		unloadData(mcuInfo, sizeof (mcUserInfo));
		return;
	}

	myInfo->lastActiveTime = currtime;
	myInfo->WorkPoint -= 9000;
	sleep(1);

	if (check_chance
	    (myInfo->robExp + myInfo->begExp, mcuInfo->robExp + mcuInfo->begExp,
	     myInfo->weapon, mcuInfo->armor, 200, 100)) {
		myInfo->begExp += 20;
		myInfo->robExp += 20;
		num = mcuInfo->credit / 3;
		myInfo->health = 0;
		mcuInfo->credit -= num;
		mcEnv->Treasury += num * 2 / 3;
		myInfo->credit += num / 3;

		myInfo->luck = MIN(100, myInfo->luck + 20);
		prints("\n    %s并不像想象中的那么滑头啊。"
		       "\n    他在你的监督下如数缴纳了%d%s的欠税。"
		       "\n    你从中获得1/3 的奖励，你的胆识身法上升。", uident,
		       num, MONEY_NAME);
		sprintf(title, "【税务】%s缴纳欠税", uident);
		sprintf(buf,
			"在警员%s的监督下，%s如期缴纳了所欠的%d%s税款。特此公告。",
			currentuser->userid, uident, num, MONEY_NAME);
		deliverreport(title, buf);
		sprintf(buf,
			"警员%s陪同税务前来查税，看来是躲不过去了。\n"
			"不得不缴纳了%d%s的税款，心痛啊。",
			currentuser->userid, num, MONEY_NAME);
		if (mcuInfo->GetLetter == 1)
			system_mail_buf(buf, strlen(buf), uident, "你缴纳欠税",
					currentuser->userid);
	} else {
		myInfo->begExp = MAX(0, myInfo->begExp - 5);
		myInfo->robExp = MAX(0, myInfo->robExp - 5);
		myInfo->health = 0;
		mcuInfo->robExp += 10;
		mcuInfo->begExp += 10;
		prints("\n    %s一副死猪不怕开水烫的样子。"
		       "\n    “要钱没有，要命一条”"
		       "\n    软硬不吃，你无计可使，不得不狼狈归去。", uident);
	}
	unloadData(mcuInfo, sizeof (mcUserInfo));
	pressanykey();
	return;
}

static void
SearchCrime()
{
	char uident[IDLEN + 1], buf[256];
	time_t currTime, ActiveTime;
	mcUserInfo *mcuInfo;
	int day, hour, minute;

	nomoney_show_stat("档案室");
	if (!getOkUser("\n请选择查询的档案：", uident, 6, 4)) {
		move(8, 4);
		prints("查无此人");
		pressanykey();
		return;
	}

	sethomefile(buf, uident, "mc.save");
	if (!file_exist(buf))
		initData(1, buf);
	if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
		return;

	ActiveTime = mcuInfo->lastActiveTime;
	currTime = time(NULL);
	if (currTime > ActiveTime) {
		day = (currTime - ActiveTime) / 86400;
		hour = (currTime - ActiveTime) % 86400 / 3600;
		minute = (currTime - ActiveTime) % 3600 / 60 + 1;
		move(9, 1);
		prints("    此人上次犯案是在%d天%d小时%d分钟以前。", day, hour,
		       minute);
	}
	unloadData(mcuInfo, sizeof (mcUserInfo));
	pressanykey();
	return;
}

static int
cop_police()
{
	int ch, quit = 0;

	while (!quit) {
		nomoney_show_stat("刑警队");

		if (!seek_in_file(DIR_MC "policemen", currentuser->userid)) {
			showAt(12, 4,
			       "警察执行任务，闲杂人等请回避，以免误伤。", YEA);
			return 0;
		}
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m 选单 \033[1;46m [1]逮捕 [2]征税 [3]移花接木 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			cop_Arrest();
			break;
		case '2':
			cop_tax();
			break;
		case '3':
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
money_cop()
{
	int ch, quit = 0;
	char uident[IDLEN + 1], buf[256];

	if (!(myInfo->GetLetter == 1)) {
		clear();
		showAt(5, 4, "你已经关闭了金融中心游戏功能，请开启后再来。",
		       YEA);
		return 0;
	}

	while (!quit) {
		sprintf(buf, "%s警署", CENTER_NAME);
		nomoney_show_stat(buf);
		move(8, 16);
		prints("打击犯罪，维持治安！");
		move(10, 4);
		prints("近期犯罪活动大大增加，刑警们也开始日以继夜的加班。");
		move(t_lines - 1, 0);
		prints("\033[1;44m 选单 \033[1;46m [1]报案 [2]通缉榜"
		       " [3]刑警队 [4]署长办公室 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			nomoney_show_stat("警署接待厅");
			cop_accuse();
			break;
		case '2':
			clear();
			move(1, 0);
			prints("%s警署当前通缉的犯罪嫌疑人:", CENTER_NAME);
			listfilecontent(DIR_MC "criminals_list");
			FreeNameList();
			pressanykey();
			break;
		case '3':
			cop_police();
			break;
		case '4':
			nomoney_show_stat("署长办公室");
			whoTakeCharge(8, uident);
			if (strcmp(currentuser->userid, uident))
				break;
			showAt(6, 0, "    请选择操作代号:\n"
			       "        1. 任命警员         2. 解职警员\n"
			       "        3. 警员名单         4. 保释罪犯\n"
			       "        5. 查询             Q. 退出", NA);
			ch = igetkey();
			switch (ch) {
			case '1':
			case '2':
				cop_arrange(ch - '0');
				break;
			case '3':
				clear();
				move(1, 0);
				prints("目前%s警署警员名单：", CENTER_NAME);
				listfilecontent(DIR_MC "policemen");
				FreeNameList();
				pressanykey();
				break;
			case '4':
				break;
			case '5':
				SearchCrime();
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
