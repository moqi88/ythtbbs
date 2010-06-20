#include "mc.h"

static void
Beg_Med()
{
	char buf[256], uident[IDLEN + 1];
	time_t currtime;
	mcUserInfo *mcuInfo;
	int num;

	nomoney_show_stat("青山绿水");

	if (check_health
	    (100, 12, 4, "炼丹很耗费精力的，要准备充分才能开始。", YEA))
		return;
	if ((myInfo->robExp < 500) || (myInfo->begExp < 1000)) {
		showAt(12, 4, "就你这水平也能练出丹来？ 练出耗子屎还差不多",
		       YEA);
		return;
	}

	if (myInfo->luck < 80) {
		showAt(12, 4, "炼丹需要平心静气。", YEA);
		return;
	}

	currtime = time(NULL);
	if (myInfo->WorkPoint < 9000) {
		showAt(12, 4, "忙啊，没时间静下心来炼丹", YEA);
		return;
	}

	move(4, 1);
	if (askyn("炼丹需要各种珍稀药材，总共需要50万，你确定要买吗？", NA, NA)
	    == NA) {
		showAt(12, 4, "你决定还是不买了，太贵了。", YEA);
		return;
	}
	if (myInfo->cash < 500000) {
		showAt(12, 4, "一手交钱一手交货，你没带足够的现金。", YEA);
		return;
	}

	myInfo->lastActiveTime = currtime;
	myInfo->WorkPoint -= 9000;
	myInfo->health -= 50;
	myInfo->Actived += 20;
	myInfo->cash -= 500000;
	mcEnv->prize777 += after_tax(500000);

	move(6, 4);
	prints("七七四十九天过去了，今天开炉，成败在此一举！");
	pressanykey();

	if (random() % 2) {
		move(7, 4);
		prints("丹炉一开，一股恶臭扑面而来，闻之即欲作呕。"
		       "\n    一炉珍贵药草被糟蹋了,交学费了。");
		myInfo->robExp += 5;
		myInfo->begExp += 5;
		move(9, 4);
		prints("你积累了少许经验，胆识身法上升！");
		sprintf(buf,
			"    本市著名风景区遭受严重污染。\n"
			"    环保组织正在调查污染来源。\n");
		deliverreport("【新闻】保护环境刻不容缓", buf);
		pressanykey();
		return;
	}

	myInfo->health -= 50;
	move(9, 4);
	prints("丹炉一开，一股异香冲天而起，多日的辛苦终于有了回报。");

	move(10, 4);
	if (askyn("你决定自己服用(Y)还是出售(N)？", NA, NA) == YEA) {
		showAt(11, 4, "如此仙丹岂能便宜了别人？显然是自己服用了！",
		       YEA);
		if (check_chance
		    (myInfo->robExp, 2500, myInfo->weapon, 7, 50, 0)) {
			move(12, 4);
			prints("仙丹效用如神，你只觉得一股仙气自顶灌入。"
			       "\n    你的胆识和身法上升了125点。!你的体力全满!");
			myInfo->robExp += 125;
			myInfo->begExp += 125;
			myInfo->health = 100 + 2 * myInfo->con;
			sprintf(buf,
				"    今日本市附近旅游景区内出现奇异景象，整个风景区内异香扑鼻，令人流连忘返。");
			deliverreport("【新闻】奇哉，异哉", buf);
			pressanykey();
			return;
		} else {
			move(12, 4);
			prints("服下之后，你终于深刻体会到了两句名言的含义。"
			       "\n    第一，丑陋的本质往往被掩盖在美丽的外表下。"
			       "\n    第二，神农氏的遗言：这草有毒！！！"
			       "\n    你的胆识和身法下降20点！你的体力耗尽！你不得不住院疗养！");
			myInfo->robExp -= 20;
			myInfo->begExp -= 20;
			myInfo->health = 0;
			myInfo->WorkPoint -= 3600;
			sprintf(buf,
				"    今日发现一名重金属中毒患者，专家提醒公众，请不要服食来历不明的物品。");
			deliverreport("【新闻】请注意饮食卫生", buf);
			pressanykey();
			return;
		}
	} else {
		if (!getOkUser("你决定让谁去当小白鼠？", uident, 12, 4)) {
			showAt(13, 4, "查无此人", YEA);
			return;
		}

		if (!strcmp(uident, currentuser->userid)) {
			showAt(13, 4, "牛魔王：“老婆～快来看神经病啦～”",
			       YEA);
			return;
		}

		sethomefile(buf, uident, "mc.save");
		if (!file_exist(buf))
			initData(1, buf);
		if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
			return;

		if (mcuInfo->GetLetter == 0) {
			clear();
			showAt(6, 4, "人家不愿意跟你玩，惹不起，躲还不行啊？",
			       YEA);
			unloadData(mcuInfo, sizeof (mcUserInfo));
			return;
		}
		if (check_chance
		    (myInfo->begExp, mcuInfo->begExp, myInfo->weapon,
		     mcuInfo->armor, 200, 0)) {
			num = MIN(myInfo->credit, mcuInfo->credit / 4);
			mcuInfo->credit -= num;
			myInfo->cash += num;
			myInfo->robExp += 30;
			myInfo->begExp += 30;
			myInfo->luck -= 30;
			mcuInfo->robExp = MAX(0, mcuInfo->robExp - 10);
			mcuInfo->begExp = MAX(0, mcuInfo->begExp - 10);

			move(14, 4);
			prints
			    ("经过你如簧之舌的一阵鼓吹，%s爽快的掏出了%d%s买下了你的仙丹。"
			     "\n    你的胆识和身法上升了30点。你的人品下降了30点。",
			     uident, num, MONEY_NAME);
			sprintf(buf,
				"    近日发现某些不明身份的人物在销售变质药品，请大家留意提防。。");
			deliverreport("【新闻】假药骗子横行", buf);
			sprintf(buf,
				"你花%d%s从%s手里买了一颗所谓的仙丹，吃下去以后上吐下泻。",
				num, MONEY_NAME, currentuser->userid);
			if (mcuInfo->GetLetter == 1)
				system_mail_buf(buf, strlen(buf), uident,
						"你吃了假药",
						currentuser->userid);
		} else {
			move(14, 4);
			prints("不管你如何鼓吹，%s就是不肯掏钱，守财奴！",
			       uident);
		}
	}
	unloadData(mcuInfo, sizeof (mcUserInfo));
	pressanykey();
}

static void
Beg_Gov()
{

	time_t currtime;
	char buf[256], title[STRLEN];
	int i = 1, num;
	char *Work[] = { "县", "市", "省", "中央", NULL };

	nomoney_show_stat("培训中心");

	if (check_health
	    (140, 12, 4, "为人民服务是一件辛苦的事情，很劳累的。", YEA))
		return;

	if ((myInfo->robExp < 10000) || (myInfo->begExp < 20000)) {
		showAt(12, 4,
		       "人民公仆需要德才兼备，你还是先努力提高自己的文化水平吧",
		       YEA);
		return;
	}

	if (myInfo->luck < 90) {
		showAt(12, 4,
		       "人民公仆需要德才兼备，你还是先努力提高自己的道德修养吧",
		       YEA);
		return;
	}

	if (myInfo->cash < 1000000) {
		showAt(12, 4, "囊中羞涩，羞于见人啊", YEA);
		return;
	}

	currtime = time(NULL);
	if (myInfo->WorkPoint < 18000) {
		showAt(12, 4, "通缉令都没取消，这不是出去自投罗网吗？", YEA);
		return;
	}

	nomoney_show_stat("考场");
	move(4, 4);

	if (askyn("公务员考试报名费100万，你确定要交费吗？", NA, NA) == NA) {
		showAt(12, 4, "\n    太黑了，你决定不考了。", YEA);
		return;
	}

	myInfo->lastActiveTime = currtime;
	myInfo->WorkPoint -= 18000;
	myInfo->health = 0;
	myInfo->Actived += 30;
	myInfo->cash -= 1000000;

	move(6, 4);
	prints("\n    %s公务员考试，考场气氛十分严肃认真。", CENTER_NAME);
	sleep(1);
	prints
	    ("\n		题目好难啊，3乘以7等于多少呢？ 管他三七二十一，写个25上去再说！");
	pressanykey();

	move(9, 4);
	prints("\n    好在本人早有准备。小抄？太落后了！");
	prints("\n    传说中的高科技作弊才是王道啊！科技才是第一生产力");
	pressanykey();

	if (!(random() % 8)) {
		move(13, 4);
		prints
		    ("\n		没天理啊，这都能被发现。\n		你被赶出了考场，你的人品下降");
		myInfo->luck -= 20;
		sprintf(title, "【新闻】 公务员考试惊现作弊丑闻");
		sprintf(buf,
			"    %s在公务员考试中企图作弊，被监考人员当场发现。\n",
			currentuser->userid);
		deliverreport(title, buf);
		pressanykey();
		return;
	}

	while (i < 5) {
		nomoney_show_stat("办公室");
		sprintf(buf,
			"你现在已经是%s级干部了,你对面墙上挂着“为人民服务”。",
			Work[i - 1]);
		showAt(8, 4, buf, YEA);
		move(10, 4);
		if (askyn("拒腐防变，一心一意为人民服务?", YEA, NA) == YEA) {
			if (check_chance
			    (myInfo->begExp, 18000 + i * 4000, myInfo->weapon,
			     12 + i, 200 + i * 200, 0)) {
				move(12, 4);
				prints
				    ("\n		你全心全意为人民做贡献，人民感谢你。"
				     "\n		你的胆识身法上升！");
				myInfo->robExp += i * 25;
				myInfo->begExp += i * 25;
				i++;
				pressanykey();
			} else {
				move(12, 4);
				num =
				    MAX(MIN
					(i * 1000000,
					 mcEnv->Treasury - 20000000), 0);
				prints
				    ("\n		得过且过，做一天和尚撞一天钟"
				     "\n		无功无过，你终于混到了发工资的日子"
				     "\n		你获得了工资%d%s",
				     num, MONEY_NAME);
				myInfo->cash += num;
				mcEnv->Treasury -= num;
				sprintf(title, "【政府】 %s离休",
					currentuser->userid);
				sprintf(buf,
					"    %s因个人原因辞职，对其曾作出的贡献表示感谢。\n",
					currentuser->userid);
				deliverreport(title, buf);
				pressanykey();
				return;
			}
		} else {
			showAt(12, 4, "金钱的诱惑太大了，你还是决定出手。",
			       YEA);
			if (check_chance
			    (myInfo->begExp, 18000 + i * 4000, myInfo->armor,
			     12 + i, 200 + i * 200, 0)) {
				move(13, 4);
				num =
				    MAX(MIN
					(i * 3000000,
					 mcEnv->Treasury - 20000000), 0);
				prints
				    ("\n		不捞白不捞，利用一切职权大肆收受贿赂。"
				     "\n		有权不用，过期作废。趁着没人发现，捞了就跑吧。"
				     "\n		你的胆识身法上升!你获得了%d%s的不义之财!",
				     num, MONEY_NAME);
				myInfo->robExp += i * 40;
				myInfo->begExp += i * 40;
				myInfo->cash += num;
				mcEnv->Treasury -= num;
				sprintf(title, "【新闻】 政府高层惊爆贪污大案");
				sprintf(buf,
					"    据悉，%s在职期间，贪污高达%d%s。"
					"\n		%s现已携款潜逃，公安部门以立案搜查",
					currentuser->userid, num,
					MONEY_NAME, currentuser->userid);
				deliverreport(title, buf);
				pressanykey();
				return;
			} else {
				move(13, 4);
				prints
				    ("\n		法网恢恢，疏而不漏。莫伸手，伸手必被捉。"
				     "\n		你不但被没收了所有非法收入，还被开除工职。"
				     "\n		你的胆识身法下降!你的人品下降!");
				myInfo->robExp -= i * 10;
				myInfo->begExp -= i * 10;
				myInfo->luck -= i * 20;
				myInfo->freeTime = time(NULL) + i * 3600;
				sprintf(title, "【新闻】 天网恢恢，疏而不漏");
				sprintf(buf,
					"    大蛀虫%s被我警方查获，现以贪污罪入狱%d小时。",
					currentuser->userid, i);
				deliverreport(title, buf);
				unloadData(myInfo, sizeof (mcUserInfo));
				unloadData(mcEnv, sizeof (MC_Env));
				pressreturn();
				Q_Goodbye();
			}
		}
	}
	nomoney_show_stat("表彰会");
	move(8, 4);
	num = MAX(MIN(10000000, mcEnv->Treasury - 20000000), 0);
	prints("\n		你为人民作出了杰出的贡献，人民不会忘记你的。"
	       "\n		人民自愿捐资%d%s来感激你。", num, MONEY_NAME);
	myInfo->credit += num;
	mcEnv->Treasury -= num;
	sprintf(title, "【新闻】 人民的好公仆%s", currentuser->userid);
	sprintf(buf, "    %s同志，在职期间，尽心尽职，全心全意为人民谋福利。"
		"\n		人民真心拥护这样的好干部。",
		currentuser->userid);
	deliverreport(title, buf);
	pressanykey();
	return;
}

int
money_beggar()
{
	char ch, quit = 0, uident[IDLEN + 1];
	void *stockMem;
	int tempMoney, day, hour, minute, i, ns, worth;
	mcUserInfo *mcuInfo;
	time_t ActiveTime, currTime;
	struct BoardStock *bs;
	size_t filesize;
	char *FreeWeapon[] =
	    { "木剑", "菜刀", "匕首", "长刀", "青锋剑", "鱼肠剑", "龙泉剑",
		"七星剑", "磐龙剑", "无尘剑", "巨阙", "干将莫邪", "倚天屠龙",
		    "心剑", "天剑",
		"无剑", NULL
	};
	char *FreeArmor[] =
	    { "布衣", "长衫", "绸衣", "夜行衣", "青铜甲", "金缕衣", "金丝甲",
		"软猬甲", "天师道袍", "玄武战袍", "白虎披风", "朱雀战衣",
		    "青龙战甲",
		"龙纹披风", "圣灵披风", "七彩琉璃衣", NULL
	};
	char *RobWeapon[] =
	    { "水枪", "仿真玩具枪", "气弹枪", "火药枪", "手枪", "步枪",
		"自动步枪", "冲锋枪", "狙击枪", "轻机枪", "重机枪", "火箭筒",
		    "电浆枪",
		"激光枪", "离子炮", "核子炮", NULL
	};
	char *RobArmor[] =
	    { "婴儿车", "独轮车", "三轮车", "平板车", "自行车", "电瓶车",
		"摩托车", "长安", "东风", "夏利", "奥拓", "标致", "宝马",
		    "奔驰", "林肯",
		"法拉利", NULL
	};
	char *BegWeapon[] =
	    { "幼稚读本", "三字经", "新华字典", "电脑基础", "数据结构",
		"Basic初步", "Pascal入门", "编程思想", "C语言", "网络协议",
		    "VC教程",
		"Java浅讲", "编译原理", "汇编语言", "机器语言", "图灵机", NULL
	};
	char *BegArmor[] =
	    { "三脚猫", "武当长拳", "罗汉拳", "截心掌", "穿云掌", "七伤拳",
		"柔掌", "大力金刚掌", "玄冥神掌", "太极拳", "黯然销魂掌",
		    "降龙十八掌",
		"易筋经", "洗髓经", "九阳神功", "九阴真经", NULL
	};

	if (!(myInfo->GetLetter == 1)) {
		clear();
		showAt(5, 4, "你已经关闭了金融中心游戏功能，请开启后再来。",
		       YEA);
		return 0;
	}

	while (!quit) {
		money_show_stat("丐帮总舵");
		showAt(4, 4,
		       "丐帮自古天下第一大帮，现在贫富差距越来越大，做乞丐的人也多起了。\n"
		       "    为了生计，免不得有偷鸡摸狗坑蒙拐骗之事，当然也有劫富济贫之举。\n\n"
		       "    一个乞丐走过来问道：“要打听消息么？丐帮天上地下无所不知，无所不晓。”",
		       NA);
		move(t_lines - 1, 0);
#if 0
		prints
		    ("\033[1;44m 选单 \033[1;46m [1]打探 [2]鸡毛信 [3]丐帮活动 [4]帮主号令 [Q]离开\033[m");
#else
		prints
		    ("\033[1;44m 选单 \033[1;46m [1]打探 [2]丐帮活动 [3]帮主号令 [4]救济穷人 [Q]离开\033[m");
#endif
		ch = igetkey();

		switch (ch) {
		case '1':
			update_health();
			if (check_health
			    (2, 12, 4, "歇会歇会，神志不清就不要查了！", YEA))
				break;
			if (!getOkUser("查谁的家底？", uident, 8, 4))
				break;
			if (myInfo->cash < 1000) {
				showAt(9, 4, "啊，你只带了这么点钱吗？", YEA);
				break;
			}
			myInfo->cash -= 1000;
			mcEnv->Treasury += 1000;
			update_health();
			myInfo->health -= 2;
			myInfo->Actived++;
			sethomefile(genbuf, uident, "mc.save");
			if (!file_exist(genbuf))
				initData(1, genbuf);
			if((mcuInfo = loadData(genbuf, sizeof (mcUserInfo))) == (void*)-1)
				break;

			//计算股票价值
			worth = 0;
			if (!file_exist(DIR_STOCK "stock"))
				initData(2, DIR_STOCK "stock");
			ns = get_num_records(DIR_STOCK "stock",
					     sizeof (struct BoardStock));
			if (ns <= 0)
				goto SHOW;
			filesize = sizeof (struct BoardStock) * ns;
			//加载股市信息 
			if ((stockMem = loadData(DIR_STOCK "stock", filesize)) == (void*)-1)
				goto SHOW;

			for (i = 0; i < STOCK_NUM; i++) {
				if (mcuInfo->stock[i].num <= 0)
					continue;
				bs = stockMem +
				    mcuInfo->stock[i].stockid *
				    sizeof (struct BoardStock);
				worth +=
				    bs->todayPrice[3] * mcuInfo->stock[i].num;
			}
			unloadData(stockMem, filesize);

		      SHOW:
			move(10, 4);
			if (mcuInfo->GetLetter == 0)
				prints("\033[1;31m%s\033[m", uident);
			else
				prints("\033[1;32m%s\033[m", uident);
			prints
			    (" 大约有 \033[1;33m%d\033[m 的现金， \033[1;33m%d\033[m 的存款， 以及\033[1;33m%d\033[m 的股票",
			     mcuInfo->cash / 5000 * 5000,
			     mcuInfo->credit / 10000 * 10000,
			     worth / 10000 * 10000);
			move(11, 4);
			if (mcuInfo->GetLetter == 0)
				prints("\033[1;31m%s\033[m ", uident);
			else
				prints("\033[1;32m%s\033[m ", uident);
			prints
			    ("的胆识 \033[1;33m%d\033[m  身法 \033[1;33m%d\033[m  人品 \033[1;33m%d\033[m  根骨 \033[1;33m%d\033[m 体力 \033[1;33m%d\033[m  大狼狗 \033[1;33m%d\033[m 条\n",
			     mcuInfo->robExp, mcuInfo->begExp, mcuInfo->luck,
			     mcuInfo->con, MAX(0, mcuInfo->health),
			     mcuInfo->guard);
			move(12, 4);
			if (mcuInfo->GetLetter == 0)
				prints("\033[1;31m%s\033[m ", uident);
			else
				prints("\033[1;32m%s\033[m ", uident);
			switch (mcuInfo->BeIn) {
			case 0:
				prints
				    ("手持\033[1;33m%s\033[m, 身穿\033[1;33m%s\033[m, ",
				     FreeWeapon[mcuInfo->weapon],
				     FreeArmor[mcuInfo->armor]);
				if (seek_in_file(DIR_MC "policemen", uident))
					prints("看起来一身正气");
				else
					prints("看起来威风凛凛");
				break;
			case 1:
				prints
				    ("握着\033[1;33m%s\033[m, 驾驶\033[1;33m%s\033[m, 努力显出穷凶极恶的样子",
				     RobWeapon[mcuInfo->weapon],
				     RobArmor[mcuInfo->armor]);
				break;
			case 2:
				prints
				    ("学习了\033[1;33m%s\033[m, 修炼了\033[1;33m%s\033[m, 真是真人不露相啊",
				     BegWeapon[mcuInfo->weapon],
				     BegArmor[mcuInfo->armor]);
				break;
			}
			unloadData(mcuInfo, sizeof (mcUserInfo));
			pressanykey();
			break;
#if 0
		case '2':
			money_show_stat("养鸡场");
			move(6, 4);
			prints
			    ("有财在身，免不得有人暗地里眼红。不过好在丐帮消息灵通。\n"
			     "预交一笔鸡毛信邮费，有人暗算你的时候，就能获知鸡毛信，\n"
			     "使得安然无恙的几率增加。");
			showAt(12, 4, "\033[1;32m正在策划中。\033[m", YEA);
			break;
#endif
		case '2':
			if (!clubtest(BEGGAR)) {
				move(12, 4);
				prints("你衣冠整整，怎么做乞丐啊。。。");
				pressanykey();
				break;
			}
			if (clubtest(ROBUNION) && clubtest(BEGGAR)) {
				move(12, 4);
				prints
				    ("黑衣人：嘘～最近黑帮跟丐帮水火不容，你脚踏两只船，还是不要暴露为好。");
				pressanykey();
				break;
			}
			if (!(myInfo->BeIn == 2)) {
				myInfo->weapon = 0;
				myInfo->armor = 0;
				myInfo->BeIn = 2;
			}
			while (!quit) {
				ActiveTime = myInfo->lastActiveTime;
				currTime = time(NULL);
				money_show_stat("丐帮活动");
				if (currTime > ActiveTime) {
					day = (currTime - ActiveTime) / 86400;
					hour =
					    (currTime -
					     ActiveTime) % 86400 / 3600;
					minute =
					    (currTime -
					     ActiveTime) % 3600 / 60 + 1;
					move(9, 1);
					prints
					    ("    数数墙上刻的记号，你上次干坏事已经是%d天%d小时%d分钟以前的事情了。\n\n"
					     "    作为丐帮成员，偷鸡摸狗，混水摸鱼是你的义务天职，你怎么能这么不思进取呢？",
					     day, hour, minute);
				} else {
					move(9, 1);
					prints
					    ("    你在上次丐帮活动中，搞得灰头灰脸，现在只能躺在床上哼哼唧唧");
				}
				move(13, 1);
				prints("    警方监控度\033[1;31m  %d\033[m %%",
				       (30000 - myInfo->WorkPoint) / 300);
				move(15, 1);
				prints
				    ("    你已经学完了\033[1;33m%s\033[m, 并且练成了\033[1;33m%s\033[m, 心中暗自得意。",
				     BegWeapon[myInfo->weapon],
				     BegArmor[myInfo->armor]);
				move(t_lines - 2, 0);
				prints
				    ("\033[1;44m 选单 \033[1;46m [1]卖花 [2]妙手空空 [3] 瞒天过海  [4]修仙炼丹 [5]吸星大法 [6]从政    \033[m");
				move(t_lines - 1, 0);
				prints
				    ("\033[1;44m      \033[1;46m [9]修炼 [Q]离开                                                      \033[m");
				ch = igetkey();
				switch (ch) {
				case '1':
					money_show_stat("闹市区");
					forceGetMoney(1);
					break;
				case '2':
					money_show_stat("商业街");
					forceGetMoney(3);
					break;
				case '3':
					money_show_stat("黑客帝国");
					stealbank();
					break;
				case '4':
					Beg_Med();
					break;
				case '5':
					money_show_stat("小黑屋");
					RobPeople(1);
					//showAt(12, 4,"\033[1;32m嘴皮子功夫不到家，说了人家也不听。\033[m",YEA);
					//randomGetMoney(1); 
					break;
				case '6':
					money_show_stat("政府大楼");
					Beg_Gov();
					//showAt(12, 4,"\033[1;32m你的能力还不够。。。\033[m",YEA);
					//randomGetMoney(1); 
					break;
				case '9':
					EquipShop(2);
					break;
				case 'q':
				case 'Q':
					quit = 1;
					break;
				}
			}
			quit = 0;
			break;
		case '3':
			whoTakeCharge(5, uident);
			if (strcmp(currentuser->userid, uident))
				break;
			while (!quit) {
				nomoney_show_stat("丐帮帮主");
				move(t_lines - 1, 0);
				prints
				    ("\033[1;44m 选单 \033[1;46m [1]均贫富 [2]打狗棒法 [3]查看本帮资产 [Q]离开\033[m");
				ch = igetkey();
				switch (ch) {
				case '1':
					nomoney_show_stat("大义厅");
					showAt(12, 4,
					       "\033[1;32m正在建设中。\033[m",
					       YEA);
					break;
				case '2':
					money_show_stat("天下无狗");
					showAt(12, 4,
					       "\033[1;32m练习中。\033[m", YEA);
//forcerobMoney(2); 
					break;
				case '3':
					money_show_stat("小金库");
					showAt(12, 4,
					       "\033[1;32m会计正在点钱，请稍候。\033[m",
					       YEA);
					break;
				case 'q':
				case 'Q':
					quit = 1;
					break;
				}
			}
			quit = 0;
			break;
		case '4':
			money_show_stat("粥棚");
			move(12, 4);
			if (askyn
			    ("\033[1;33m天下大旱，颗粒无收。你要不要救济一下穷人？\033[m",
			     YEA, NA) == YEA) {
				tempMoney =
				    userInputValue(13, 4, "要出资", "万", 5,
						   100) * 10000;
				if (tempMoney < 0)
					break;
				if (myInfo->cash < tempMoney) {
					showAt(15, 4,
					       "\033[1;37m没钱还当什么善人。\033[m",
					       YEA);
					break;
				}
				update_health();
				if (check_health
				    (1, 15, 4, "你的体力不够了！", YEA))
					break;
				move(15, 4);
				prints("\033[1;37m你给穷人发了%d%s的粮食\n"
				       "    你的身法似乎有所提高！\033[m",
				       tempMoney, MONEY_NAME);
				myInfo->health--;
				myInfo->Actived += tempMoney / 25000;
				myInfo->cash -= tempMoney;
				myInfo->begExp +=
				    (tempMoney - 30000) / (30000 +
							   (random() % 30000)) +
				    1;
				mcEnv->Treasury += tempMoney;
				pressanykey();
			} else {
				showAt(14, 4,
				       "\033[1;32m葛朗台！要那么多钱有什么用！\033[m",
				       YEA);
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
