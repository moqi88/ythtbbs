#include "mc.h"
//-------------------------冒险者工会------------------------------//

int
kill_dragon()
{
	int currTime, num;
	char buf[256], title[STRLEN];

	nomoney_show_stat("山谷入口");
	currTime = time(NULL);
	if (myInfo->WorkPoint < 9000) {
		showAt(12, 4, "你以为遍地都是魔兽啊？还不早把人都吃光了!", YEA);
		return 0;
	}
	if (myInfo->health < 50) {
		showAt(12, 4,
		       "追捕魔兽是一件艰苦的事情，看你觉都没睡好，怎么可能有足够的体力？",
		       YEA);
		return 0;
	}
	if ((myInfo->robExp < 1240) || (myInfo->begExp < 1250)) {
		showAt(12, 4,
		       "这个，兄弟，不是我打击你，你还是别去给魔兽当食物了",
		       YEA);
		return 0;
	}
	if (myInfo->luck < 80) {
		showAt(12, 4, "工会不放心你去，怕你和魔兽是一伙的！", YEA);
		return 0;
	}

	myInfo->lastActiveTime = currTime;
	update_health();
	myInfo->WorkPoint -= 9000;
	myInfo->health -= 40;
	myInfo->Actived += 10;
	move(6, 1);
	prints
	    ("你顺着魔兽留下的各种蛛丝马迹，一路寻找，终于找到了一个隐秘的山谷");
	move(7, 1);
	if (askyn("你决定继续搜索吗？魔兽可是很凶猛的啊！", NA, NA) == YEA) {
		move(9, 1);
		prints
		    ("	小小魔兽岂不是手到擒来？你略一思考就冲进了山谷");
		sleep(1);
		if (check_chance
		    (myInfo->robExp + myInfo->begExp, 6000,
		     myInfo->weapon, 7, 200, 0)) {
			num = MAX(MIN(800000, mcEnv->Treasury - 20000000), 0);
			move(11, 1);
			prints
			    ("	虽然魔兽很强大，但是怎么可能和我们ymsw,yjxs,yslf的主角相抗衡呢？\n"
			     "	经过一番激烈的搏斗，你轻松消灭了魔兽—— 一只施莱姆\n"
			     "	回去报告的时候就说这是龙变的施莱姆吧，哈哈\n"
			     "	你获得赏金%d元!你的胆识和身法上升了!你的人品上升了!",
			     num);
			myInfo->cash += num;
			mcEnv->Treasury -= num;
			myInfo->robExp += 30;
			myInfo->begExp += 30;
			myInfo->luck += 20;
			sprintf(title, "【工会】英雄屠龙归来");
			sprintf(buf,
				"冒险者%s经过浴血奋战，成功消灭了在当地为害多年的恶龙!\n\n"
				"让我们一起向英雄的行为表示致敬!",
				currentuser->userid);
			deliverreport(title, buf);
			pressanykey();
			return 1;
		} else {
			move(11, 1);
			prints
			    ("	冲进山谷之后，你荣幸的发现了一双磨盘大的眼睛正盯着你看呢! 魔龙!\n"
			     "	谁tmd 的告诉我这附近只有一些小魔兽的？快逃吧!!\n"
			     "	但是魔龙会放过到嘴的午餐吗？\n"
			     "	你遍体鳞伤的逃出了山谷，发誓再也不来这个鬼地方了!\n"
			     "	你的胆识和身法下降了!");
			myInfo->robExp -= 20;
			myInfo->begExp -= 20;
			myInfo->WorkPoint -= 3600;
			sprintf(title, "【工会】魔龙作恶，期待英雄现世");
			sprintf(buf,
				"%s率领的冒险团队遭到恶龙的袭击，损失惨重\n"
				"权威人士对此事件发表评论：希望能有更多的英雄出现，消灭恶龙!",
				currentuser->userid);
			deliverreport(title, buf);
			pressanykey();
			return 0;
		}
	} else {
		num = MAX(MIN(500000, mcEnv->Treasury - 20000000), 0);
		move(9, 1);
		prints
		    ("	经过反复思量，你还是放弃了继续寻找的念头，决定把已经发现的消息带回工会\n"
		     "	工会根据你的消息消灭了魔兽\n"
		     "	你获得赏金%d元，你的人品上升了!", num);
		myInfo->cash += num;
		mcEnv->Treasury -= num;
		myInfo->luck += 10;
		sprintf(title, "【工会】赏金猎人再立奇功");
		sprintf(buf,
			"根据%s的情报，工会消灭了为恶已久的恶龙。特此嘉奖",
			currentuser->userid);
		if (!(random() % 10))
			deliverreport(title, buf);
		pressanykey();
		return 1;
	}
	return 0;
}

int
ghoul()
{
	int currTime, num;
	char buf[256], title[STRLEN];

	nomoney_show_stat("乱葬岗");
	currTime = time(NULL);
	if (myInfo->WorkPoint < 12000) {
		showAt(12, 4, "古墓都被盗墓贼挖的差不多了，不好找啊", YEA);
		return 0;
	}
	if (myInfo->health < 80) {
		showAt(12, 4, "是探险，不是旅游！休息好了再去", YEA);
		return 0;
	}
	if ((myInfo->robExp < 5000) || (myInfo->begExp < 5000)) {
		showAt(12, 4,
		       "现在还能幸存下来的古墓必然有特异之处，你还是省省吧",
		       YEA);
		return 0;
	}
	if (myInfo->luck < 80) {
		showAt(12, 4, "工会给你的情报是假的，他们不信任你！", YEA);
		return 0;
	}
	myInfo->lastActiveTime = currTime;
	update_health();
	myInfo->WorkPoint -= 12000;
	myInfo->health -= 60;
	myInfo->Actived += 20;
	move(6, 1);
	prints("根据工会提供的情报，你顺利的找到了古墓");
	sleep(1);
	move(7, 1);
	if (askyn
	    ("在古墓的一个不起眼的角落里，你发现了一条隐蔽的通道，继续探索吗？",
	     NA, NA) == NA) {
		num = MAX(MIN(1000000, mcEnv->Treasury - 20000000), 0);
		move(9, 1);
		prints
		    ("	经过反复思量，你还是放弃了继续寻找的念头，决定把已经发现的消息带回工会\n"
		     "	工会根据你的消息发掘了古墓\n"
		     "	你获得赏金%d元，你的人品上升了!", num);
		myInfo->cash += num;
		mcEnv->Treasury -= num;
		myInfo->luck += 15;
		pressanykey();
		return 1;
	}
	move(9, 1);
	prints("	不入虎穴，焉得虎子？你决定继续向前！");
	sleep(1);
	if (check_chance
	    (myInfo->robExp + myInfo->begExp, 15000, myInfo->weapon, 7, 300,
	     0)) {
		move(11, 1);
		prints
		    ("	果然有埋伏！古墓中防守巨石像被你惊醒了！\n"
		     "	你左冲右突，将他们重新变成一堆堆的石头\n"
		     "	你的胆识和身法上升！\n");
		myInfo->robExp += 50;
		myInfo->begExp += 50;
		pressanykey();
		move(14, 1);
		if (askyn
		    ("越往深处走，防守的力量越强大，你渐渐有些力不从心，还继续往前吗?",
		     NA, NA) == YEA) {
			if (check_chance
			    (myInfo->robExp +
			     myInfo->begExp, 25000,
			     myInfo->armor, 10, 300, -100)) {
				num =
				    MAX(MIN
					(3000000,
					 mcEnv->Treasury - 20000000), 0);
				move(15, 1);
				prints
				    ("    坚持就是胜利！终于，最后一个巨石像倒在了你的剑下！\n"
				     "    古墓的宝藏已经全部展现在你的面前，无数的黄金珠宝啊！\n"
				     "    你获得了价值数百万的珠宝！");
				myInfo->cash += num;
				mcEnv->Treasury -= num;
				sprintf(title, "【工会】冒险者满载而归");
				sprintf(buf,
					"冒险者%s在古墓中发掘出了价值数百万的珠宝！",
					currentuser->userid);
				deliverreport(title, buf);
				pressanykey();
				return 1;
			}
		} else {
			move(17, 1);
			prints
			    ("    经过再三思考，你还是决定退了出来！\n"
			     "    留得青山在，不怕没柴烧，我还会回来的！");
			sprintf(title, "【工会】古墓发掘计划");
			sprintf(buf,
				"冒险者%s发现了一座保存完好的古墓但无力发掘，现诚寻合作者。！",
				currentuser->userid);
			deliverreport(title, buf);
			pressanykey();
			return 0;
		}
	}
	move(15, 1);
	prints
	    ("	人力总有极限，源源不断的巨石像却是杀而不绝。\n"
	     "	你已经错过了最佳的撤退时间，现在只能且战且退了。\n"
	     "	你终于撤了出来，带着遍身的伤痕!\n" "	你的胆识和身法下降了!");
	myInfo->robExp -= 20;
	myInfo->begExp -= 20;
	myInfo->WorkPoint -= 3600;
	sprintf(title, "【工会】古墓幽深，团队历险");
	sprintf(buf,
		"%s率领的冒险团队在古墓探索中冒险前进，损失惨重。\n"
		"\n工会提醒：请量力而行。", currentuser->userid);
	deliverreport(title, buf);
	pressanykey();
	return 0;
}

static int
normal_work()
{
	int ch, quit = 0, day, hour, minute, num;
	time_t ActiveTime, currTime;
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

	if (clubtest(ROBUNION) || clubtest(BEGGAR)) {
		move(12, 4);
		prints("你不是工会的人喔, 先加入组织吧");
		pressanykey();
		return 0;
	}
	if (!(myInfo->BeIn == 0)) {
		myInfo->weapon = 0;
		myInfo->armor = 0;
		myInfo->BeIn = 0;
	}
	while (!quit) {
		if (!(random() % 10))
			if (show_cake()) {
				prints
				    ("\n    警官怒吼：“身份验证失败！跟我走一趟！”"
				     "\n    555，你被没收所有现金，并且被监禁30分钟。");
				myInfo->freeTime = time(NULL) + 1800;
				mcEnv->Treasury += myInfo->cash;
				myInfo->cash = 0;
				myInfo->mutex = 0;
				unloadData(myInfo, sizeof (mcUserInfo));
				unloadData(mcEnv, sizeof (MC_Env));
				pressreturn();
				Q_Goodbye();
			}
		money_show_stat("工会前台");
/*		if (mcEnv->Treasury<20000000) {
			showAt(5, 4, "国库刮空了，发不起工资了，cmft", YEA);
			return 0;
	  }
*/
		move(5, 4);
		prints("这里是安分守己的良民合法营生的场所，勤劳致富光荣");
		move(7, 4);
		prints
		    ("高难度的工作当然有高收入，但危险性也相应增加，你可要根据自己情况酌情选择哦");
		ActiveTime = myInfo->lastActiveTime;
		currTime = time(NULL);
		if (currTime > ActiveTime) {
			day = (currTime - ActiveTime) / 86400;
			hour = (currTime - ActiveTime) % 86400 / 3600;
			minute = (currTime - ActiveTime) % 3600 / 60 + 1;
			prints
			    ("\n\n    工作人员翻了翻笔记本，语重心长的对你说："
			     "\n\n	  你上次工作已经是%d天%d小时%d分钟以前了"
			     "\n\n    现在找个好的工作不容易，你一定要注意珍惜啊!",
			     day, hour, minute);
		} else {
			prints
			    ("\n\n    你在上次工作中因公受伤，还需要疗养一段时间");
		}
		move(15, 1);
		prints("    疲劳度\033[1;31m  %d\033[m %%",
		       (30000 - myInfo->WorkPoint) / 300);
		move(17, 1);
		prints
		    ("    你挥舞着\033[1;33m%s\033[m, 披挂着\033[1;33m%s\033[m, 一副江湖老手的行头。",
		     FreeWeapon[myInfo->weapon], FreeArmor[myInfo->armor]);
		move(t_lines - 2, 0);
		prints
		    ("\033[1;44m 选单 \033[1;46m[1]锻炼身体 [2]义务交警 [3]教书育人 [4]添置装备\n"
		     "\033[1;44m      \033[1;46m[5]屠龙     [6]挖坟     [Q]离开\033[0m");
		ch = igetkey();
		switch (ch) {
		case '1':
			nomoney_show_stat("大操场前");
			currTime = time(NULL);
			if (myInfo->WorkPoint < 1800) {
				showAt(12, 4, "连续运动身体会吃不消的", YEA);
				return 0;
			}
			if (myInfo->health < 30) {
				showAt(12, 4,
				       "你太疲惫了，没有精力来进行这些运动",
				       YEA);
				return 0;
			}
			myInfo->lastActiveTime = currTime;
			update_health();
			myInfo->WorkPoint -= 1800;
			myInfo->health -= 30;
			myInfo->Actived += 10;
			if (random() % 4) {
				move(6, 1);
				prints
				    ("	锻炼身体，为祖国健康工作50年！！\n"
				     "	一二三四，嘿呀呀\n");
				if (random() % 2)
					myInfo->robExp++;
				if (random() % 2)
					myInfo->begExp++;
				pressanykey();
				return 1;
			} else {
				move(6, 1);
				prints
				    ("	哎哟哎哟，锻炼把腰闪了，把脚拐了，真是得不偿失啊。");
				pressanykey();
				return 0;
			}
			break;
		case '2':
			nomoney_show_stat("十字路口");
			currTime = time(NULL);
			if (myInfo->WorkPoint < 3300) {
				showAt(12, 4,
				       "最近交通秩序良好，不需要义务交警", YEA);
				return 0;
			}
			if (myInfo->health < 50) {
				showAt(12, 4,
				       "看你这疲惫的样子，想去制造交通事故啊？",
				       YEA);
				return 0;
			}
			if ((myInfo->robExp < 50) || (myInfo->begExp < 50)) {
				showAt(12, 4,
				       "看你这风都吹得倒的样子，指挥交通？人家还不想给你付医药费呢!",
				       YEA);
				return 0;
			}
			myInfo->lastActiveTime = currTime;
			update_health();
			myInfo->WorkPoint -= 3300;
			myInfo->health -= 50;
			myInfo->Actived += 10;
			if (random() % 2) {
				num =
				    MAX(MIN(80000, mcEnv->Treasury - 20000000),
					0);
				move(6, 1);
				prints
				    ("	义务交警很简单嘛，就是挥挥手而已\n"
				     "	咦，前面走过来的哪个是谁？难道是传说中的财神\n"
				     "	赶快走上去献殷勤，嘿嘿\n"
				     "	哇！发了，包里凭空多出%d元来\n"
				     "	果然好人有好报啊!", num);
				myInfo->cash += num;
				mcEnv->Treasury -= num;
				myInfo->luck += 2;
				pressanykey();
				return 1;
			} else {
				move(6, 1);
				prints("	你认真的指挥着交通\n"
				       "	比如说：“左转的靠右走，右转的靠左走，直行的向后走，没死的继续走”\n"
				       "	恭喜你，在如此混乱的情况下，你都没有挂掉\n"
				       "	你的胆识和身法上升了");
				myInfo->robExp += 2;
				myInfo->begExp += 2;
				pressanykey();
				return 1;
			}
			break;
		case '3':
			nomoney_show_stat("学校");
			currTime = time(NULL);
			if (myInfo->WorkPoint < 4500) {
				showAt(12, 4,
				       "现在提倡减轻学生负担，课程不要安排太多了",
				       YEA);
				return 0;
			}
			if (myInfo->health < 90) {
				showAt(12, 4,
				       "你自己都困得快睡着了，还能去上课？",
				       YEA);
				return 0;
			}
			if ((myInfo->robExp < 250) || (myInfo->begExp < 250)) {
				showAt(12, 4,
				       "就你现在这个样子？别去误人子弟了", YEA);
				return 0;
			}
			if (myInfo->luck < 60) {
				showAt(12, 4,
				       "就你这德行？让你教书怕带坏了学生!",
				       YEA);
				return 0;
			}
			myInfo->lastActiveTime = currTime;
			update_health();
			myInfo->WorkPoint -= 4500;
			myInfo->health -= 80;
			myInfo->Actived += 10;
			if (random() % 2) {
				num =
				    MAX(MIN(150000, mcEnv->Treasury - 20000000),
					0);
				move(6, 1);
				prints("	你从山顶洞人扯到臭氧层空洞\n"
				       "	你从飞流直下三千尺联想到椭圆曲线方程\n"
				       "	仿佛天上地下无所不知\n"
				       "	学生们听得津津有味，看来效果还不错\n"
				       "	你获得工资%d元!你的人品上升了!",
				       num);
				myInfo->cash += num;
				mcEnv->Treasury -= num;
				myInfo->luck += 5;
				pressanykey();
				return 1;
			} else {
				move(6, 1);
				prints
				    ("	你刚一走上讲台，学生们就不断起哄\n"
				     "	一时间粉笔和书本齐飞，矿泉水瓶漫天飞舞\n"
				     "	你不得不左躲右闪，充分锻炼了胆识和身法");
				myInfo->robExp += 5;
				myInfo->begExp += 5;
				pressanykey();
				return 1;
			}
			break;
		case '4':
			EquipShop(0);
			break;
		case '5':
			nomoney_show_stat("山谷");
			kill_dragon();
			break;
		case '6':
			nomoney_show_stat("乱葬岗");
			ghoul();
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
free_work()
{
	char uident[IDLEN + 1], buf[256], buff[256];
	int ch, quit = 0;
	mcUserInfo *mcuInfo;

	while (!quit) {
		nomoney_show_stat("工会办事处");
		move(4, 4);
		prints("如果您有什么需要完成的任务，可以在这里发布。");
		move(6, 4);
		prints("每个任务悬赏100万，工会提成20万。");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m 选单 \033[1;46m [1]发布任务 [2]任务完成 [3]当前任务 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			if (myInfo->cash < 1200000) {
				showAt(8, 4, "不带钱发表什么任务啊", YEA);
				break;
			}
			if (seek_in_file
			    (DIR_MC "freework_list", currentuser->userid)) {
				showAt(8, 4, "你已经发布了一个任务了。", YEA);
				break;
			}
			getdata(8, 4,
				"简述任务内容[\033[1;33mENTER\033[0m放弃]：",
				genbuf, 40, DOECHO, YEA);
			if (genbuf[0] == '\0')
				break;
			move(9, 4);
			if (askyn("\033[1;33m你确认任务内容吗\033[0m", NA, NA)
			    == NA)
				break;
			myInfo->cash -= 1200000;
			mcEnv->Treasury += 1200000;
			snprintf(buf, STRLEN - 1, "%s %s", currentuser->userid,
				 genbuf);
			addtofile(DIR_MC "freework_list", buf);
			break;
		case '2':
			if (!
			    (seek_in_file
			     (DIR_MC "freework_list", currentuser->userid))) {
				showAt(8, 4, "你没有发布过任务啊。", YEA);
				break;
			}
			if (!getOkUser("\n谁完成了任务：", uident, 6, 4)) {
				move(8, 4);
				prints("查无此人");
				pressanykey();
				break;
			}
			move(9, 4);
			if (askyn("\033[1;33m你确认吗?\033[0m", NA, NA) == NA)
				break;
			if (!strcmp(uident, currentuser->userid)) {
				showAt(8, 4, "自己的任务自己做，吃饱了撑啊",
				       YEA);
				break;
			}
			move(8, 4);
			sethomefile(buf, uident, "mc.save");
			if (!file_exist(buf))
				initData(1, buf);
			if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
				return;
			if (mcuInfo->GetLetter == 0) {
				mcuInfo->cash += 50000;
				mcEnv->Treasury -= 50000;
			} else {
				mcuInfo->cash += 1000000;
				mcEnv->Treasury -= 1000000;
			}
			del_from_file(DIR_MC "freework_list",
				      currentuser->userid);
			sprintf(buff,
				"你完成了%s 提供的自由任务，获得奖金 %d%s。",
				currentuser->userid, 1000000, MONEY_NAME);
			if (mcuInfo->GetLetter == 1)
				system_mail_buf(buff, strlen(buff), uident,
						"工会通知", "deliver");
			sprintf(buff,
				"%s完成了%s 提供得自由任务，工会予以嘉奖。",
				uident, currentuser->userid);
			deliverreport("[工会]自由任务", buff);
			unloadData(mcuInfo, sizeof (mcUserInfo));
			break;
		case '3':
		 	clear();
			move(1, 0);
			prints("%s工会当前任务:\n", CENTER_NAME);
			listfilecontent(DIR_MC "freework_list");
			FreeNameList();
			pressanykey();
			break; 
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	return;
}

static int
get_task()
{
	int i, j;

	nomoney_show_stat("丽香楼");
	move(4, 4);
	prints("这是大富翁之城里最好的声色之所, 进出的客人络绎不绝, 觥筹交错.\n"
	       "    凭着敏锐的直觉, 你一进去就在角落中找到了神秘人.\n");
	move(7, 4);
	if (askyn("您要领取任务吗? 有酬劳喔~", NA, NA) == NA)
		return 0;
	move(8, 4);
	if (myInfo->task.time > 0) {
		prints("您的任务还没有完成!");
		pressanykey();
		return 0;
	}
	if ((myInfo->task.task_flag & TASK_DONE) != 0) {
		prints("神秘人嘴角露出了笑意, 递给你 100000 %s作为奖励.\n",
		       MONEY_NAME);
		myInfo->cash += 100000;
	}
	while (1) {
		i = random() % maps[0].row;
		j = random() % maps[0].col;
		if (maps[0].map[i][j].exit & (~NODE_QUEUED))
			break;
	}
	maps[0].map[i][j].npc_num += 1;
	myInfo->task.time = time(NULL);
	myInfo->task.x = i;
	myInfo->task.y = j;
	myInfo->task.task_flag &= ~TASK_DONE;
	move(9, 4);
	prints("神秘人小声的说: \"去邪恶荒原一趟, 到了那里会给你提示的.\"");
	pressanykey();
	return 0;
}

static int
cancel_task()
{
	nomoney_show_stat("丽香楼");
	move(4, 4);
	prints("这是大富翁之城里最好的声色之所, 进出的客人络绎不绝, 觥筹交错.\n"
	       "    凭着敏锐的直觉, 你一进去就在角落中找到了神秘人.\n");
	move(7, 4);
	if (askyn("您要取消任务吗? ", NA, NA) == NA)
		return 0;
	move(8, 4);
	if (myInfo->task.time == 0) {
		prints("您没有领取任务啊!");
		pressanykey();
		return 0;
	}
	myInfo->task.task_flag &= ~TASK_DONE;
	if (myInfo->task.time + 3600 < time(NULL)) {
		myInfo->task.time = 0;
		prints("神秘人狠狠的说: \"这个任务已经过时了! 没本事就别接!\"");
		pressanykey();
		return 0;
	}
	if (myInfo->cash > 100000) {
		myInfo->cash -= 100000;
		mcEnv->Treasury += 100000;
		myInfo->task.time = 0;
		myInfo->task.x = -1;
		prints("神秘人淡淡的说: \"看在钱的份上, 这次就算了.\"");
		pressanykey();
		return 1;
	}
	myInfo->freeTime = time(NULL) + 1800;
	prints("神秘人冷笑道: \"敢耍我?!  来人呀!\"");
	refresh();
	sleep(3);
	Q_Goodbye();
	return 0;
}

int
check_task(int flag)
{
	int r = 0;

	if (myInfo->task.time == 0 || myInfo->task.x < 0 || myInfo->task.y < 0)	//no task
		return 0;
	if (maps[0].map[myInfo->task.x][myInfo->task.y].npc_num > 0)	//still task
		return 0;
	if (flag == 0 && myInfo->pos_x == myInfo->task.x && myInfo->pos_y == myInfo->task.y) {	//failed task
		myInfo->task.time = 0;
		myInfo->task.x = -1;
		return 0;
	}
	if (myInfo->task.time + 3600 >= time(NULL)) {	//intime
		myInfo->task.task_flag |= TASK_DONE;
		r = 1;
	}
	myInfo->task.time = 0;
	myInfo->task.x = -1;
	return r;
}

int
money_task()
{
	int ch, quit = 0;
	char uident[IDLEN + 1];
	time_t currTime;

	if (!(myInfo->GetLetter == 1)) {
		clear();
		showAt(5, 4, "你已经关闭了金融中心游戏功能，请开启后再来。",
		       YEA);
		return 0;
	}
	while (!quit) {
		nomoney_show_stat("大富翁之城");
		move(8, 4);
		prints("这里是冒险者的天堂！英雄将无畏艰险, 在征程上勇敢前进!");
		move(t_lines - 2, 0);
		prints
		    ("\033[1;44m 选单 \033[1;46m [1]领取任务 [2]出城     [3]取消任务 [4]闭关\033[0m\n"
		     "\033[1;44m      \033[1;46m [5]赏金任务 [6]工会活动 [7]黑鹰堂   [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			//get_task();
			break;
		case '2':
			clear();
			//if (walk_map(&maps[0]) == 1)
			//	return 0;
			break;
			/*prints("%s工会当前任务:\n", CENTER_NAME);
			   listfilecontent(DIR_MC "freework_list");
			   FreeNameList();
			   pressanykey();
			   break; */
		case '3':
			//cancel_task();
			break;
		case '4':
			money_show_stat("闭关室");
			currTime = time(NULL);
			if (currTime - myInfo->lastActiveTime < 21600) {
				if (myInfo->BeIn) {
					move(5, 4);
					prints
					    ("你最近太活跃了，警察随时盯着你呢，东躲西藏，一刻都安宁不下来");
				} else {
					move(5, 4);
					prints
					    ("刚干了这么多活，太兴奋了，静不下心来");
				}
				pressanykey();
				break;
			}
			myInfo->WorkPoint +=
			    (currTime - myInfo->lastActiveTime - 3600);
			update_health();
			myInfo->lastActiveTime = time(NULL);
			if (myInfo->BeIn) {
				move(5, 4);
				prints
				    ("呵呵，闭关这么久，警察该放松警惕了吧，可以大干一场了");
			} else {
				move(5, 4);
				prints
				    ("从闭关室出来，你感到心旷神怡，浑身的疲劳已经无影无踪");
			}
			pressanykey();
			break;
		case '5':
			free_work();
			break;
		case '6':
			normal_work();
			break;
		case '7':
			nomoney_show_stat("黑鹰堂");
			whoTakeCharge(0, uident);
			if (strcmp(currentuser->userid, uident))
				break;

			if (!getOkUser("\n你要删除谁的任务：", uident, 6, 4)) {
				move(8, 4);
				prints("查无此人");
				pressanykey();
				break;
			}
			move(9, 4);
			if (askyn("\033[1;33m你确认吗?\033[0m", NA, NA) == NA)
				break;
			del_from_file(DIR_MC "freework_list", uident);
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
