#include "mc.h"
 
// -------------------------   商店   --------------------------  // 
static int 
buy_card(char *cardname, char *filepath) 
{
	int i;
	char uident[IDLEN + 1], note[3][STRLEN], tmpname[STRLEN];
	clear();
	ansimore2(filepath, 0, 0, 25);
	for (i = 0; i < 4; i++) {
		move(2 + i * 5, 2 + i * 20);
		prints("\033[1;5;32m%s\033[m", MY_BBS_DOMAIN);
	}
	move(t_lines - 2, 4);
	sprintf(genbuf, "你确定要买贺卡%s么", cardname);
	if (askyn(genbuf, YEA, NA) == NA)
		return 0;
	if (myInfo->cash < 1000) {
		showAt(t_lines - 2, 60, "你的钱不够啊！", YEA);
		return 0;
	}
	myInfo->cash -= 1000;
	mcEnv->Treasury += 1000;
	clear();
	if (!getOkUser("要把这卡片送给谁? ", uident, 0, 0))
		return 0;
	move(2, 0);
	prints("有话要在卡片里说吗？[可以写3行喔]");
	bzero(note, sizeof (note));
	for (i = 0; i < 3; i++) {
		getdata(3 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
		if (note[i][0] == '\0')
			break;
	}
	sprintf(tmpname, "bbstmpfs/tmp/card.%s.%d", currentuser->userid,
		 uinfo.pid);
	copyfile(filepath, tmpname);
	if (i > 0) {
		int j;
		FILE * fp = fopen(tmpname, "a");
		fprintf(fp, "\n以下是 %s 的附言:\n", currentuser->userid);
		for (j = 0; j < i; j++)
			fprintf(fp, "%s", note[j]);
		fclose(fp);
	}
	if (mail_file 
	     (tmpname, uident, "看看我从商店里挑给你的贺卡，喜欢吗？",
	      currentuser->userid) >= 0) {
		showAt(8, 0, "你的贺卡已经发出去了", YEA);
	} else {
		showAt(8, 0, "贺卡发送失败，对方信件超容", YEA);
	}
	unlink(tmpname);
	return 0;
}
static int 
shop_card_show() 
{
	DIR * dp;
	struct dirent *dirp;
	char dirNameBuffer[10][32], buf[STRLEN];
	char dirpath[256], filepath[256];
	int numDir, numFile, dirIndex, cardIndex;
	clear();
	sprintf(buf,
		 "    本店贺卡均非本店制作，部分贺卡由于种种原因，未能标明作者。\n"
		 
		 "    如贺卡创作者对其作品用于本店持有异议，请与本站系统维护联系\n"
		  "    本站将及时根据作者意愿作出调整。\n\n" 
		 "                    \033[1;32m%s贺卡店\033[0m\n",
		 CENTER_NAME);
	showAt(6, 0, buf, YEA);
	sprintf(buf, "%s贺卡店", CENTER_NAME);
	nomoney_show_stat(buf);
	sprintf(buf, "贺卡目前每张定价 1000 %s。本店出售如下类型的贺卡:",
		 MONEY_NAME);
	showAt(4, 4, buf, NA);
	if ((dp = opendir(DIR_SHOP)) == NULL)
		return -1;
	for (numDir = 0; (dirp = readdir(dp)) != NULL && numDir < 10;) {
		snprintf(dirpath, 255, "%s%s", DIR_SHOP, dirp->d_name);
		if (!file_isdir(dirpath) || dirp->d_name[0] == '.')
			continue;
		move(6 + numDir, 8);
		sprintf(buf, "\033[1;%dm%d. %s\033[m", numDir + 31, numDir,
			 dirp->d_name);
		prints(buf);
		strncpy(dirNameBuffer[numDir], dirp->d_name, 31);
		dirNameBuffer[numDir][31] = '\0';
		numDir++;
	}
	while (1) {
		getdata(16, 4, "请选择类型:", buf, 3, DOECHO, YEA);
		if (buf[0] == '\0')
			return 0;
		dirIndex = atoi(buf);
		if (dirIndex >= 0 && dirIndex < numDir)
			break;
	}
	snprintf(dirpath, 255, "%s%s", DIR_SHOP, dirNameBuffer[dirIndex]);
	if ((dp = opendir(dirpath)) == NULL)
		return -1;
	for (numFile = 0; (dirp = readdir(dp)) != NULL;) {
		snprintf(filepath, 255, "%s/%s", dirpath, dirp->d_name);
		if (file_isfile(filepath) && dirp->d_name[0] != '.')
			numFile++;
	}
	move(17, 4);
	sprintf(genbuf,
		 "%s 类型的卡片共有 %d 张. 请选择要预览的卡号[ENTER放弃]: ",
		 dirNameBuffer[dirIndex], numFile);
	while (1) {
		getdata(18, 4, genbuf, buf, 3, DOECHO, YEA);
		if (buf[0] == '\0')
			return 0;
		cardIndex = atoi(buf);
		if (cardIndex >= 1 && cardIndex <= numFile)
			break;
	}
	snprintf(buf, STRLEN - 1, "%s%d", dirNameBuffer[dirIndex], cardIndex);
	snprintf(filepath, 255, "%s/%d", dirpath, cardIndex);
	buy_card(buf, filepath);
	limit_cpu();
	return 0;
}
static int 
shop_sellExp() 
{
	int convertMoney, exp, ch, quit = 0;
	float convertRate;
	char buf[256];
	clear();
	sprintf(buf, "%s当铺", CENTER_NAME);
	money_show_stat(buf);
	convertRate =
	    MIN(bbsinfo.utmpshm->ave_score / 2000.0 + 0.1, 10) * 1000;
	sprintf(genbuf,
		 "    您可以通过变卖经验值获得%s。今天的报价是\n" 
		 "    [1]经验值：%.1f每点\t"  "[2]属性：40000每点\t" 
		 "[3]人品： 6000每点\t"  "[Q]退出\t\t请选择：", MONEY_NAME,
		 convertRate);
	move(4, 0);
	prints("%s", genbuf);
	while (!quit) {
		ch = igetkey();
		switch (ch) {
		case '1':
			exp = countexp(currentuser, 2);
			convertMoney = (exp - myInfo->selledExp) * convertRate;
			if (convertMoney <= 0)
				break;
			sprintf(genbuf,
				 "按现在的经验值,当铺最多给您 %d %s, 还要变卖吗",
				 convertMoney, MONEY_NAME);
			move(7, 4);
			if (askyn(genbuf, NA, NA) == NA)
				break;
			if (mcEnv->Treasury - 10000000 > convertMoney) {	//当铺里国库至少保持1000w 资金下限
				myInfo->Actived++;
				mcEnv->Treasury -= convertMoney;
				myInfo->selledExp = exp;
				myInfo->cash += convertMoney;
				move(8, 4);
				prints("交易成功，这里是您的 %d %s。",
					convertMoney, MONEY_NAME);
			} else {
				prints("国库没有那么多钱……");
			}
			break;
			
/*		case '2':
			convertMoney = userInputValue(7, 4, "卖", "点胆识", 1, 100);
			if (convertMoney < 0)	break;
			if (convertMoney > myInfo->robExp) {
				showAt(8, 4, "你没有那么高的胆识！", YEA);
				break;
			}
			if (mcEnv->Treasury < convertMoney * 30000) {
				showAt(8, 4, "国库没有那么多钱……", YEA);
				break;
			}
			myInfo->Actived++;
			myInfo->robExp -= convertMoney;
			myInfo->cash += convertMoney * 30000;
			mcEnv->Treasury -= convertMoney * 30000;
			showAt(9, 4, "交易成功！", YEA);
			break;
		case '3':
                        convertMoney = userInputValue(7, 4, "卖", "点身法", 1, 100);
			if (convertMoney < 0)   break;
			if (convertMoney > myInfo->begExp) {
				showAt(8, 4, "你没有那么高的身法！", YEA);
				break;
			}
			if (mcEnv->Treasury < convertMoney * 30000) {
				showAt(8, 4, "国库没有那么多钱……", YEA);
				break;
			}
			myInfo->Actived++;
			myInfo->begExp -= convertMoney;
			myInfo->cash += convertMoney * 30000;
			mcEnv->Treasury -= convertMoney * 30000;
			showAt(9, 4, "交易成功！", YEA);
			break;
*/ 
		case '2':
			convertMoney =
			    userInputValue(7, 4, "卖", "点属性", 1, 100);
			if (convertMoney < 0)
				break;
			if (convertMoney > myInfo->robExp) {
				showAt(8, 4, "你没有那么高的胆识！", YEA);
				break;
			}
			if (convertMoney > myInfo->begExp) {
				showAt(8, 4, "你没有那么高的身法！", YEA);
				break;
			}
			if (mcEnv->Treasury - 25000000 < convertMoney * 40000) {	//卖属性国库保底2500w
				showAt(8, 4, "国库没有那么多钱……", YEA);
				break;
			}
			myInfo->Actived++;
			myInfo->robExp -= convertMoney;
			myInfo->begExp -= convertMoney;
			myInfo->cash += convertMoney * 40000;
			mcEnv->Treasury -= convertMoney * 40000;
			showAt(9, 4, "交易成功！", YEA);
			break;
		case '3':
			convertMoney =
			    userInputValue(7, 4, "卖", "点人品", 1, 100);
			if (convertMoney < 0)
				break;
			if (convertMoney > myInfo->luck) {
				showAt(8, 4, "你没有那么好的人品！", YEA);
				break;
			}
			if (mcEnv->Treasury - 25000000 < convertMoney * 6000) {
				showAt(8, 4, "国库没有那么多钱……", YEA);
				break;
			}
			myInfo->Actived++;
			myInfo->luck -= convertMoney;
			myInfo->cash += convertMoney * 6000;
			mcEnv->Treasury -= convertMoney * 6000;
			showAt(9, 4, "交易成功！", YEA);
			break;
		case 'q':
		case 'Q':
		default:
			quit = 1;
			break;
		}
	}
	return 1;
}
static void 
buydog() 
{
	int guard_num;
	clear();
	nomoney_show_stat("猛兽园");
	move(4, 4);
	prints 
	    ("%s猛兽园出售纯种大狼狗给需要保护的人士，每只10000%s。",
	     CENTER_NAME, MONEY_NAME);
	
#if 0
	    if (myInfo->cash < 100000) {
		showAt(8, 4, "你还是省省吧?没人会打你的主意的。", YEA);
		return;
	}
	
#endif				/*  */
	    guard_num = (myInfo->robExp / 100) + 1;
	guard_num = (guard_num > 8) ? 8 : guard_num;
	if (myInfo->guard >= guard_num) {
		showAt(8, 4, "你已经有足够的大狼狗了啊。不用这么胆小吧？",
			YEA);
		return;
	}
	move(6, 4);
	prints("按照您目前的身份地位，再买%d只大狼狗就够了。",
		guard_num - myInfo->guard);
	guard_num =
	    userInputValue(7, 4, "买", "只大狼狗", 1,
			    guard_num - myInfo->guard);
	if (guard_num == -1) {
		showAt(8, 4, "哦，您不想买了啊。。。那欢迎下次再来！", YEA);
		return;
	}
	if (myInfo->cash < guard_num * 10000) {
		showAt(8, 4, "不好意思，您的钱不够，慢走不送～", YEA);
		return;
	}
	myInfo->cash -= guard_num * 10000;
	mcEnv->prize777 += after_tax(guard_num * 10000);
	myInfo->guard += guard_num;
	move(8, 4);
	showAt(9, 4, "买大狼狗成功,你可以有一段时间安享太平了。", YEA);
	return;
}
int 
money_shop() 
{
	int ch, quit = 0, bonus = 0;
	char buf[256], uident[IDLEN + 1];
	if (!(myInfo->GetLetter == 1)) {
		clear();
		showAt(5, 4, "你已经关闭了金融中心游戏功能，请开启后再来。",
			YEA);
		return 0;
	}
	while (!quit) {
		sprintf(buf, "%s百货公司", CENTER_NAME);
		nomoney_show_stat(buf);
		move(6, 4);
		prints("%s商场最近生意红火，大家尽兴！", CENTER_NAME);
		move(t_lines - 1, 0);
		prints 
		    ("\033[1;44m 选项 \033[1;46m [1]道具 [2]贺卡 [3]当铺 [4]商场经理办公室 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			money_show_stat("商场购物柜台");
			move(5, 0);
			prints("\033[1;42m功能简介：\033[m\n" 
				"大狼狗（使用对象：自己。作用：多重保护减少被拍砖可能）\n"
				
				"平衡器（使用对象：自己。作用：平衡胆识和身法）\n"
				
				"菩提子（使用对象：自己。作用：随机增加胆识和身法）\n"
				 "好运丸（使用对象：自己。作用：随机加人品）\n"
				
				"洗髓丹（使用对象：自己。作用：人品爆发的话，有可能长根骨）\n"
				
				"穿梭机（使用对象：自己。作用：神秘的时间机器）\n"
				 "千年参（使用对象：自己。作用：增加体力）\n" 
				"正气水（使用对象：自己。作用：减少疲劳,清醒头脑）\n"
				
#if 0			      
				"厄运丹（使用对象：可指定。作用：成功后可短期减其运气）\n"
				 "长筒袜 (使用对象：自己。作用：短期加胆识)\n"
				
				"偷拍机（使用对象：可指定。作用：成功后可减其胆识）\n"
				 "自行车（使用对象：自己。作用：短期加身法)\n"
				
				"西瓜皮（使用对象：可指定。作用：成功后可短期减其身法）"
				
#endif				/*  */
			    );
			move(t_lines - 2, 0);
			
#if 0
			    prints 
			    ("\033[1;44m 您要购买 \033[1;46m [1]大狼狗 [2]肉包子 [3]好运丸 [4]厄运丹\n"
			     
			     "\033[1;44m          \033[1;46m [5]长筒袜 [6]偷拍机 [7]自行车 [8]西瓜皮 [Q]离开\033[m");
			
#else				/*  */
			    prints
			    ("\033[1;44m 您要购买 \033[1;46m [1]大狼狗 [2]平衡器 [3]菩提子 [4]好运丸 [5]洗髓丹 [6]穿梭机\033[0m\n"
			     
			     "\033[1;44m          \033[1;46m [7]千年参 [8]正气水 [Q]离开\033[m");
			
#endif				/*  */
			    ch = igetkey();
			switch (ch) {
			case '1':
				update_health();
				if (check_health
				     (1, 19, 4, "体力不够了！", YEA))
					break;
				myInfo->health--;
				myInfo->Actived++;
				buydog();
				break;
			case '2':
				update_health();
				move(18, 4);
				if (askyn
				     ("平衡器100,000元一个，确实要购买吗？", NA,
				      NA) == NA)
					break;
				if (check_health
				     (1, 19, 4, "你的体力不够了！", YEA))
					break;
				if (myInfo->cash < 100000) {
					showAt(19, 4, "你没带那么多现金！",
						YEA);
					break;
				}
				myInfo->health--;
				myInfo->Actived++;
				myInfo->cash -= 100000;
				mcEnv->Treasury += 100000;
				myInfo->robExp =
				    (myInfo->robExp + myInfo->begExp) / 2;
				myInfo->begExp = myInfo->robExp;
				showAt(20, 4, "转换成功！", YEA);
				break;
			case '3':
				update_health();
				move(18, 4);
				if (askyn
				     ("菩提子500,000元一个，确实要购买吗？", NA,
				      NA) == NA)
					break;
				if (check_health
				     (20, 19, 4, "你的体力不够了！", YEA))
					break;
				if (myInfo->cash < 500000) {
					showAt(19, 4, "你没带够现金。", YEA);
					break;
				}
				myInfo->health -= 20;
				myInfo->Actived += 5;
				myInfo->cash -= 500000;
				mcEnv->Treasury += 500000;
				bonus = random() % 15 + 1;
				myInfo->robExp += bonus;
				myInfo->begExp += bonus;
				move(20, 4);
				prints
				    ("\033[1;31m恭喜！你的身法和胆识各增加了%d点！！",
				     bonus);
				pressanykey();
				break;
			case '4':
				update_health();
				move(18, 4);
				if (askyn
				     ("好运丸500,000元一颗，确实要购买吗？", NA,
				      NA) == NA)
					break;
				if (check_health
				     (20, 19, 4, "你的体力不够了！", YEA))
					break;
				if (myInfo->cash < 500000) {
					showAt(14, 4, "你没带够现金。", YEA);
					break;
				}
				myInfo->health -= 20;
				myInfo->Actived += 5;
				myInfo->cash -= 500000;
				mcEnv->Treasury += 500000;
				bonus = random() % (100 - myInfo->luck) + 1;
				myInfo->luck += bonus;
				move(20, 4);
				prints
				    ("\033[1;31m恭喜！你的人品增加了%d点！！",
				     bonus);
				pressanykey();
				break;
			case '5':
				update_health();
				move(18, 4);
				if (askyn
				     ("洗髓丸500,000元一颗，而且可能没效果，确实要购买吗？",
				      NA, NA) == NA)
					break;
				if (check_health
				     (20, 19, 4, "你的体力不够了！", YEA))
					break;
				if (myInfo->cash < 500000) {
					showAt(19, 4, "你没带够现金。", YEA);
					break;
				}
				myInfo->health -= 20;
				myInfo->Actived += 5;
				myInfo->cash -= 500000;
				mcEnv->Treasury += 500000;
				if ((random() % 2) && (myInfo->luck >= 90)
				     && (myInfo->con < 25)) {
					myInfo->con += random() % 6 + 1;
					move(20, 4);
					prints
					    ("\033[1;31m你吞下一颗洗髓丹！只听骨骼哗哗响，好像体内发生了点变化！！");
				}
				
				else {
					move(20, 4);
					prints
					    ("\033[1;31m你吞下一颗洗髓丹！就像嚼腊一样，什么感觉都没有。。。");
				}
				pressanykey();
				break;
			case '6':
				update_health();
				move(18, 4);
				if (askyn
				     ("穿梭机500,000元一台，据说能跨越时间，确实要购买吗？",
				      NA, NA) == NA)
					break;
				if (check_health
				     (20, 19, 4, "你的体力不够了！", YEA))
					break;
				if (myInfo->cash < 500000) {
					showAt(19, 4, "你没带够现金。", YEA);
					break;
				}
				myInfo->health -= 20;
				myInfo->Actived += 5;
				myInfo->cash -= 500000;
				mcEnv->Treasury += 500000;
				if ((random() % 4) && (myInfo->luck >= 90)) {
					myInfo->lastActiveTime -= 1800;
					myInfo->WorkPoint += 3600;
					move(20, 4);
					prints
					    ("\033[1;31m你启动了穿梭机！一阵天旋地转之后，世界好像变得有点不同了！！");
				}
				
				else {
					move(20, 4);
					prints
					    ("\033[1;31m你启动了穿梭机！突然一股呛人的烟味冒了出来，你好像碰上伪劣产品了...");
				}
				pressanykey();
				break;
			case '7':
				update_health();
				move(18, 4);
				if (askyn
				     ("千年参200,000元一只，据说能恢复体力，确实要购买吗",
				      NA, NA) == NA)
					break;
				if (myInfo->cash < 200000) {
					showAt(19, 4, "你没带够现金。", YEA);
					break;
				}
				myInfo->health += 10 + random() % 10;
				myInfo->Actived += 5;
				myInfo->cash -= 200000;
				mcEnv->Treasury += 200000;
				move(20, 4);
				prints
				    ("\033[1;31m你吞下一只千年人参！只觉得体力开始恢复...");
				pressanykey();
				break;
			case '8':
				update_health();
				move(18, 4);
				if (askyn
				     ("正气水500,000元一瓶，据说能减少疲劳，确实要购买吗",
				      NA, NA) == NA)
					break;
				if (check_health
				     (10, 19, 4, "你的体力不够了！", YEA))
					break;
				if (myInfo->cash < 500000) {
					showAt(19, 4, "你没带够现金。", YEA);
					break;
				}
				myInfo->WorkPoint += 1800 + random() % 1800;
				myInfo->health -= 5;
				myInfo->Actived += 5;
				myInfo->cash -= 500000;
				mcEnv->Treasury += 500000;
				move(20, 4);
				prints
				    ("\033[1;31m你服下一瓶正气水！只觉得头脑清醒了许多...");
				pressanykey();
				break;
			case 'q':
			case 'Q':
				quit = 1;
				break;
			}
			quit = 0;
			break;
		case '2':
			shop_card_show();
			break;
		case '3':
			money_show_stat("当铺柜台");
			update_health();
			if (check_health(1, 12, 4, "体力不够了！", YEA))
				break;
			myInfo->health--;
			shop_sellExp();
			break;
		case '4':
			money_show_stat("商场经理办公室");
			whoTakeCharge(7, uident);
			if (strcmp(currentuser->userid, uident))
				break;
			while (!quit) {
				nomoney_show_stat("商场业务");
				update_health();
				if (check_health 
				     (1, 12, 4, "歇会歇会，体力不够了！", YEA))
					break;
				move(t_lines - 1, 0);
				prints 
				    ("\033[1;44m 选单 \033[1;46m [1]采购 [2]定价 [3]查看商店资产 [Q]离开\033[m");
				ch = igetkey();
				switch (ch) {
				case '1':
					nomoney_show_stat("采购部");
					showAt(12, 4,
						"\033[1;32m规划中。\033[m",
						YEA);
					break;
				case '2':
					money_show_stat("市场部");
					showAt(12, 4,
						"\033[1;32m规划中。\033[m",
						YEA);
					break;
				case '3':
					money_show_stat("小金库");
					showAt(12, 4,
						"\033[1;32m会计正在点钱，请稍候。\033[m",
						YEA);
					break;
				case 'q':
					quit = 1;
					break;
				}
			}
			quit = 0;
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
void
EquipShop(int type) 
{
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
	char ch, quit = 0;
	char buf[256];
	int Wnum = 0, Anum = 0;
	while (!quit) {
		Wnum = myInfo->weapon * myInfo->weapon * myInfo->weapon + 1;
		Anum = myInfo->armor * myInfo->armor * myInfo->armor + 1;
		switch (type) {
		case 0:
			money_show_stat("装备店");
			move(5, 8);
			prints("磨刀不误砍柴功,买好装备事半功倍");
			move(t_lines - 1, 0);
			prints 
			    ("\033[1;44m 选单 \033[1;46m [1]武器 [2]护甲 [Q]离开\033[m");
			break;
		case 1:
			money_show_stat("黑帮军火库");
			move(5, 8);
			prints("要出去砍人了吗？先挑好趁手的家伙吧");
			move(t_lines - 1, 0);
			prints 
			    ("\033[1;44m 选单 \033[1;46m [1]枪支 [2]车辆 [Q]离开\033[m");
			break;
		case 2:
			money_show_stat("丐帮书屋");
			move(5, 8);
			prints("知识就是力量，多读书总是有好处的");
			move(t_lines - 1, 0);
			prints 
			    ("\033[1;44m 选单 \033[1;46m [1]书籍 [2]武功 [Q]离开\033[m");
			break;
		}
		ch = igetkey();
		switch (ch) {
		case '1':
			switch (type) {
			case 0:
				if (myInfo->weapon > 14) {
					showAt(7, 8, "小店没有更好的武器了",
						YEA);
					return;
				}
				if ((myInfo->robExp < 10 * Wnum)
				     || (myInfo->begExp < 10 * Wnum)) {
					sprintf(buf,
						 "这把\033[1;33m%s\033[m太锋利了，不适合你,等你属性到\033[1;33m%d\033[m再说吧",
						 FreeWeapon[myInfo->weapon + 1],
						 10 * Wnum);
					showAt(7, 8, buf, YEA);
					return;
				}
				if (myInfo->cash < 20000 * Wnum) {
					sprintf(buf,
						 "这把\033[1;33m%s\033[m%d%s,你还是带够钱再来吧",
						 FreeWeapon[myInfo->weapon + 1],
						 20000 * Wnum, MONEY_NAME);
					showAt(7, 8, buf, YEA);
					return;
				}
				sprintf(buf,
					   "这把\033[1;33m%s\033[m%d%s,你确定要买吗？",
					   FreeWeapon[myInfo->weapon + 1],
					   20000 * Wnum, MONEY_NAME);
				move(7, 8);
				if (askyn(buf, NA, NA) == YEA) {
					myInfo->weapon++;
					myInfo->cash -= 20000 * Wnum;
					mcEnv->Treasury += 20000 * Wnum;
				}
				break;
			case 1:
				if (myInfo->weapon > 14) {
					showAt(7, 8,
						"更好的武器还没走私过来呢",
						YEA);
					return;
				}
				if (myInfo->robExp < 10 * Wnum) {
					sprintf(buf,
						 "这把\033[1;33m%s\033[m火力太猛了，你用浪费,等你属性到\033[1;33m%d\033[m再说吧",
						 RobWeapon[myInfo->weapon + 1],
						 10 * Wnum);
					showAt(7, 8, buf, YEA);
					return;
				}
				if (myInfo->cash < 20000 * Wnum) {
					sprintf(buf,
						 "这把\033[1;33m%s\033[m%d%s,你还是带够了钱再来吧",
						 RobWeapon[myInfo->weapon + 1],
						 20000 * Wnum, MONEY_NAME);
					showAt(7, 8, buf, YEA);
					return;
				}
				sprintf(buf,
					   "这把\033[1;33m%s\033[m%d%s,你确定要买吗？",
					   RobWeapon[myInfo->weapon + 1],
					   20000 * Wnum, MONEY_NAME);
				move(7, 8);
				if (askyn(buf, NA, NA) == YEA) {
					myInfo->weapon++;
					myInfo->cash -= 20000 * Wnum;
					mcEnv->Treasury += 20000 * Wnum;
				}
				break;
			case 2:
				if (myInfo->weapon > 14) {
					showAt(7, 8, "新书预订中", YEA);
					return;
				}
				if (myInfo->begExp < 10 * Wnum) {
					sprintf(buf,
						 "这本\033[1;33m%s\033[m太深奥了，你看不懂,等你属性到\033[1;33m%d\033[m再说吧",
						 BegWeapon[myInfo->weapon + 1],
						 10 * Wnum);
					showAt(7, 8, buf, YEA);
					return;
				}
				if (myInfo->cash < 20000 * Wnum) {
					sprintf(buf,
						 "这本\033[1;33m%s\033[m%d%s,你还是带够了钱再来吧",
						 BegWeapon[myInfo->weapon + 1],
						 20000 * Wnum, MONEY_NAME);
					showAt(7, 8, buf, YEA);
					return;
				}
				sprintf(buf,
					   "这本\033[1;33m%s\033[m%d%s,你确定要买吗？",
					   BegWeapon[myInfo->weapon + 1],
					   20000 * Wnum, MONEY_NAME);
				move(7, 8);
				if (askyn(buf, NA, NA) == YEA) {
					myInfo->weapon++;
					myInfo->cash -= 20000 * Wnum;
					mcEnv->Treasury += 20000 * Wnum;
				}
				break;
			}
			break;
		case '2':
			switch (type) {
			case 0:
				if (myInfo->armor > 14) {
					showAt(7, 8, "小店没有更好的护甲了",
						YEA);
					return;
				}
				if ((myInfo->robExp < 10 * Anum)
				     || (myInfo->begExp < 10 * Anum)) {
					sprintf(buf,
						 "这件\033[1;33m%s\033[m你穿没必要,等你属性到\033[1;33m%d\033[m再说吧",
						 FreeArmor[myInfo->armor + 1],
						 10 * Anum);
					showAt(7, 8, buf, YEA);
					return;
				}
				if (myInfo->cash < 20000 * Anum) {
					sprintf(buf,
						 "这件\033[1;33m%s\033[m%d%s,你还是带够了钱再来吧",
						 FreeArmor[myInfo->armor + 1],
						 20000 * Anum, MONEY_NAME);
					showAt(7, 8, buf, YEA);
					return;
				}
				sprintf(buf,
					   "这件\033[1;33m%s\033[m%d%s,你确定要买吗？",
					   FreeArmor[myInfo->armor + 1],
					   20000 * Anum, MONEY_NAME);
				move(7, 8);
				if (askyn(buf, NA, NA) == YEA) {
					myInfo->armor++;
					myInfo->cash -= 20000 * Anum;
					mcEnv->Treasury += 20000 * Anum;
				}
				break;
			case 1:
				if (myInfo->armor > 14) {
					showAt(7, 8, "要好车？自己抢去", YEA);
					return;
				}
				if (myInfo->robExp < 10 * Anum) {
					sprintf(buf,
						 "这辆\033[1;33m%s\033[m跑太快你，你会翻车的,等你属性到\033[1;33m%d\033[m再说吧",
						 RobArmor[myInfo->armor + 1],
						 10 * Anum);
					showAt(7, 8, buf, YEA);
					return;
				}
				if (myInfo->cash < 20000 * Anum) {
					sprintf(buf,
						 "这辆\033[1;33m%s\033[m%d%s,你还是带够了钱再来吧",
						 RobArmor[myInfo->armor + 1],
						 20000 * Anum, MONEY_NAME);
					showAt(7, 8, buf, YEA);
					return;
				}
				sprintf(buf,
					   "这辆\033[1;33m%s\033[m%d%s,你确定要买吗？",
					   RobArmor[myInfo->armor + 1],
					   20000 * Anum, MONEY_NAME);
				move(7, 8);
				if (askyn(buf, NA, NA) == YEA) {
					myInfo->armor++;
					myInfo->cash -= 20000 * Anum;
					mcEnv->Treasury += 20000 * Anum;
				}
				break;
			case 2:
				if (myInfo->armor > 14) {
					showAt(7, 8,
						"原创秘笈更新暂停，估计太监了，没得看了",
						YEA);
					return;
				}
				if (myInfo->begExp < 10 * Anum) {
					sprintf(buf,
						 "\033[1;33m%s\033[m不是你现在的内力能修炼的,等你属性到\033[1;33m%d\033[m再说吧",
						 BegArmor[myInfo->armor + 1],
						 10 * Anum);
					showAt(7, 8, buf, YEA);
					return;
				}
				if (myInfo->cash < 20000 * Anum) {
					sprintf(buf,
						 "这本\033[1;33m%s\033[m%d%s,你还是带够了钱再来吧",
						 BegArmor[myInfo->armor + 1],
						 20000 * Anum, MONEY_NAME);
					showAt(7, 8, buf, YEA);
					return;
				}
				sprintf(buf,
					   "这本\033[1;33m%s\033[m%d%s,你确定要买吗？",
					   BegArmor[myInfo->armor + 1],
					   20000 * Anum, MONEY_NAME);
				move(7, 8);
				if (askyn(buf, NA, NA) == YEA) {
					myInfo->armor++;
					myInfo->cash -= 20000 * Anum;
					mcEnv->Treasury += 20000 * Anum;
				}
				break;
			}
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	return;
}


