#include "mc.h"

void
Rob_Fight()
{
	char buf[256], title[STRLEN], uident[IDLEN + 1];
	time_t currtime;
	mcUserInfo *mcuInfo;
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

	nomoney_show_stat("黑帮总部");

	if (check_health
	    (110, 12, 4, "这是去打架，你当是休假啊？好好休息一下。", YEA))
		return;
	if (myInfo->robExp < 5000) {
		showAt(12, 4, "听说打架要流血，好可怕，好可怕", YEA);
		return;
	}
	if (myInfo->begExp < 3000) {
		showAt(12, 4, "你这么身手呆板，到时候打不过跑也跑不了。", YEA);
		return;
	}
	if (myInfo->luck < 85) {
		showAt(12, 4, "你的伪装不够好，起不到出其不意的效果", YEA);
		return;
	}

	currtime = time(NULL);
	if (myInfo->WorkPoint < 12000) {
		showAt(12, 4, "警察盯的太紧，没时间干活", YEA);
		return;
	}

	if (!getOkUser("你准备对谁下手？", uident, 5, 4)) {
		showAt(7, 4, "查无此人", YEA);
		return;
	}

	if (!strcmp(uident, currentuser->userid)) {
		showAt(7, 4, "自己殴自己？大脑有问题啊", YEA);
		return;
	}

	sethomefile(buf, uident, "mc.save");
	if (!file_exist(buf))
		initData(1, buf);
	if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
		return;
	if (mcuInfo->GetLetter == 0) {
		showAt(7, 4, "人家不愿意跟你玩，惹不起，躲还不行啊？", YEA);
		unloadData(mcuInfo, sizeof (mcUserInfo));
		return;
	}

	if (mcuInfo->BeIn == 1) {
		showAt(7, 4, "帮主禁令，严禁内斗", YEA);
		unloadData(mcuInfo, sizeof (mcUserInfo));
		return;
	}

	if (!(seek_in_file(DIR_MC "policemen", uident) || (mcuInfo->BeIn == 2))) {
		showAt(7, 4, "有仇报仇，黑帮成员恩怨分明。", YEA);
		unloadData(mcuInfo, sizeof (mcUserInfo));
		return;
	}

	if (mcuInfo->robExp + mcuInfo->begExp < 6000) {
		showAt(7, 4, "这家伙太弱了，搞他一点成就感没有，不屑", YEA);
		unloadData(mcuInfo, sizeof (mcUserInfo));
		return;
	}

	move(6, 4);
	if (askyn("你确定是这小子吗？", NA, NA) == NA) {
		showAt(7, 4, "还没找准目标？下次再说吧", YEA);
		unloadData(mcuInfo, sizeof (mcUserInfo));
		return;
	}

	myInfo->lastActiveTime = currtime;
	myInfo->WorkPoint -= 12000;
	myInfo->health -= 80;
	myInfo->Actived += 30;

	nomoney_show_stat("繁华都市");

	move(6, 4);
	prints
	    ("你开着心爱的%s,握紧手中的%s,心中不免有一些激动",
	     RobArmor[myInfo->armor], RobWeapon[myInfo->weapon]);
	pressanykey();

	move(7, 4);
	prints("根据已有情报，这家伙住在戊土路63号。");
	move(8, 4);
	prints("你赶到那里时，周围一点警戒都没有，这家伙死到临头了！");
	sleep(1);

	if (random() % 2) {
		move(9, 4);
		prints
		    ("你拿出%s疯狂开火，周围顿时陷入一片火海，连根毛都没跑出来。",
		     RobWeapon[myInfo->weapon]);
		move(10, 4);
		prints("你满意的大笑而归，完全忽略了旁边的路牌，戌士路......");
		myInfo->robExp += 10;
		myInfo->begExp += 10;
		myInfo->luck -= 80;
		move(12, 4);
		prints("你的胆识身法上升！\n    你的人品大幅度降低!");
		sprintf(buf,
			"    戌士路遭不明身份的恐怖分子袭击，无辜平民伤亡多人！\n"
			"    政府方面强烈谴责这种恐怖主义行动!\n");
		deliverreport("【新闻】本市遭受恐怖袭击", buf);
		unloadData(mcuInfo, sizeof (mcUserInfo));
		pressanykey();
		return;
	}

	move(9, 4);
	prints("你拿出%s疯狂开火，周围顿时陷入一片火海。",
	       RobWeapon[myInfo->weapon]);
	prints("\n    %s从中狼狈的冲了出来，一看到你顿时咬牙切齿的冲了上来。",
	       uident);
	prints
	    ("\n	  神奇的古武术能够抵挡高科技的重火力嘛？ 答案马上就要揭晓了!");
	pressanykey();

	if (check_chance
	    (myInfo->robExp, mcuInfo->robExp, myInfo->weapon, mcuInfo->armor,
	     300, 50)) {
		myInfo->begExp += 150;
		myInfo->robExp += 150;
		mcuInfo->begExp -= mcuInfo->begExp / 50;
		mcuInfo->robExp -= mcuInfo->robExp / 50;
		myInfo->luck -= 50;
		myInfo->health = 0;
		mcuInfo->health = 0;

		move(13, 4);
		prints("你冷静的对准%s来了两个漂亮的点射。", uident);
		prints
		    ("\n    血肉之躯毕竟不能抵挡高科技的武器，%s无法近身，不得不狼狈退去。",
		     uident);
		prints
		    ("\n	  虽然他溜得很快，不过还是没有子弹快啊，这次得伤够他休息一段时间的了。");
		if (!(random() % 100)) {
			if (random() % 2) {
				mcuInfo->weapon--;
				prints
				    ("\n		对方武器等级下降！！！");
			} else {
				mcuInfo->armor--;
				prints
				    ("\n		对方护具等级下降！！！");
			}
		}

		prints
		    ("\n    你的胆识和身法上升150点!\n    %s的胆识身法下降2%%。\n    你的人品下降30点。",
		     uident);
		sprintf(title, "【新闻】戊土路发生不明原因火灾");
		sprintf(buf,
			"    %s等住宅被烧毁，伤亡人数和经济损失正在进一步调查中。\n"
			"    警方怀疑与近来的黑帮活动有关。\n", uident);
		deliverreport(title, buf);
		sprintf(title, "你遭到偷袭");
		sprintf(buf, "%s趁你不备，偷袭你的住宅，此仇不可不报。",
			currentuser->userid);
		if (mcuInfo->GetLetter == 1)
			system_mail_buf(buf, strlen(buf), uident, title,
					currentuser->userid);

	} else {
		myInfo->begExp -= 50;
		myInfo->robExp -= 50;
		mcuInfo->begExp += myInfo->begExp / 100;
		mcuInfo->robExp += myInfo->robExp / 100;
		myInfo->luck -= 30;
		myInfo->WorkPoint -= 7200;
		myInfo->health = 0;

		move(13, 4);
		prints("%s竟然躲过了你的火力，飞身向你扑来!", uident);
		prints("\n    你连忙扔出烟雾弹，借着混乱撤退了!");
		prints
		    ("\n	  你嗓子一甜，一口鲜血吐了出来，毕竟还是受了暗伤啊。");
		prints
		    ("\n    你的胆识和身法下降50点!\n    %s的胆识身法上升1%%。\n    你的人品下降30点。",
		     uident);

		sprintf(title, "【新闻】黑帮发动恐怖袭击");
		sprintf(buf,
			"    昨天本市发生一起恶性黑帮恐怖袭击事件。\n"
			"    根据目击者举报，警方已经锁定犯罪嫌疑人%s。\n",
			currentuser->userid);
		deliverreport(title, buf);
		sprintf(title, "%s企图偷袭你", currentuser->userid);
		sprintf(buf,
			"%s偷袭你的住宅，不够你也狠狠的给了他一下，够这小子受的。",
			currentuser->userid);
		if (mcuInfo->GetLetter == 1)
			system_mail_buf(buf, strlen(buf), uident, title,
					currentuser->userid);

	}
	unloadData(mcuInfo, sizeof (mcUserInfo));
	pressanykey();
	return;

}

void
Rob_Rebellion()
{
	char buf[256], title[STRLEN], uident[IDLEN + 1], quit = 0, ch;
	time_t currtime;
	mcUserInfo *mcuInfo;
	int WorkPoint, num;
	char *RobWeapon[] =
	    { "水枪", "仿真玩具枪", "气弹枪", "火药枪", "手枪", "步枪",
		"自动步枪", "冲锋枪", "狙击枪", "轻机枪", "重机枪", "火箭筒",
		    "电浆枪",
		"激光枪", "离子炮", "核子炮", NULL
	};

	nomoney_show_stat("司令部");

	if (check_health(140, 12, 4, "准备不充分就不要发动了。", YEA))
		return;

	if ((myInfo->robExp < 20000) || (myInfo->begExp < 10000)) {
		showAt(12, 4, "你这样子也想暴动？先去精神病院看看吧", YEA);
		return;
	}

	if (myInfo->luck < 90) {
		showAt(12, 4, "在敌人的心脏发动暴动，一定需要良好得伪装", YEA);
		return;
	}

	currtime = time(NULL);
	if (myInfo->WorkPoint < 18000) {
		showAt(12, 4, "你累累前科，警察随时盯着你呢。", YEA);
		return;
	}

	myInfo->lastActiveTime = currtime;
	myInfo->WorkPoint -= 12000;
	myInfo->health -= 80;
	myInfo->Actived += 30;

	nomoney_show_stat("首都");

	move(6, 4);
	prints
	    ("你高唱着“我们是害虫，我们是害虫。”，兴高采烈的向%s政府进发",
	     CENTER_NAME);
	pressanykey();

	move(7, 4);
	prints
	    ("顺利来到%s政府门口，你挥舞这手中的%s,高呼“革命有理，造反无罪！”",
	     CENTER_NAME, RobWeapon[myInfo->weapon]);
	move(8, 4);
	prints("周围一群fq 跟着起哄，仿佛都是你的忠实支持者。");
	sleep(1);

	if (random() % 2) {
		move(9, 4);
		prints("在你陶醉在这热烈的气氛中时，救护车的呼啸声由远而近。");
		move(10, 4);
		prints
		    ("这些家伙居然把你伟大的事业看成了精神病，真是不可救药！");
		move(11, 4);
		prints
		    ("更可耻的是，他们居然不顾你的抗议，强行把你拖上救护车，送进了精神病院");
		move(13, 4);
		prints("你的胆识身法下降！    你的人品降低!\n");
		myInfo->robExp -= 20;
		myInfo->begExp -= 20;
		myInfo->luck -= 50;
		sprintf(title, "【新闻】 精神病街头闹事");
		sprintf(buf,
			"    今日本市一精神病人在政府前闹事，引起众人围观。\n"
			"    经精神病院诊断，此人患有严重妄想性精神病，住院治疗之后已大有好转。");
		deliverreport(title, buf);
		pressanykey();
		return;
	}

	if (check_chance(myInfo->robExp, 23000, myInfo->weapon, 15, -750, 0)) {
		move(9, 4);
		prints
		    ("一群武警从政府里冲出，fq 们立刻一哄而散，将你一个人孤零零的留在原地。");
		move(10, 4);
		prints
		    ("你还没有从这巨大的反差中反应过来，武警已经威严的对你说：“你涉嫌反政府罪，依法予以逮捕”");
		move(12, 4);
		prints
		    ("你的胆识身法下降！    你的人品降低!\n    你的体力全失!    你被警察关押4小时!");
		myInfo->robExp -= 50;
		myInfo->begExp -= 50;
		myInfo->health = 0;
		myInfo->luck -= 100;
		myInfo->mutex = 0;
		myInfo->freeTime = time(NULL) + 14400;

		sprintf(title, "【新闻】 %s企图发动反革命政变",
			currentuser->userid);
		sprintf(buf,
			"    %s企图发动反革命政变，我政府洞察其阴谋，果断予以粉碎。\n"
			"    %s已经被依法逮捕，并以反政府罪处以4小时徒刑。",
			currentuser->userid, currentuser->userid);
		deliverreport(title, buf);
		unloadData(myInfo, sizeof (mcUserInfo));
		unloadData(mcEnv, sizeof (MC_Env));
		pressanykey();
		Q_Goodbye();
		return;
	}

	myInfo->begExp += 200;
	myInfo->robExp += 200;
	num = MAX(MIN(3000000, mcEnv->Treasury - 20000000), 0);
	myInfo->cash += num;
	mcEnv->Treasury -= num;
	WorkPoint = myInfo->WorkPoint;
	move(9, 4);
	prints("你迅速冲破了警察的人墙，冲进了政府大楼。");
	prints("\n    你的旗帜终于高高飘扬在楼顶，%s新政府宣布成立了!",
	       currentuser->userid);
	prints("\n	  你的胆识身法上升! 你从国库中搜刮了%d%s。", num,
	       MONEY_NAME);
	sprintf(title, "【新闻】 %s发动政变", currentuser->userid);
	sprintf(buf, "    %s成功发动政变，占领了政府大楼。\n",
		currentuser->userid);
	deliverreport(title, buf);
	pressanykey();

	while (!(quit || (WorkPoint < 6000))) {
		nomoney_show_stat("办公室");
		move(7, 4);
		prints("革命已经成功，到摘果实的时候了，现在可以发号施令了。");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m 选单 \033[1;46m [1]批斗 [2]抓捕 [3]休闲        [Q]离开                                  \033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			nomoney_show_stat("批斗会");
			if (!getOkUser("你准备批斗谁？", uident, 5, 4)) {
				showAt(7, 4, "查无此人", YEA);
				break;
			}

			if (!strcmp(uident, currentuser->userid)) {
				showAt(7, 4, "自己批斗自己？大脑有问题啊", YEA);
				break;
			}

			if (!(seek_in_file(DIR_MC "policemen", uident))) {
				showAt(7, 4,
				       "你找不到批斗他的罪状，只能草草了事。",
				       YEA);
				break;
			}

			sethomefile(buf, uident, "mc.save");
			if (!file_exist(buf))
				initData(1, buf);
			if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
				break;
			if (mcuInfo->GetLetter == 0) {
				showAt(7, 4,
				       "人家不愿意跟你玩，惹不起，躲还不行啊？",
				       YEA);
				unloadData(mcuInfo, sizeof (mcUserInfo));
				break;
			}

			WorkPoint -= 6000;
			myInfo->WorkPoint -= 6000;
			if (check_chance
			    (myInfo->robExp, mcuInfo->robExp, myInfo->weapon,
			     mcuInfo->armor, 300, 300)) {
				move(10, 4);
				prints
				    ("你摇头晃脑的说:“要武斗，不要文斗，要触及肉体才能触及灵魂。”"
				     "\n    你的狂热拥护者们一拥而上，%s被打得遍体鳞伤。",
				     uident);
				sprintf(title, "你被%s的拥护者打伤",
					currentuser->userid);
				sprintf(buf,
					"%s煽动他的拥护者打伤了你，你不得不住院4小时。",
					currentuser->userid);
				if (mcuInfo->GetLetter == 1)
					system_mail_buf(buf, strlen(buf),
							uident, title,
							currentuser->userid);
				mcuInfo->freeTime = time(NULL) + 14400;
			} else {
				move(10, 4);
				prints("%s奋力反抗，你费尽周折都没能制服他。",
				       uident);
			}
			pressanykey();
			break;
		case '2':
			nomoney_show_stat("警察局");
			if (!getOkUser("你准备逮捕谁？", uident, 5, 4)) {
				showAt(7, 4, "查无此人", YEA);
				break;
			}

			if (!strcmp(uident, currentuser->userid)) {
				showAt(7, 4, "自己抓自己？大脑有问题啊", YEA);
				break;
			}

			if (seek_in_file(DIR_MC "policemen", uident)) {
				showAt(7, 4, "你找不到他的罪状，只能草草了事。",
				       YEA);
				break;
			}

			sethomefile(buf, uident, "mc.save");
			if (!file_exist(buf))
				initData(1, buf);
			if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
				break;
			if (mcuInfo->GetLetter == 0) {
				showAt(7, 4,
				       "人家不愿意跟你玩，惹不起，躲还不行啊？",
				       YEA);
				unloadData(mcuInfo, sizeof (mcUserInfo));
				break;
			}

			WorkPoint -= 6000;
			myInfo->WorkPoint -= 6000;
			if (check_chance
			    (myInfo->robExp, mcuInfo->robExp, myInfo->weapon,
			     mcuInfo->armor, 300, 300)) {
				move(10, 4);
				prints
				    ("你摆出一副大公无私的样子对%s说:\n		 “由于你放屁不脱裤子，所以我决定代表人民逮捕你!”",
				     uident);
				sprintf(title, "你被%s逮捕",
					currentuser->userid);
				sprintf(buf,
					"%s篡夺政权以后，以莫须有的罪名逮捕了你，你入狱4小时。",
					currentuser->userid);
				if (mcuInfo->GetLetter == 1)
					system_mail_buf(buf, strlen(buf),
							uident, title,
							currentuser->userid);
				mcuInfo->freeTime = time(NULL) + 14400;
			} else {
				move(10, 4);
				prints("%s居然武装拒捕，你无可奈何的离去了。",
				       uident);
			}
			pressanykey();
			break;
		case '3':
			WorkPoint -= 6000;
			myInfo->WorkPoint -= 6000;
			if (random() % 2) {
				nomoney_show_stat("山青水秀");
				move(7, 4);
				prints
				    ("每日为国操劳，真是费劲心神啊，找个机会休息一下。"
				     "\n    清凉的溪水中，一条条小鱼在欢乐得游动，你决定抓几条来一饱口福。");
				pressanykey();
				if (check_chance
				    (myInfo->robExp, 32000, myInfo->weapon, 15,
				     1000, 0)) {
					move(14, 4);
					prints
					    ("你美美吃了一顿烤鱼，觉得心神气爽。"
					     "\n    你的胆识身法上升！");
					myInfo->robExp += 150;
					myInfo->begExp += 150;
				} else {
					move(14, 4);
					prints
					    ("小鱼滑不留手，你手忙脚乱却连一条都没有抓住。"
					     "\n    你怒骂：“死鱼，臭鱼，还没变成烤鱼就这么坏了！”");
				}
			} else {
				nomoney_show_stat("打猎场");
				move(7, 4);
				prints
				    ("每日为国操劳，真是费尽心神啊，找个机会休息一下。"
				     "\n    看着天空中高飞的黑鹰，你弯弓搭箭，欲一显身手。");
				pressanykey();
				if (check_chance
				    (myInfo->robExp, 32000, myInfo->weapon, 15,
				     1000, 0)) {
					move(14, 4);
					prints
					    ("箭若流星，黑鹰应声而落，你感到心情大快。"
					     "\n    你的胆识身法上升！");
					myInfo->robExp += 150;
					myInfo->begExp += 150;
				} else {
					move(14, 4);
					prints("失之毫厘，差之千里。"
					       "\n    黑鹰不屑地从你头上掠过");
				}
			}
			pressanykey();
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}

	nomoney_show_stat("落魄街头");
	showAt(7, 4, "政府军反攻了，你决定暂时战略性撤退。", YEA);
	return;
}

int
forceGetMoney(int type)
{
	int money, cost_health;
	time_t currtime;
	char uident[IDLEN + 1], buf[256], place[STRLEN];
	char *actionDesc[] = { "勒索", "卖花", "抢劫", "妙手空空", NULL };
	mcUserInfo *mcuInfo;

	if ((type == 2 && myInfo->robExp < 50)
	    || (type == 3 && myInfo->begExp < 50)) {
		move(10, 8);
		prints("你还没有足够的%s来做%s这么大的事情。",
		       type % 2 ? "身法" : "胆识", actionDesc[type]);
		pressanykey();
		return 0;
	}
	move(4, 4);
	switch (type) {
	case 0:
		strcpy(place, "富人区");
		break;
	case 1:
		strcpy(place, "中心地段");
		break;
	case 2:
		strcpy(place, "冷清小巷");
		break;
	case 3:
		strcpy(place, "商业街");
		break;
	}
	prints("这里是%s的%s，实在是%s的好地方。", CENTER_NAME, place,
	       actionDesc[type]);
	if (!getOkUser("你要向谁下手？", uident, 6, 4)) {
		move(7, 4);
		prints("查无此人");
		pressanykey();
		return 0;
	}
	if (!strcmp(uident, currentuser->userid)) {
		showAt(7, 4, "牛魔王：“老婆～快来看神经病啦～”", YEA);
		return 0;
	}
	move(7, 4);
	if ((type % 2 == 0 && !clubtest(ROBUNION))
	    || (type % 2 == 1 && !clubtest(BEGGAR))) {
		prints("怎么看你也不像是会%s的人啊！", actionDesc[type]);
		pressanykey();
		return 0;
	}
#if 0				//不在线也可以偷抢，加速货币流动
	if (!t_search(uident, NA, 1) && type <= 1) {
		if (type == 0)
			prints("你看错人了吧？刚过这个人不是%s啊！", uident);
		else if (type == 1)
			prints("%s不在家，你敲了半天门也没人应。", uident);
		else if (type == 2)
			prints("冷清小巷确实冷清啊，居然一个人都没有。。。。");
		else
			prints("十字路口人来人往，你找了半天没找到要找的%s。",
			       uident);
		pressanykey();
		return 0;
	}
#endif
	cost_health = 25 + (type / 2 * 10);
	if (check_health(cost_health, 12, 4, "你哪有那么多体力做事啊？", YEA))
		return 0;

	currtime = time(NULL);
	if (myInfo->WorkPoint < (type / 2 + 1) * 1500) {
		if (type % 4 == 0)
			prints
			    ("你刚做完坏事，人家记忆犹新。%s一看见你就远远的躲开了，压根不往这边走。",
			     uident);
		else if (type == 1)
			prints
			    ("%s怒不可遏，骂道：“臭要饭的，烦死了，还不快滚！”",
			     uident);
		else		//妙手空空 
			prints
			    ("你正要下手，就听见边上有人喊：”啊，我的钱包不见了！“ \n    %s一听，马上把钱包给捂住了。”",
			     uident);
		pressanykey();
		return 0;
	}
	myInfo->lastActiveTime = currtime;
	update_health();
	myInfo->WorkPoint -= (type / 2 + 1) * 1500;
	myInfo->health -= cost_health;
	myInfo->Actived += 5;
	myInfo->luck--;
	switch (type) {
	case 0:
		prints("你手里晃着指甲刀, 对着%s嘿嘿奸笑道: “收取过路费！”\n",
		       uident);
		break;
	case 1:
		prints
		    ("你对着%s哭喊道：“买朵玫瑰花吧～我已经好几分钟没吃饭了，我好饿啊！”\n",
		     uident);
		break;
	case 2:
		prints
		    ("你手拿刀片对着%s恶狠狠的喊道：“IC~IP~IQ~~卡，统统告诉我密码！！”\n",
		     uident);
		break;
	case 3:
		prints
		    ("你不动声色的靠近%s，神不知鬼不觉的把手伸向他的衣兜。。。\n",
		     uident);
		break;
	}

	sleep(1);
	sethomefile(buf, uident, "mc.save");
	if (!file_exist(buf))
		initData(1, buf);
	if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
		return 0;

	if (mcuInfo->GetLetter == 0) {
		clear();
		showAt(6, 4, "人家不愿意跟你玩，惹不起，躲还不行啊？", YEA);
		unloadData(mcuInfo, sizeof (mcUserInfo));
		return 0;
	}

	if ((!(random() % 4)) || mcuInfo->luck - myInfo->luck >= 100) {
		if (type % 2 == 1) {
			prints("    %s一脚把你踹开。骂道：臭叫化子一边去！\n",
			       uident);
		} else {
			prints
			    ("    %s飞起一脚把你踢飞。哼道：”我最瞧不起勒索打劫的，一点技术含量都没有。“\n",
			     uident);
		}
		prints("\n    你脸一红，赶紧灰溜溜的跑开了");
		pressanykey();
		return 0;
	}

	money = MIN(mcuInfo->cash / 10, 1000000);
	money = MIN(money, myInfo->cash * 2);
	move(8, 4);
	if (money == 0) {
		prints("%s身上没钱，碰上了穷鬼真倒霉...", uident);
		goto UNMAP;
	}
	if (type == 0) {
		if (check_chance
		    (myInfo->robExp, mcuInfo->robExp, myInfo->weapon,
		     mcuInfo->armor, 300, 0)) {
			mcuInfo->cash -= money;
			myInfo->cash += money;
			myInfo->robExp += 1;
			myInfo->health -= 20;
			prints
			    ("%s吓得直打哆嗦, 赶紧从身上拿出 %d %s给你。\n\n\033[32m    你的胆识增加了！\033[m",
			     uident, money, MONEY_NAME);
			if (myInfo->luck > -100) {
				myInfo->luck -= 1;
				prints("\n\033[31m    你的人品降低了！");
			}
			sprintf(genbuf, "你被%s 勒索了 %d%s，太不幸了。",
				currentuser->userid, money, MONEY_NAME);
			sprintf(buf, "你遭到勒索");
		} else if (random() % 3) {
			myInfo->robExp += 1;
			myInfo->health -= 20;
			prints
			    ("%s吓得拔腿就跑, 虽然没有抢到钱，但是你的表现还是可圈可点。 \n\n\033[32m    你的胆识增加了！\033[m",
			     uident);
			if (myInfo->luck > -100) {
				myInfo->luck -= 1;
				prints("\n\033[31m    你的人品降低了！");
			}
			pressanykey();
			return 0;
		} else {
			prints("啊，有警察过来了！风紧，扯乎～\n");
			pressanykey();
			return 0;
		}
	} else if (type == 1) {
		if (check_chance
		    (myInfo->begExp, mcuInfo->begExp, myInfo->weapon,
		     mcuInfo->armor, 300, 0)) {
			mcuInfo->cash -= money;
			myInfo->cash += money;
			myInfo->begExp += 1;
			myInfo->health -= 20;
			prints
			    ("%s眼圈顿时红了，赶紧从身上拿出 %d %s从你手里接过一朵。\n\n\033[32m    你的身法提高了！\033[m",
			     uident, money, MONEY_NAME);
			if (myInfo->luck > -100) {
				myInfo->luck -= 1;
				prints("\n\033[31m    你的人品降低了！");
			}
			sprintf(genbuf,
				"你一时好心，花了%d%s从%s那买了朵花 ，过后发现是狗尾巴草。。。",
				money, MONEY_NAME, currentuser->userid);
			sprintf(buf, "你遇到卖花小孩");
		} else {
			if (random() % 3) {
				myInfo->begExp += 1;
				myInfo->health -= 20;
				prints
				    ("%s微笑着拍着你的头说，“小兄弟，表演得不错。”\n\n\033[32m    你的身法提高了！\033[m",
				     uident);
				if (myInfo->luck > -100) {
					myInfo->luck -= 1;
					prints
					    ("\n\033[31m    你的人品降低了！");
				}
				pressanykey();
				return 0;
			} else {
				prints("哇哇，城管来了，快跑啊～\n");
				pressanykey();
				return 0;
			}
		}
	} else if (type == 2) {
		if (check_chance
		    (myInfo->robExp, mcuInfo->robExp, myInfo->weapon,
		     mcuInfo->armor, 250, 0)) {
			money = MIN(mcuInfo->cash / 2, 5000000);
			money = MIN(myInfo->cash / 2 * 3, money);
			mcuInfo->cash -= money;
			myInfo->cash += money;
			myInfo->robExp += 5;
			myInfo->health -= 30;
			prints
			    ("%s吓得把身上的%d%s全掏了出来，哭道：”您可千万别劫色啊～“\n\n\033[32m    你的胆识增加了！\033[m",
			     uident, money, MONEY_NAME);
			if (myInfo->luck > -100) {
				myInfo->luck = MAX((myInfo->luck - 3), -100);
				prints("\n\033[31m    你的人品降低了！");
			}
			sprintf(genbuf,
				"你遇到劫匪，被 %s 抢走 %d%s，真是欲哭无泪啊。\n",
				currentuser->userid, money, MONEY_NAME);
			sprintf(buf, "你被抢劫");
		} else if (random() % 3) {
			myInfo->robExp += 5;
			myInfo->health -= 30;
			prints
			    ("%s吓得把身上的%d%s全掏了出来，可是，为什么里面就没一张真钞票呢？“\n\n\033[32m    你的胆识增加了！\033[m",
			     uident, money, MONEY_NAME);
			if (myInfo->luck > -100) {
				myInfo->luck = MAX((myInfo->luck - 3), -100);
				prints("\n\033[31m    你的人品降低了！");
			}
			pressanykey();
			return 0;
		} else {
			money = MIN(myInfo->cash / 2, 5000000);
			mcuInfo->cash += money;
			myInfo->cash -= money;
			myInfo->robExp = MAX((myInfo->robExp - 10), 0);
			myInfo->health -= 40;
			prints
			    ("%s不慌不忙的从兜里掏出一把手枪，顶住你脑门：还是你把钱交出来吧。\n\n    你遇到亡命之徒，损失了%d%s...\n\n\033[31m    你的胆识大减！\033[m",
			     uident, money, MONEY_NAME);
			sprintf(genbuf,
				"劫匪 %s 被你黑吃黑，抢到 %d%s，恭喜你～\n",
				currentuser->userid, money, MONEY_NAME);
			sprintf(buf, "你黑吃黑成功");
		}
	} else {
		if (check_chance
		    (myInfo->begExp, mcuInfo->begExp, myInfo->weapon,
		     mcuInfo->armor, 250, 0)) {
			money = MIN(mcuInfo->cash / 2, 5000000);
			money = MIN(myInfo->cash / 2 * 3, money);
			mcuInfo->cash -= money;
			myInfo->cash += money;
			myInfo->begExp += 5;
			myInfo->health -= 30;
			prints
			    ("哈哈，成功了！你用刀片在%s的衣兜割了一个口子,偷到%d%s。\n\n\033[32m    你的身法提高了！\033[m",
			     uident, money, MONEY_NAME);
			if (myInfo->luck > -100) {
				myInfo->luck = MAX((myInfo->luck - 3), -100);
				prints("\n\033[31m    你的人品降低了！");
			}
			sprintf(genbuf,
				"你去看《天下无贼》，结果在电影院被偷走 %d%s，真是欲哭无泪啊。\n",
				money, MONEY_NAME);
			sprintf(buf, "你遇到窃贼");
		} else if (random() % 3) {
			myInfo->begExp += 5;
			myInfo->health -= 30;
			prints
			    ("哈哈，成功了！你用刀片在%s的衣兜割了一个口子,可惜摸出来的全是草纸。\n 你恨恨的写上，请不要开这样的玩笑，影响正常工作，塞了回去。 \n\n\033[32m    你的身法提高了！\033[m",
			     uident);
			if (myInfo->luck > -100) {
				myInfo->luck = MAX((myInfo->luck - 3), -100);
				prints("\n\033[31m    你的人品降低了！");
			}
			pressanykey();
			return 0;
		} else {
			money = MIN(myInfo->cash / 2, 5000000);
			mcuInfo->cash += money;
			myInfo->cash -= money;
			myInfo->begExp = MAX((myInfo->begExp - 10), 0);
			myInfo->health -= 40;
			prints
			    ("哈哈，成功了！你用刀片在%s的衣兜割了一个口子，偷到...咦？\n    一张字条：“21世纪最缺什么？。。。。”\n    你遇到黎叔，损失了%d%s...\n\n\033[31m    你的身法大降！\033[m",
			     uident, money, MONEY_NAME);
			sprintf(genbuf,
				"%s 偷鸡不成反蚀把米，被你顺手牵羊拿走 %d%s，嘿嘿～\n",
				currentuser->userid, money, MONEY_NAME);
			sprintf(buf, "移花接木成功");
		}
	}
	if (mcuInfo->GetLetter == 1)
		system_mail_buf(genbuf, strlen(genbuf), uident, buf,
				currentuser->userid);
UNMAP:
	unloadData(mcuInfo, sizeof (mcUserInfo));
	pressanykey();
	return 1;
}

static int
money_pat()
{
	int r, x, y, num, count = 0;
	char uident[IDLEN + 1], buf[256];
	struct userec *lookupuser;
	mcUserInfo *mcuInfo;

	sprintf(buf, "%s黑帮", CENTER_NAME);
	money_show_stat(buf);
	move(4, 4);
	if (check_health(5, 12, 4, "您的体力不够了！", YEA))
		return 0;
	prints("这里的板砖质地优良，拿去拍人一定痛快。\n"
	       "   现在大酬宾，一块板砖才 %d %s。", BRICK_PRICE, MONEY_NAME);
	move(6, 4);
	usercomplete("你要拍谁:", uident);
	if (uident[0] == '\0')
		return 0;
	if (!getuser(uident, &lookupuser)) {
		showAt(7, 4, "错误的使用者代号...", YEA);
		return 0;
	}

	if (!strcmp(uident, currentuser->userid)) {
		showAt(7, 4, "牛魔王：“老婆～快来看神经病啦～”", YEA);
		return 0;
	}
	if (!t_search(uident, NA, 1)) {
		showAt(7, 4, "你等的花儿都谢了，目标还是没有出现。", YEA);
		return 0;
	}
	count = userInputValue(7, 4, "拍", "块砖", 1, 100);
	if (count < 0)
		return 0;
	num = count * BRICK_PRICE;
	if (myInfo->cash < num) {
		move(9, 4);
		prints("您的钱不够...需要 %d %s", num, MONEY_NAME);
		pressanykey();
		return 0;
	}
	if (myInfo->luck > -100) {
		myInfo->luck = MAX((myInfo->luck - 1), -100);
		prints("\033[31m    你的人品降低了！\033[m");
	}
	myInfo->cash -= num;
	mcEnv->Treasury += num;	//拍砖的钱流回国库
	move(10, 8);
	prints("经过几天的偷窥和跟踪，你发现每天早上7点10分%s会路过僻静的",
	       uident);
	move(11, 4);
	prints("三角地。今天你拿着买来的板砖，准备行动了...");
	move(12, 8);
	prints("拍砖是很危险的喔！ 搞不好会出人命的，小心啊！");
	move(13, 4);
	if (askyn("废话少说，你还想拍么？", YEA, NA) == NA) {
		move(15, 4);
		myInfo->robExp = MAX((myInfo->robExp - 1), 0);
		update_health();
		myInfo->health--;
		myInfo->Actived++;
		prints
		    ("唉，最后关头你害怕了，所以不拍了。\n    你的胆识减少了。");
		pressanykey();
		return 0;
	}
#if 0
	x = countexp(currentuser);
	y = countexp(lookupuser);
#endif
	sethomefile(buf, uident, "mc.save");
	if (!file_exist(buf))
		initData(1, buf);
	if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
		return -1;

	if (mcuInfo->GetLetter == 0) {
		clear();
		showAt(6, 4, "人家不愿意跟你玩，惹不起，躲还不行啊？", YEA);
		unloadData(mcuInfo, sizeof (mcUserInfo));
		return 0;
	}

	x = myInfo->robExp * myInfo->robExp + 1;
	y = mcuInfo->robExp * mcuInfo->robExp;
	r = (double) x *100 / (x + y + 1);	//修正了胆识>4634 时会因为溢出而被反拍的bug
	num = count * BRICK_PRICE / 2 + 6000;	//医药费 
	update_health();
	myInfo->health -= 5;
	myInfo->Actived += 2;
	move(16, 4);
	if (r + count < 101) {	//目标胆识大于自己约10倍就会必反弹，嘿嘿 
		prints("很不幸，你没有拍中。反而被砸中小脑袋瓜...");
		move(17, 4);
		prints("你流血不止，被送到医院缝了十多针，好惨啊！");
		move(18, 4);
		if (random() % 2) {
			myInfo->robExp = MAX((myInfo->robExp - 1), 0);
			prints("你的胆识减少了。\n    ");
		}
		if (random() % 2) {
			myInfo->begExp = MAX((myInfo->begExp - 1), 0);
			prints("你的身法降低了。\n    ");
		}
		mcEnv->Treasury += MIN(myInfo->cash, num * 2);
		myInfo->cash = MAX((myInfo->cash - num * 2), 0);
		update_health();
		myInfo->health = 0;
		prints("最后还结了 %d %s的医药费，看你以后还敢不。", num * 2,
		       MONEY_NAME);
		pressanykey();
		return 0;
	}
	if (myInfo->begExp > mcuInfo->begExp * 20 + 50) {
		myInfo->luck--;
		prints("对方那么弱你居然忍心下手！\n    你的人品降低了。");
		pressanykey();
		return 0;
	}
	if (mcuInfo->guard > 0 && rand() % 3) {
		prints("斜刺里突然冲出一只大狼狗，你一慌神，板砖全砸空了。\n"
		       "    哇，大狼狗朝你扑过来了！");
		if (askyn("你要不要跑？", YEA, NA) == YEA) {
			if (random() % 2) {
				myInfo->robExp = MAX((myInfo->robExp - 1), 0);
				prints("你的胆识减少了。\n    ");
			}
			if (random() % 4) {
				prints
				    ("\n    你一看不妙，飞快的钻进一条小巷跑掉了。好险啊～");
				pressanykey();
				return 0;
			} else {
				prints
				    ("\n    你居然跑进了一条死胡同，狼狗跟上来了......\n 一阵天翻地覆之后......");
				if (random() % 3) {
					if (myInfo->begExp) {
						myInfo->begExp--;
						prints
						    ("\n    你的身法降低了！");
					}
					update_health();
					myInfo->health = myInfo->health / 2;
					prints("\n    你的体力减半！");
					sprintf(buf,
						"%s被你的大狼狗咬伤，记得奖励狼狗几块骨头哦！",
						currentuser->userid);
					if (mcuInfo->GetLetter == 1)
						system_mail_buf(buf,
								strlen(buf),
								uident,
								"你的大狼狗成功保护你",
								currentuser->
								userid);
				}
			}
		}

		if (random() % 3) {
			mcuInfo->guard--;
			myInfo->health -= 5;
			sleep(1);
			prints("\n    经过激烈搏斗，你终于干掉了大狼狗，哈哈");
			sprintf(buf,
				"你的一只大狼狗被%s干掉了，你现在还剩%d只大狼狗。",
				currentuser->userid, mcuInfo->guard);
			if (mcuInfo->GetLetter == 1)
				system_mail_buf(buf, strlen(buf), uident,
						"你的一只大狼狗壮烈牺牲",
						currentuser->userid);
			if (!(random() % 4)) {
				myInfo->robExp++;
				prints("\n    你的胆识增加了！");
			}
			prints("\n    你气喘吁吁，体力下降！");
		} else {
			sleep(1);
			prints
			    ("\n    你虽奋力搏斗，最后还是被大狼狗在腿上咬了一口。");
			if (random() % 3) {
				if (myInfo->begExp) {
					myInfo->begExp--;
					prints("\n    你的身法降低了！");
				}
				update_health();
				myInfo->health = myInfo->health / 2;
				prints("\n    你的体力减半！");
				sprintf(buf,
					"%s被你的大狼狗咬伤，记得奖励狼狗几块骨头哦！",
					currentuser->userid);
				if (mcuInfo->GetLetter == 1)
					system_mail_buf(buf, strlen(buf),
							uident,
							"你的大狼狗成功保护你",
							currentuser->userid);
			}
		}
		goto UNMAP;
	}
	if ((random() % 5 || (myInfo->begExp > mcuInfo->begExp * 10 + 50)) &&
	    !(mcuInfo->begExp > myInfo->begExp * 10 + 50)) {
		prints("你这坏蛋，背后偷袭，砸中%s的小脑袋瓜。", uident);
		if (mcuInfo->cash < num) {
			move(17, 4);
			update_health();
			mcuInfo->health = 0;
			mcEnv->Treasury += mcuInfo->cash;
			mcuInfo->cash = 0;
			prints("你都拍到人家没钱了治伤了...积点阴德吧！");
			sprintf(buf,
				"你被%s拍了板砖，你没钱治伤，只能咬牙忍痛...",
				currentuser->userid);
			if (!(random() % 10)) {
				myInfo->robExp++;
				prints("\n    你的胆识增加了！");
			}
			if (!(random() % 10)) {
				myInfo->begExp++;
				prints("\n    你的身法增加了！");
			}
			if (!(random() % 4))
				mcuInfo->begExp = MAX(0, mcuInfo->begExp--);
		} else {
			mcEnv->Treasury += num;
			mcuInfo->cash -= num;
			mcuInfo->health -= 10;
			if (!(random() % 5))
				mcuInfo->begExp = MAX(0, mcuInfo->begExp--);
			move(17, 4);
			prints("哈哈，%s花了%d%s治伤，在医院里躺了好多天！",
			       uident, num, MONEY_NAME);
			sprintf(buf, "你被%s拍了板砖，花了%d%s治伤，呜呜呜...",
				currentuser->userid, num, MONEY_NAME);
			if (!(random() % 4)) {
				myInfo->robExp++;
				prints("\n    你的胆识增加了！");
			}
			if (!(random() % 4)) {
				myInfo->begExp++;
				prints("\n    你的身法增加了！");
			}
		}
		if (mcuInfo->GetLetter == 1)
			system_mail_buf(buf, strlen(buf), uident,
					"你被拍了板砖", currentuser->userid);
	} else {
		if (random() % 2) {
			mcuInfo->begExp++;
			sprintf(buf,
				"%s拿板砖拍你落空，你的身法增加了，哦耶～\n",
				currentuser->userid);
		} else {
			mcuInfo->robExp++;
			sprintf(buf,
				"%s拿板砖拍你落空，你的胆识增加了，哦耶～\n",
				currentuser->userid);
		}
		if (mcuInfo->GetLetter == 1)
			system_mail_buf(buf, strlen(buf), uident,
					"你躲过板砖袭击", currentuser->userid);
		prints("啊呀呀，没拍中。。。");

	}

UNMAP:
	unloadData(mcuInfo, sizeof (mcUserInfo));
	pressanykey();
	return 0;
}

static void
RobShop()
{
	int num;
	char buf[256], title[STRLEN];
	time_t currtime;

	if (check_health(90, 12, 4, "这么大的事情没有充沛体力无法完成。", YEA))
		return;
	if (myInfo->robExp < 200) {
		showAt(12, 4, "你犹豫了半天，还是没敢动手。。。", YEA);
		return;
	}
	if (myInfo->begExp < 100) {
		showAt(12, 4,
		       "你刚想动手，结果刚站起来就被边上的凳子绊倒了。。。",
		       YEA);
		return;
	}
	if (myInfo->luck < 60) {
		showAt(12, 4, "你总觉得背后有双眼睛在盯着你看。", YEA);
		return;
	}

	nomoney_show_stat("黑市");
	move(4, 4);
	currtime = time(NULL);
	if (myInfo->WorkPoint < 4500) {
		showAt(12, 4,
		       "刚才出了个案子，有警察在附近巡视，先不要动手为好。",
		       YEA);
		return;
	}

	if (askyn("武装抢劫就要买武器，装备需要20万，你确定要买吗？", NA, NA) ==
	    NA) {
		myInfo->robExp--;
		prints("\n    你决定不买了。\n    你的胆识降低！");
		return;
	}
	if (myInfo->cash < 200000) {
		showAt(12, 4, "一手交钱一手交货，你没带足够的现金。", YEA);
		return;
	}
	update_health();
	myInfo->health -= 10;
	prints("\n    你拎上%s牌冲锋枪，穿上%s牌小马甲，哇！帅呆了！",
	       CENTER_NAME, CENTER_NAME);
	pressanykey();

	nomoney_show_stat("赌场大厅");
	move(4, 4);
	if (askyn("抢赌场很危险的，你真的要动手吗？", YEA, NA) == NA) {
		myInfo->robExp -= 2;
		prints
		    ("\n    你决定放弃了！还偷偷把装备扔进了附近的垃圾桶。\n    你的胆识降低！");
		pressanykey();
		return;
	}

	myInfo->lastActiveTime = currtime;
	update_health();
	myInfo->WorkPoint -= 4500;
	myInfo->health -= 50;
	myInfo->luck -= 10;
	myInfo->Actived += 10;

	move(6, 4);
	prints
	    ("你一把亮出冲锋枪，对着赌客大喊：”喂～安静点，没看这里抢劫吗？！“");
	pressanykey();

	sleep(1);

	move(8, 4);
	if (!(random() % 3)) {
		myInfo->robExp -= 10;
		prints
		    ("周围的人哈哈大笑，神经病，你拿把水枪干吗啊？！\n    啊，被黑店骗了！你转身就跑。\n    你的体力减半\n    你的胆识减少10点。");
		sprintf(buf,
			"    一个拿着水枪身穿马甲的神经病在赌场发飙后逃逸。\n"
			"    希望有知情者能向警方提供有关消息。\n");
		deliverreport("【新闻】神经病赌场发飙", buf);
		pressreturn();
		return;
	}

	update_health();
	if (check_chance(myInfo->robExp, 600, myInfo->weapon, 5, 100, 0)) {
		num = MIN(MAX_MONEY_NUM / 20, mcEnv->prize777) / 2;
		myInfo->cash += num - 200000;
		mcEnv->prize777 -= (num - after_tax(200000));
		myInfo->begExp += 20;
		myInfo->robExp += 30;
		prints
		    ("\n    乘着周围的人目瞪口呆之际，你拿出布袋把777台上的钱往里装满。\n    然后潇洒的一转身走出大门。\n    你的胆识增加！\n    你的身法增加！");
		sprintf(title, "【新闻】蒙面人武装抢劫赌场");
		sprintf(buf,
			"    本站刚刚收到的消息：早些时候，一蒙面人武装抢劫赌场后逃逸。\n"
			"    目前警署正式介入调查此事。希望正义市民提供相关线索。\n");

	} else if (random() % 3) {
		myInfo->cash -= 200000;
		mcEnv->prize777 += after_tax(200000);
		myInfo->health = 0;
		myInfo->robExp += 30;
		prints
		    ("\n    刷刷刷，周围出现一堆全副武装的警察，原来警方早就得到了线报，cmft"
		     "\n    你立刻拿起冲锋枪一顿乱扫,趁着警察躲闪的时候慌忙逃出赌场"
		     "\n    你的体力全失！ \n    你的胆识增加了!\n");
		sprintf(title, "【新闻】蒙面歹徒赌场行凶");
		sprintf(buf,
			"    本站刚刚收到的消息：警方根据早先得到的线报埋伏在%s赌场。但由于歹徒火力过于凶猛，未能将其擒获。\n\n"
//                      "    目前警署正式介入调查此事。希望正义市民提供相关线索。\n"
			"    广告后请继续关注%s新闻节目\n"
			"    广告:%s牌冲锋枪，携带方便，火力一流，居家旅游，杀人放火之必备。",
			CENTER_NAME, CENTER_NAME, CENTER_NAME);
	} else {
		myInfo->health = 0;
		myInfo->robExp -= 20;
		myInfo->luck -= 20;
		prints
		    ("\n    刷刷刷，周围出现一堆全副武装的警察，你只好束手就擒！\n    原来警方早就得到了线报，cmft\n"
		     "    你的体力全失！\n    你的人品降低！\n    你的胆识减少20点!\n");
		sprintf(title, "【新闻】警方守株待兔 %s束手就擒",
			currentuser->userid);
		sprintf(buf,
			"    警方根据早先得到的线报埋伏在%s赌场，在%s武装抢劫的时候，成功将其擒获\n"
			"    在审讯过程中发现%s为精神病患者，所以免于刑事诉讼，取保就医。\n",
			CENTER_NAME, currentuser->userid, currentuser->userid);
	}
	deliverreport(title, buf);
	pressanykey();

	return;

}

void
stealbank()
{
	int num;
	struct userec *lookupuser;
	char uident[IDLEN + 1], buf[256], title[STRLEN], police[IDLEN + 1];
	mcUserInfo *mcuInfo;
	time_t currtime;

	if (check_health(90, 12, 4, "这么大的事情没有充沛体力无法完成。", YEA))
		return;
	if (myInfo->robExp < 100) {
		showAt(12, 4, "别怕别怕。。。你的手抖的太厉害了。。。", YEA);
		return;
	}
	if (myInfo->begExp < 200) {
		showAt(12, 4, "凭你现在的身法，必死无疑！再去练练吧。。。",
		       YEA);
		return;
	}
	if (myInfo->luck < 60) {
		showAt(12, 4, "你根本打听不到关键的情报。。。", YEA);
		return;
	}

	currtime = time(NULL);
	if (myInfo->WorkPoint < 4500) {
		showAt(12, 4, "银行系统刚刚换过，你还要花点时间熟悉一下。",
		       YEA);
		return;
	}
	move(4, 4);
	if (askyn
	    ("入侵银行系统需要购买电脑，软件，需要20万，你确定要买吗？", NA,
	     NA) == NA) {
		myInfo->begExp--;
		prints("\n    你决定不买了。\n    你的身法降低！");
		return;
	}
	myInfo->health -= 10;

	if (myInfo->cash < 200000) {
		showAt(12, 4, "一手交钱一手交货，你没带足够的现金。", YEA);
		return;
	}

	myInfo->lastActiveTime = currtime;
	update_health();
	myInfo->WorkPoint -= 4500;
	myInfo->health -= 50;
	myInfo->luck -= 10;
	myInfo->Actived += 10;
	myInfo->cash -= 200000;
	mcEnv->prize777 += after_tax(200000);
	move(5, 4);
	prints
	    ("你坐在电脑前面，从容的点上一根烟，然后快速敲了几下键盘。\n    屏幕上显示：开始入侵%s银行系统！",
	     CENTER_NAME);
	pressanykey();
	move(7, 4);
	prints("正在连接。。。请稍候");
	sleep(1);

	move(8, 4);
	if (!(random() % 3)) {
		myInfo->begExp -= 10;
		prints
		    ("嘟嘟嘟，银行的超级计算机发现了你的入侵，你赶紧断开了连接。好险！\n    你的体力减半\n    你的身法减少10点。");
		sprintf(buf,
			"    超级计算机刚刚监视到了一次入侵银行系统的行为，在试图将入侵者锁定的时候，\n"
			"    被其察觉而逃脱。希望有知情者能向警方提供有关消息。\n");
		deliverreport("【新闻】不明人士入侵银行系统失败", buf);
		pressreturn();
		return;
	}
	prints
	    ("恭喜你！已经成功连接到了银行转帐系统。\n    你有一次机会从别人的帐户转帐到你帐户。\n");
	usercomplete("你要从谁哪里转帐过来？按Enter取消。", uident);
	if (uident[0] == '\0') {
		myInfo->robExp -= 10;
		prints
		    ("/n    你胆怯放弃了。。。。\n    你的体力减半\n    你的胆识减少10点。");
		return;
	}
	if (!getuser(uident, &lookupuser)) {
		myInfo->begExp -= 10;
		showAt(12, 4,
		       "/n    你手一抖输错了，还把热咖啡碰倒在裤子上了\n    你的体力减半\n    你的身法减少10点。",
		       YEA);
		return;
	}
	if (!strcmp(uident, currentuser->userid)) {
		showAt(7, 4, "啊哦，青山医院欢迎您～！”", YEA);
		myInfo->robExp -= 10;
		myInfo->begExp -= 10;
		myInfo->luck -= 10;
		return;
	}
	sethomefile(buf, uident, "mc.save");
	if (!file_exist(buf))
		initData(1, buf);
	if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
		return;

	if (mcuInfo->GetLetter == 0) {
		clear();
		showAt(6, 4, "人家不愿意跟你玩，惹不起，躲还不行啊？", YEA);
		unloadData(mcuInfo, sizeof (mcUserInfo));
		return;
	}

/*	sprintf(buf, "要从%s的帐户转过来", uident);
	num = userInputValue(12, 4, buf, MONEY_NAME, 0, num);
	if ((num > (myInfo->credit + myInfo->cash) * 5)
	    || (myInfo->begExp * 5 + 50 < mcuInfo->begExp))
		if (random() % 4) {
			sleep(1);
			goto FAILED;
		}
*/
	sleep(1);
	num = MIN(MIN(MAX_MONEY_NUM / 20, mcuInfo->credit / 5), myInfo->credit);
	if (check_chance
	    (myInfo->begExp, mcuInfo->begExp, myInfo->weapon, mcuInfo->armor,
	     200, 0)) {
		mcuInfo->credit -= num;
		myInfo->credit += after_tax(num);
		myInfo->begExp += 25;
		myInfo->robExp += 20;
		myInfo->health = 0;
		if ((mcuInfo->robExp + mcuInfo->begExp < 100)
		    && (mcuInfo->begExp * 100 + 500 < myInfo->begExp)) {
			prints
			    ("\n    此人年纪轻轻，居然拥有巨额不明财富，你感到十分嫉妒!");
			if (askyn("\n	一不做二不休，大家一拍两散？", NA, NA)
			    == YEA) {
				prints
				    ("\n    我得不到别人也别想得到，你决定走之前在他的账户里种下一个木马"
				     "\n    你的人品下降20点!");
				myInfo->luck -= 20;
				mcEnv->Treasury += mcuInfo->credit;
				num += mcuInfo->credit;
				mcuInfo->credit = 0;
				if (readstrvalue
				    (MC_BOSS_FILE, "police", police,
				     IDLEN + 1) != 0)
					police[0] = '\0';
				sprintf(title, "丐帮内线消息");
				sprintf(buf,
					"    %s不幸被%s侵入银行帐户，并植入了木马。",
					uident, currentuser->userid);
				system_mail_buf(buf, strlen(buf), police, title,
						"deliver");
			}
		}
		sprintf(buf,
			"上次去银行查询，发现少了%d的存款，银行居然说我记错了，kao。"
			"\n上次%s喝醉了，自吹可以入侵银行系统，不知道是不是和他有关系。",
			num, currentuser->userid);
		if (mcuInfo->GetLetter == 1)
			system_mail_buf(buf, strlen(buf), uident, "日记",
					"deliver");
		prints("\n    恭喜你！转帐成功！\n    你的胆识增加！\n"
		       "    你的身法增加！\n    你的体力耗尽！\n");
		sprintf(title, "【谣言】可疑人物入侵银行转帐系统");
		sprintf(buf,
			"    据坊间流传的谣言，有人成功入侵了银行系统。\n    但银行的超级计算机并无监视到入侵行为。\n"
			"    目前警署正在调查此消息的真实度。请各位关注自己的帐户，及时提供相关消息。\n");
	} else if (random() % 3) {
		myInfo->health = 0;
		myInfo->begExp += 25;
		prints
		    ("    嘟嘟嘟，又是讨厌的网络巡警。算了，这次就放过他吧,这次入侵就当练手吧。\n"
		     "    你的体力归零！\n    你的身法增加了!\n");
		sprintf(title, "【新闻】可疑人物企图入侵银行系统");
		sprintf(buf,
			"    本报刚刚接到的消息：在今天凌晨，曾有一可疑人物企图入侵%s银行系统,但被网络安全刑警成功阻截。\n"
			"    在此，我们%s银行系统郑重向广大新老用户承诺,我们的系统是绝对安全可靠的。\n",
			CENTER_NAME, CENTER_NAME);
	} else {
		myInfo->health = 0;
		myInfo->begExp -= 20;
		myInfo->luck = MAX(-100, myInfo->luck - 20);
		num = MIN(MAX_MONEY_NUM / 2, myInfo->credit / 5);
		mcuInfo->credit += after_tax(num / 2);
		mcEnv->prize777 += num / 2;
		myInfo->credit -= num;
		prints
		    ("    超级计算机监视到了你的可疑行动，你被抓住了！\n    你的体力归零！\n"
		     "    你的人品减少20点！\n    你的身法减少20点!\n    你被罚款%d%s!",
		     num, MONEY_NAME);
		sprintf(title, "【新闻】某人入侵银行系统被抓获");
		sprintf(buf,
			"    超级计算机刚刚监视到了某人入侵银行系统的行为，并成功将其锁定！\n"
			"    鉴于其认罪态度较好，本着治病救人的原则，不公布姓名。罚款%d%s!\n"
			"    其中一半归受害者做为压惊费，一半注入777。    \n    诸君引以为戒！\n",
			num, MONEY_NAME);
	}
	deliverreport(title, buf);
	unloadData(mcuInfo, sizeof (mcUserInfo));
	pressanykey();

	return;

}

int
RobPeople(int type)
{
	int transcash, transcredit, transrob, transbeg;
	time_t currtime;
	char uident[IDLEN + 1], buf[256], title[70], content[256],
	    police[IDLEN + 1];
	mcUserInfo *mcuInfo;

	switch (type) {
	case 0:
		showAt(4, 4, "黑帮成员就是要无恶不作。", YEA);
		if (!getOkUser("你想绑架谁？", uident, 5, 4)) {
			showAt(7, 4, "查无此人", YEA);
			return 0;
		}
		if (!strcmp(uident, currentuser->userid)) {
			showAt(7, 4, "牛魔王：“老婆～快来看神经病啦～”", YEA);
			return 0;
		}
		move(6, 4);
		if (askyn("你确定要绑架他吗？", NA, NA) == NA) {
			showAt(7, 4, "什么？你还没想好？？想好了再来！！", YEA);
			return 0;
		}
		update_health();
		if (check_health(100, 7, 4, "你没有那么多体力绑架别人。", YEA)) {
			return 0;
		}
		if (myInfo->robExp < 1000 || myInfo->begExp < 500) {
			showAt(7, 4, "你没有足够的经验来干这么大的事情。", YEA);
			return 0;
		}
		if (myInfo->luck < 80) {
			showAt(7, 4,
			       "由于你有前科，警察时刻监控着你的活动，你没法行动",
			       YEA);
			return 0;
		}
		if (myInfo->cash < 300000) {
			showAt(7, 4,
			       "现在干点活不容易，干啥不需要钱啊，两手空空能干嘛？",
			       YEA);
			return 0;
		}
		currtime = time(NULL);
		if (myInfo->WorkPoint < 9000) {
			showAt(7, 4,
			       "你刚做了个大案子，还是继续躲避一下风声吧。",
			       YEA);
			return 0;
		}
		showAt(7, 4, "你看见有警察在他的住所外面巡逻。", YEA);
		myInfo->lastActiveTime = currtime;
		myInfo->WorkPoint -= 9000;
		myInfo->luck -= 10;
		if (random() % 2) {
			showAt(8, 4, "你被迫取消了绑架行动。", YEA);
			sprintf(title, "【黑帮】有人在策划绑架行动");
			sprintf(content, "    大家要多加小心。");
			deliverreport(title, content);
			return 0;
		}
		if (askyn("你还要继续进行绑架行动吗？", NA, NA) == NA) {
			showAt(8, 4, "你被迫取消了绑架行动。", YEA);
			myInfo->luck -= 10;
			myInfo->health -= 50;
			sprintf(title, "【黑帮】有人预谋绑架别人");
			sprintf(content,
				"    幸亏他在" MY_BBS_NAME
				"上走漏了风声，在大家的教诲下他改邪归正了。");
			deliverreport(title, content);
			return 0;
		}
		myInfo->Actived += 20;
		showAt(8, 4,
		       "你偷偷塞了300000元给门口的警察，然后放心的摸了进去。",
		       YEA);
		myInfo->cash -= 300000;
		mcEnv->prize777 += after_tax(300000);
		sethomefile(buf, uident, "mc.save");
		if (!file_exist(buf))
			initData(1, buf);
		if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
			return 0;
		if (mcuInfo->GetLetter == 0) {
			clear();
			showAt(6, 4, "人家不愿意跟你玩，惹不起，躲还不行啊？",
			       YEA);
			unloadData(mcuInfo, sizeof (mcUserInfo));
			return 0;
		}

		if (check_chance
		    (myInfo->robExp, mcuInfo->robExp, myInfo->weapon,
		     mcuInfo->armor, -200, 0)) {
			if (random() % 3) {
				myInfo->robExp += 30;
				myInfo->begExp += 30;
				myInfo->health = 0;
				move(10, 0);
				prints
				    ("    摸进去以后一看，你倒吸一口冷气，190cm的大块头啊\n"
				     "    这个，好像忘记带打大象用的麻醉枪了\n"
				     "    算了，钱都花出去了，拼了吧\n"
				     "    一番激烈的搏斗。\n"
				     "    不好，好像惊动警察了，跑吧!\n"
				     "    你的胆识和身法上升! 你的体力耗尽!");
				sprintf(title, "【黑帮】 %s绑架失败",
					currentuser->userid);
				sprintf(content,
					"    黑帮成员%s妄图绑架他人，幸得当事者奋力反抗，未能得逞",
					currentuser->userid);
				deliverreport(title, content);
				sprintf(title, "%s企图绑架你",
					currentuser->userid);
				sprintf(content,
					"   不过你多年锻炼的强壮体魄也不是吃素的。他想绑架？再等几年吧！");
				if (mcuInfo->GetLetter == 1)
					system_mail_buf(content,
							strlen(content), uident,
							title,
							currentuser->userid);
				unloadData(mcuInfo, sizeof (mcUserInfo));
				pressanykey();
				return 0;
			} else {
				myInfo->robExp -= 30;
				myInfo->begExp -= 30;
				myInfo->health = 0;
				myInfo->luck -= 20;
				myInfo->mutex = 0;
				myInfo->freeTime = time(NULL) + 10800;
//                      myInfo->credit += myInfo->cash;
//                      myInfo->cash = 0;
				move(10, 0);
				prints
				    ("    你被警察抓住了！门口那家伙居然是个假货！！\n"
				     "    你的胆识和身法各降30点！\n"
				     "    你的体力降为0！\n"
				     "    你的人品下降20点！\n"
				     "    你被警察关押3小时！\n"
				     "    你偷鸡不成反蚀把米，真是欲哭无泪啊！");
				sprintf(title, "【黑帮】 %s绑架失败",
					currentuser->userid);
				sprintf(content,
					"    黑帮成员%s妄图绑架他人，幸好被警察及时发现。\n"
					"    他被警察关押3小时。",
					currentuser->userid);
				deliverreport(title, content);
				unloadData(myInfo, sizeof (mcUserInfo));
				unloadData(mcEnv, sizeof (MC_Env));
				pressanykey();
				Q_Goodbye();
			}
		}
		myInfo->robExp += 50;
		myInfo->begExp += 50;
		myInfo->luck -= 10;
		transcash = MIN(myInfo->cash * 2, mcuInfo->cash);
		transcredit = MIN(myInfo->credit, mcuInfo->credit / 4);
		mcuInfo->cash -= transcash;
		mcuInfo->credit -= transcredit;
		myInfo->cash += after_tax(transcash);
		myInfo->credit += after_tax(transcredit);
		myInfo->health = 0;
		move(9, 0);
		prints("    你绑架成功！\n"
		       "    你的胆识和身法各加50点！   你的人品下降10点！\n"
		       "    %s身上的现金被你洗劫一空，家人还拿出了%d%s为他赎身。\n"
		       "    经历了这么大的事，你虚脱了。", uident, transcredit,
		       MONEY_NAME);
		if ((mcuInfo->robExp + mcuInfo->begExp < 100)
		    && (mcuInfo->robExp * 100 + 500 < myInfo->robExp)) {
			if (askyn
			    ("\n 	这家伙已经看到我的脸了，反正钱已到手，杀了算了？",
			     NA, NA) == YEA) {
				prints
				    ("\n    虽然拿到了赎金，但是心狠手辣的你还是决定一不做，二不休，撕票了!"
				     "\n    你的人品下降20点!");
				myInfo->luck -= 20;
				mcEnv->Treasury += mcuInfo->credit;
				mcuInfo->credit = 0;
				sprintf(title, "【黑帮】 黑帮残暴不仁");
				sprintf(content,
					"    %s不幸被黑帮绑架，虽按期缴纳了赎身费仍被撕票。\n"
					"    %s警署对此恶性事件给予高度重视，已成立专案组，限期破案。",
					uident, CENTER_NAME);
				deliverreport(title, content);
				sprintf(title, "你惨遭撕票!");
				sprintf(content,
					"    你不幸被黑帮绑架，交出赎身费以后仍被撕票，死不瞑目啊!\n    变鬼也不放过这些坏家伙!!!");
				if (mcuInfo->GetLetter == 1)
					system_mail_buf(content,
							strlen(content), uident,
							title,
							currentuser->userid);
				if (readstrvalue
				    (MC_BOSS_FILE, "police", police,
				     IDLEN + 1) != 0)
					police[0] = '\0';
				sprintf(title, "黑帮内线消息");
				sprintf(content,
					"    %s不幸被%s绑架，交出赎身费以后仍被撕票。",
					uident, currentuser->userid);
				system_mail_buf(content, strlen(content),
						police, title, "deliver");
				unloadData(mcuInfo, sizeof (mcUserInfo));
				pressanykey();
				return 1;
			}
		}
		sprintf(title, "【黑帮】 黑帮又实施一次绑架行动");
		sprintf(content,
			"    %s不幸被黑帮绑架，交出了财产的1/4作为赎身费才重获自由。",
			uident);
		deliverreport(title, content);
		sprintf(title, "你被黑帮绑架！");
		sprintf(content, "    你不幸被黑帮绑架，交了%d的赎身费",
			transcash + transcredit);
		if (mcuInfo->GetLetter == 1)
			system_mail_buf(content, strlen(content), uident, title,
					currentuser->userid);
		unloadData(mcuInfo, sizeof (mcUserInfo));
		pressanykey();
		break;
	case 1:
		showAt(4, 4,
		       "丐帮弟子总是受欺压，你练成了吸星大法有他们好看的！",
		       YEA);
		if (!getOkUser("你要向谁下手？", uident, 5, 4)) {
			showAt(7, 4, "查无此人", YEA);
			return 0;
		}
		if (!strcmp(uident, currentuser->userid)) {
			showAt(7, 4, "牛魔王：“老婆～快来看神经病啦～”", YEA);
			return 0;
		}
		move(6, 4);
		if (askyn("你确定要对他施展吸星大法吗？", NA, NA) == NA) {
			showAt(7, 4, "哦？你还不忍心下手？把心肠练硬了再来吧。",
			       YEA);
			return 0;
		}
		update_health();
		if (check_health
		    (110, 7, 4, "你没有那么多体力施展吸星大法。", YEA)) {
			return 0;
		}
		if (myInfo->robExp < 3000 || myInfo->begExp < 5000) {
			showAt(7, 4, "你的神功还差点火候，继续修炼吧。", YEA);
			return 0;
		}
		if (myInfo->luck < 85) {
			showAt(7, 4,
			       "由于你劣迹斑斑，他不和你接近，你没法下手。",
			       YEA);
			return 0;
		}
		currtime = time(NULL);
		if (myInfo->WorkPoint < 12000) {
			showAt(7, 4, "你还需要继续运功才能施展吸星大法。", YEA);
			return 0;
		}
		showAt(7, 4, "你感觉不到他的内息，难道他练成了金钟罩铁布衫？",
		       YEA);
		myInfo->lastActiveTime = currtime;
		myInfo->WorkPoint -= 12000;
		myInfo->luck -= 10;
		if (random() % 2) {
			showAt(8, 4, "你打消了施展吸星大法的念头。", YEA);
			sprintf(title, "【丐帮】有人在修炼吸星大法");
			sprintf(content, "    大家要多加小心，不要着了道。");
			deliverreport(title, content);
			return 0;
		}
		if (askyn("你还要再尝试一下吗？", NA, NA) == NA) {
			showAt(8, 4, "你打消了施展吸星大法的念头。", YEA);
			myInfo->luck -= 20;
			myInfo->health -= 50;
			sprintf(title, "【丐帮】有人在施展吸星大法");
			sprintf(content,
				"    幸亏他悬崖勒马，没有继续做伤天害理的事。");
			deliverreport(title, content);
			return 0;
		}
		myInfo->Actived += 30;
		sethomefile(buf, uident, "mc.save");
		if (!file_exist(buf))
			initData(1, buf);
		if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
			return 0;

		if (mcuInfo->GetLetter == 0) {
			clear();
			showAt(6, 4, "人家不愿意跟你玩，惹不起，躲还不行啊？",
			       YEA);
			unloadData(mcuInfo, sizeof (mcUserInfo));
			return 0;
		}

		if (mcuInfo->BeIn == 2) {
			showAt(7, 4, "帮主禁令，严禁内斗", YEA);
			unloadData(mcuInfo, sizeof (mcUserInfo));
			return 0;
		}

		if (!(seek_in_file(DIR_MC "policemen", uident)
		      || (mcuInfo->BeIn == 1))) {
			showAt(7, 4, "有仇报仇，丐帮成员恩怨分明。", YEA);
			unloadData(mcuInfo, sizeof (mcUserInfo));
			return 0;
		}

		if (mcuInfo->robExp + mcuInfo->begExp < 6000) {
			prints
			    ("    %s冷冷一笑，无招胜有招。根本无内力，何处可吸？"
			     "    你被气得吐血而归!", uident);
			myInfo->luck -= 20;
			myInfo->health = 0;
			sprintf(title, "【丐帮】有人施展了吸星大法");
			sprintf(content,
				"    幸亏受害者功力不深，未能造成严重后果。");
			deliverreport(title, content);
			unloadData(mcuInfo, sizeof (mcUserInfo));
			pressanykey();
			return 0;
		}

		if (check_chance
		    (myInfo->begExp, mcuInfo->begExp, myInfo->weapon,
		     mcuInfo->armor, -300, -50)) {
			transrob = MIN(myInfo->robExp / 100, mcuInfo->robExp);
			transbeg = MIN(myInfo->begExp / 100, mcuInfo->begExp);
			myInfo->robExp -= transrob;
			mcuInfo->robExp += transrob;
			myInfo->begExp -= transbeg;
			mcuInfo->begExp += transbeg;
			myInfo->luck -= 30;
			myInfo->health = 0;
			//myInfo->lastActiveTime = time(NULL) + 7200;
			myInfo->WorkPoint -= 7200;
			move(9, 0);
			prints
			    ("    不好!此人居然身兼最上乘的北冥神功，你的功力反而源源不断的被他吸去。\n"
			     "    你的胆识下降%d点，你的身法下降%d点。\n"
			     "    你的体力降为0！\n" "    你的人品下降20点！\n"
			     "    你偷鸡不成反蚀把米，真是欲哭无泪啊！",
			     transrob, transbeg);
			sprintf(title, "【丐帮】 %s施展吸星大法失败",
				currentuser->userid);
			sprintf(content,
				"    丐帮成员%s妄图使用吸星大法吸取他人功力，结果技不如人反被震伤。\n",
				currentuser->userid);
			deliverreport(title, content);
			sprintf(title, "%s企图对你使用吸星大法！",
				currentuser->userid);
			sprintf(content,
				"    幸亏你早已修炼了更为高深的北冥神功，反吸了他%d的胆识和%d的身法",
				transrob, transbeg);
			if (mcuInfo->GetLetter == 1)
				system_mail_buf(content, strlen(content),
						uident, title,
						currentuser->userid);
			unloadData(mcuInfo, sizeof (mcUserInfo));
			pressanykey();
			return 0;
		}
		transrob = MIN(myInfo->robExp, mcuInfo->robExp / 50);
		transbeg = MIN(myInfo->begExp, mcuInfo->begExp / 50);
		myInfo->robExp += transrob;
		mcuInfo->robExp -= transrob;
		myInfo->begExp += transbeg;
		mcuInfo->begExp -= transbeg;
		myInfo->luck -= 20;
		myInfo->health = 0;
		move(9, 0);
		prints("    你施展吸星大法成功！\n"
		       "    你的胆识增加%d点，身法增加%d点！\n"
		       "    你的人品下降20点！\n"
		       "    经历了这么大的事，你虚脱了。", transrob, transbeg);
//              unloadData(mcuInfo, sizeof (mcUserInfo));
		sprintf(title, "【丐帮】 %s不幸成为吸星大法的受害者", uident);
		sprintf(content, "%s被吸星大法偷袭，功力损失了2%%。", uident);
		deliverreport(title, content);
		sprintf(title, "你被吸星大法命中！");
		sprintf(content,
			"    你不幸被人施了吸星大法，损失了%d的胆识和%d的身法",
			transrob, transbeg);
		if (mcuInfo->GetLetter == 1)
			system_mail_buf(content, strlen(content), uident, title,
					currentuser->userid);
		unloadData(mcuInfo, sizeof (mcUserInfo));
		pressanykey();
		break;
	}
	return 1;
}

int
money_robber()
{
	int ch, quit = 0, tempMoney, day, hour, minute;
	time_t ActiveTime, currTime;
	char uident[IDLEN + 1];
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

	if (!(myInfo->GetLetter == 1)) {
		clear();
		showAt(5, 4, "你已经关闭了金融中心游戏功能，请开启后再来。",
		       YEA);
		return 0;
	}

	while (!quit) {
		money_show_stat("黑帮总部");
		move(4, 4);
		prints
		    ("两年前的%s黑帮无恶不作，名噪一时，被警察严打后有一段时间消声匿迹。"
		     "\n    不过最近又逐渐活跃起来，作案手段也趋于隐蔽化多样化。",
		     CENTER_NAME);
		move(7, 4);
		prints
		    ("一个黑衣人走过来小声说：“要板砖么？拍人很疼的。拍好了还能长胆识身法。”");
		move(t_lines - 1, 0);
#if 0
		prints
		    ("\033[1;44m 选单 \033[1;46m [1]拍砖 [2]交保护费 [3]黑帮活动 [4] 帮主号令 [Q]离开\033[m");
#else
		prints
		    ("\033[1;44m 选单 \033[1;46m [1]拍砖 [2]黑帮活动 [3] 帮主号令 [4] 交保护费 [Q]离开\033[m");
#endif
		ch = igetkey();
		switch (ch) {
		case '1':
			money_pat();
			break;
#if 0
		case '2':
			money_show_stat("黑帮保护费收费处");
			move(6, 4);
			prints
			    ("所谓消财免灾，交了保护费，就能保一段时间平安，不在黑帮行动对象之内。");
			showAt(12, 4, "\033[1;32m正在策划中。\033[m", YEA);
			break;
#endif
		case '2':
			if (!clubtest(ROBUNION)) {
				move(12, 4);
				prints("你不是黑帮的！别想混进来！！！");
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
			if (!(myInfo->BeIn == 1)) {
				myInfo->weapon = 0;
				myInfo->armor = 0;
				myInfo->BeIn = 1;
			}
			while (!quit) {
				ActiveTime = myInfo->lastActiveTime;
				currTime = time(NULL);
				money_show_stat("黑帮活动");
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
					    ("    翻开日记本，你上次干坏事已经是%d天%d小时%d分钟以前的事情了。\n\n"
					     "    作为黑帮成员，伤天害理，无恶不作是你的本职工作，你怎么如此放纵自己呢？",
					     day, hour, minute);
				} else {
					move(9, 1);
					prints
					    ("    你在上次黑帮活动中，被人打至重伤，还不知道什么时候能起床呢。");
				}
				move(13, 1);
				prints("    警方监控度\033[1;31m  %d\033[m %%",
				       (30000 - myInfo->WorkPoint) / 300);
				move(15, 1);
				prints
				    ("    你配备了火力强大的\033[1;33m%s\033[m, 驾驶着新买的\033[1;33m%s\033[m, 摩拳擦掌，准备大干一场。",
				     RobWeapon[myInfo->weapon],
				     RobArmor[myInfo->armor]);
				move(t_lines - 1, 0);
				prints
				    ("\033[1;44m 选单 \033[1;46m [1]勒索 [2]抢劫 [3]劫赌场 [4]绑票 [5]火并 [6]暴动 [9]军火库 [Q]离开\033[m");
				ch = igetkey();
				switch (ch) {
				case '1':
					money_show_stat("背阴巷");
					forceGetMoney(0);
					break;
				case '2':
					money_show_stat("光天化日");
					forceGetMoney(2);
					break;
				case '3':
					money_show_stat("赌场");
					RobShop();
					break;
				case '4':
					money_show_stat("黑窝");
					RobPeople(0);
					//showAt(12, 4,"就你现在这熊样，谁绑谁啊？",YEA);
					break;
				case '5':
					Rob_Fight();
					break;
				case '6':
					Rob_Rebellion();
					break;
				case '9':
					EquipShop(1);
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
			whoTakeCharge(4, uident);
			if (strcmp(currentuser->userid, uident))
				break;
			while (!quit) {
				nomoney_show_stat("黑帮帮主");
				move(t_lines - 1, 0);
				prints
				    ("\033[1;44m 选单 \033[1;46m [1]下令分赃 [2]黑吃黑 [3]查看本帮资产 [Q]离开\033[m");
				ch = igetkey();
				switch (ch) {
				case '1':
					nomoney_show_stat("销金窟");
					showAt(12, 4,
					       "\033[1;32m正在建设中。\033[m",
					       YEA);
					break;
				case '2':
					money_show_stat("黑吃黑");
					showAt(12, 4,
					       "\033[1;32m苦练绝招中。\033[m",
					       YEA);
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
			money_show_stat("保护费交费点");
			move(12, 4);
			if (askyn
			    ("\033[1;32m最近黑帮到处打劫，你要不要找人罩着你？\033[m",
			     YEA, NA) == YEA) {
				tempMoney =
				    userInputValue(13, 4, "要出资", "万", 5,
						   100) * 10000;
				if (tempMoney < 0)
					break;
				if (myInfo->cash < tempMoney) {
					showAt(15, 4,
					       "\033[1;37m你胆敢戏耍黑帮，是不是活的不耐烦了！\033[m",
					       YEA);
					break;
				}
				update_health();
				if (check_health
				    (1, 15, 4, "你的体力不够了！", YEA))
					break;
				move(15, 4);
				prints("\033[1;37m你交了%d%s保护费\n"
				       "    你的胆识似乎有所增加！\033[m",
				       tempMoney, MONEY_NAME);
				myInfo->health--;
				myInfo->Actived += tempMoney / 25000;
				myInfo->cash -= tempMoney;
				myInfo->robExp +=
				    (tempMoney - 30000) / (30000 +
							   (random() % 30000)) +
				    1;
				mcEnv->Treasury += tempMoney;
				pressanykey();
			} else {
				showAt(14, 4,
				       "\033[1;33m遇到人打劫可别说我没警告过你！\033[m",
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
