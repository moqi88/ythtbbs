#include "bbs.h"
#include "ythtbbs.h"
#include "moneycenter.h"
#include "bbslib.h"
#define REGFORM MY_BBS_HOME "/wwwtmp/tshirt"
#define TSHIRTADMIN MY_BBS_HOME "/wwwtmp/tshirtadmin"
#define TSHIRTBOARD "TshirtInfo"
#define MINPOSTFEE 8
#define BASEPOSTFEE 2
#define PAGESIZE 10
#define SIZEHELP "con?B=330&F=M.1122796285.A&T=0"
#define ORDERHELP "con?B=330&F=M.1122796674.A&T=0"
#define PUBBOARD "Tshirt"
#define MAXTYPE 48		//最多48个品种

struct orderitem {
	int seq_num;		//流水号
	char userid[IDLEN + 1];
	char name[40];
	int type;		//0 邮寄 1 自取
	char address[200];
	char postcode[12];
	char email[80];
	char phone[30];
	int amount[MAXTYPE];	//最多48个站衫品种，保存订购的数量
	int bonus[10];		//赠品数量
	time_t time;
	int status;		//状态，0 成功订购 1 收到货款 2 已发货 -1 被删除
};

const int itemsize = sizeof (struct orderitem);

const char *bonusname[] = {
	"糊涂币+200000",
	"根骨+2",
	"精华值+20",
	0
};

const static struct tshirt {
	int num;
	char sex;
	char size[5];
	char color[4];
	int price;
	int postfee;
} tshirts[] = {
	{
	1, 'B', "S", "白", 30, 1}, {
	2, 'B', "S", "黑", 30, 1}, {
	3, 'B', "M", "白", 30, 1}, {
	4, 'B', "M", "黑", 30, 1}, {
	5, 'B', "L", "白", 30, 1}, {
	6, 'B', "L", "黑", 30, 1}, {
	7, 'B', "XL", "白", 30, 1}, {
	8, 'B', "XL", "黑", 30, 1}, {
	9, 'B', "XXL", "白", 30, 1}, {
	10, 'B', "XXL", "黑", 30, 1}, {
	11, 'B', "XXXL", "白", 30, 1}, {
	12, 'B', "XXXL", "黑", 30, 1}, {
	13, 'B', "背包", "黑", 50, 2}, {
	14, 'F', "S(F)", "白", 30, 1}, {
	15, 'F', "S(F)", "黑", 30, 1}, {
	0, 'F', "F", "F", 0, 0}
};

void printcss(void);
void printform(void);
int getform(struct orderitem *item);
void checkuserinfo(struct orderitem *item);
int saveitem(struct orderitem *item);
void printmyorders(char *userid);
void printorderdetail(struct orderitem *item);
void printstatus(struct orderitem *item);
int isadmin(void);
int ismine(int seq, struct orderitem *item);
void query(int seq);
int update(int seq, int status);	//更改状态
void post_record(char *title, struct orderitem *item);
void admin(void);
void printitemhead(void);
void printitemline(struct orderitem *item);
void printitemend(void);
void printitemoper(struct orderitem *item);
void printitembystatus(int fd, struct orderitem *item, int status, int offset,
		       int admintype);
void dobonus(struct orderitem *item);
void printoffsetnav(int admintype, int offset);
void fixorders(void);

int
bbstshirt_main()
{
	char action;
	static struct orderitem *item = NULL;

	if (!item)
		item = (struct orderitem *) malloc(itemsize);
	if (!item)
		http_fatal("No Memory?");

	action = getparm("action")[0];

	html_header(1);

	if (!loginok || isguest) {
		printf("匆匆过客不能订购T Shirt，请先登录!<br>");
		printf
		    ("如果您确实已经登录却看到这个提示，请尝试了解登录时的 IP验证范围 选项<br>");
		printf("[<a href='javascript:history.go(-1)'>返回</a>]");
		http_quit();
	}

	printcss();

	printf("<script src=/tshirt.js></script>");

	switch (action) {
	case 'n':
		if (getform(item) == 0) {
			printf("<div align=center><div class=msgbox>");
			printf("<p>您一件也没有订购啊！请点击 [<a href=#"
			       " onclick=history.go(-1)>这里</a>] 返回上一页重新填写。</p>");
			printf("</div>");
			return -1;
		}
		printf("<div align=center><div class=msgbox>");
		checkuserinfo(item);	//检验是否符合要求

		if (saveitem(item) <= 0)
			http_fatal("保存订单信息失败。");

		printf("<p>订单提交成功，以下为详情，点击"
		       " [<a href=tshirt>这里</a>] 返回订购界面。"
		       "</p></div></div><br><br>");
		printorderdetail(item);
		break;

	case 'q':
		query(atoi(getparm("seq")));
		break;

	case 'u':
		update(atoi(getparm("seq")), atoi(getparm("status")));
		break;

	default:
		if (isadmin()) {
			printf("<h1>管理选项</h1>");
			admin();
		}
		printf("<h1>订购站衫(<a href = " ORDERHELP
		       "><font color=red>订购系统帮助</font></a>，"
		       "</h1>");
		printform();
		printf("<br><br><h1>我的订单</h1>");
		printmyorders(currentuser->userid);
	}
	return 0;
}

void
printcss()
{
	printf("<style>.inputbox{\n"
	       "width: 90%%; \n"
	       "border-color: #67a8ea; \n"
	       "border-width: 1px; \n"
	       "border-style: solid; \n"
	       "background-color: #fff;}\n"
	       "tr {text-align: center;}\n"
	       "td {border-width: 0px; "
	       "border-style: solid; "
	       "border-color: #67a8ea; "
	       "background-color: #eee; "
	       "padding: 4px 10px 4px 10px;}\n"
	       ".msgbox {"
	       "width: 85%%;"
	       "background-color: #eee;"
	       "padding: 10px 10px 10px 10px;"
	       "border-color: #67a8ea;"
	       "border-width: 1px;"
	       "border-style: solid;"
	       "float: center;"
	       "}\n"
	       "h1 {"
	       "border-style: dotted dotted dotted solid; "
	       "border-color: rgb(124, 189, 255); "
	       "border-width: 1px 1px 1px 15px; "
	       "margin: 0pt 0pt 10px; font-size: "
	       "14px; line-height: 20px; "
	       "padding: 12 0 12 25px; "
	       "text-align: left; "
	       "background-color: rgb(234, 244, 255);" "</style>");
}

void
printform()
{
	int tshirt_num = 0;
	int bonus_num = 0;
	struct userdata currentdata;

	loaduserdata(currentuser->userid, &currentdata);
	printf("<form action='tshirt?action=new' method=post>"
	       "<table width=100%% align=center>"
	       "<tr><td width=25%%>您的ID</td><td>"
	       "<input type=text name=userid value=%s "
	       "class=inputbox disabled></tr>"
	       "<tr><td>收货方式</td><td>"
	       "<input type=radio name=usertype value=0 checked "
	       "onclick=show_sh_mail()>邮寄　　"
	       "<input type=radio name=usertype value=1 "
	       "onclick=show_sh_self()>自取 (限北京市)"
	       "<input type=radio name=usertype value=2 "
	       "onclick=show_sh_sh() disabled>送货上门 (限北京市)</tr>"
	       "<tr><td>姓名</td><td>"
	       "<input type=text name=username value=\"%s\""
	       "maxlength=20 class=inputbox></tr>"
	       "<tr><td><span id=sh_mail1>邮寄</span>地址</td><td><input type=text "
	       "name=useraddress maxlength=100 class=inputbox value=\"%s\"></tr>"
	       "<tr><td><span id=sh_mail2>邮政编码</span><span id=sh_sh1 style=\"DISPLAY: none\">送货时间</span></td>"
	       "<td><span id=sh_mail3><input type=text "
	       "name=userpostcode maxlength=6 class=inputbox value=%d></span>"
	       "<span id=sh_sh2 style=\"DISPLAY: none\"><input type=text "
	       "name=sendtime maxlength=11 class=inputbox></span></tr>"
	       "<tr><td>电子信箱(可选)</td><td><input type=text "
	       "name=useremail maxlength=40 class=inputbox value=\"%s\"></tr>"
	       "<tr><td>联系电话</td><td><input type=text "
	       "name=userphone maxlength=15 class=inputbox value=\"%s\"></tr>"
	       "<tr><td valign=middle>订购样式"
	       "<br><br><a href= " SIZEHELP
	       "><font color=red>查看尺寸详情</font></a>" "</td><td>",
	       currentuser->userid, currentdata.realname, currentdata.address,
	       currentdata.postcode, currentdata.email, currentdata.phone);
	printf("<table width=100%%>");
	printf("<tr><td>样式<td>性别<td>大小<td>颜色<td>价格<td>数量</tr>");
	while (tshirts[tshirt_num].num != 0) {
		printf("<tr><td>%4d<td>%s<td>%s<td>%s<td>%d<td>"
		       "<input type=text name=tshirt%d maxlength=2 class=inputbox></tr>",
		       tshirts[tshirt_num].num,
		       tshirts[tshirt_num].sex == 'M' ? "男" :
		       tshirts[tshirt_num].sex == 'B' ? "男/女" : "女",
		       tshirts[tshirt_num].size,
		       tshirts[tshirt_num].color,
		       tshirts[tshirt_num].price, tshirt_num);
		tshirt_num++;
	}
	printf("</table>");
	printf
	    ("<tr><td>&nbsp;</td><td><input style=\"width: 33%%\" type=reset value=重写>"
	     "<input style=\"width: 33%%\" type=submit value=提交></td></tr>");
	printf("<tr><td>赠　　品<td align=left>"
	       "<p>　　为了答谢您对本次站衫订购活动的支持，我们特别推出以下赠品供您选择，"
	       "每件站衫可以选择一件赠品，请在您选择的赠品后面输入需要的数量，赠品总数超过"
	       "您订购的站衫总数的部分将被自动忽略。(其中糊涂币与根骨是本站大富翁游戏相关，"
	       "精华值是经验值一部分。详情可到 <a href=bbshome?B=BBSHelp>BBSHelp</a> 版查询。)<p>"
	       "<div align=center>");
	while (bonusname[bonus_num] != 0) {
		printf
		    ("%s <input name=bonus%d type=text maxlength=2 class=inputbox style='width: 20px'>　　",
		     bonusname[bonus_num], bonus_num);
		bonus_num++;
	}
	printf("<tr><td></td><td align=left><br><p>　　以上信息不会泄露给任何第三方，"
	     "除订购站衫外，也不会在站内被用于其它用途。"
	     "如果您有任何疑问和建议，或有需要特殊说明的情况请到SYSOP版提出。</p>"
	     "<p>　　如果选择自取方式可以不必填写通讯地址和邮政编码。</p>"
	     "<p>　　以上内容请您慎重填写，一旦提交将不能修改。</p>"
	     "</table></form>");
}

int
getform(struct orderitem *item)
{
	int num = 0, amount, total = 0, i = 0, r;
	char buf[10];

	memset(item, 0, sizeof (*item));
	strsncpy(item->userid, currentuser->userid, sizeof (item->userid));
	strsncpy(item->name, getparm("username"), sizeof (item->name));
	item->type = atoi(getparm("usertype"));
	strsncpy(item->address, getparm("useraddress"), sizeof (item->address));
	strsncpy(item->postcode, getparm("userpostcode"),
		 sizeof (item->postcode));
	strsncpy(item->email, getparm("useremail"), sizeof (item->email));
	strsncpy(item->phone, getparm("userphone"), sizeof (item->phone));
	if (item->type == 2)
		strsncpy(item->postcode, getparm("sendtime"),
			 sizeof (item->postcode));

	while (tshirts[num].num != 0) {	//获取站衫订购详情
		sprintf(buf, "tshirt%d", num);
		amount = atoi(getparm(buf));
		if (amount > 0) {
			item->amount[num] = amount;
			total += amount;
		} else
			item->amount[num] = 0;
		num++;
	}
	r=total;

	while (bonusname[i] != 0) {	//获取赠品信息
		sprintf(buf, "bonus%d", i);
		if ((amount = atoi(getparm(buf))) > 0 && total > 0) {
			item->bonus[i] = amount > total ? total : amount;
			total -= amount;
		} else
			item->bonus[i] = 0;
		i++;
	}

	item->status = 0;
	return r;
}

void
checkuserinfo(struct orderitem *item)
{
	int retv = 1;
	struct userdata currentdata;
#if 1
	if (!strcasecmp(item->userid, "guest") || item->userid == NULL) {
		printf
		    ("匆匆过客不能订购站衫，请从左侧选择登录后重新订购。<br>");
		retv = 0;
	}
#endif
	loaduserdata(item->userid, &currentdata);
	if (strlen(item->name) < 4) {
		printf("　　·  请留下您的大名。<br>");
		retv = 0;
	} else
		strsncpy(currentdata.realname, item->name,
			 sizeof (currentdata.realname));
	if (strlen(item->address) < 4) {
		printf("　　·  请留下您的地址。<br>");
		retv = 0;
	} else
		strsncpy(currentdata.address, item->address,
			 sizeof (currentdata.address));
	if (strlen(item->phone) < 6) {
		printf("　　·  请留下您的电话。<br>");
		retv = 0;
	} else
		strsncpy(currentdata.phone, item->phone,
			 sizeof (currentdata.phone));

	if (item->type == 0) {
		if (strlen(item->postcode) < 6) {
			printf("　　·  请留下您的邮编。<br>");
			retv = 0;
		} else
			currentdata.postcode = atoi(item->postcode);
	} else if (item->type == 2) {
		if (strlen(item->postcode) < 6) {
			printf("　　·  请留下合适的送货时间。<br>");
			retv = 0;
		}
	} else if (item->type != 1) {
		printf
		    ("　　·  请选择邮购或自取或者送货上门 三种递送方式之一。<br>");
		retv = 0;
	}
	if (strlen(item->email) < 4) {
#if 0
		printf("　　·  请留下您的电子信箱。<br>");
		retv = 0;
#endif
	} else
		strsncpy(currentdata.email, item->email,
			 sizeof (currentdata.email));
	saveuserdata(item->userid, &currentdata);
	if (retv == 0) {
		printf("<p>订单提交失败，请点击 [<a href=#"
		       " onclick=history.go(-1)>这里</a>] 返回上一页重新填写。</p>");
		printf("</div>");
		http_quit();
	}
}

int
saveitem(struct orderitem *item)
{
	int fd;
	char title[STRLEN];

	if ((fd = open(REGFORM, O_RDWR | O_CREAT, 0600)) < 0)
		return 0;

	if (lseek(fd, file_size(REGFORM), SEEK_SET) < 0)
		return 0;

	item->time = time(NULL);
	item->seq_num = file_size(REGFORM) / itemsize;
	flock(fd, LOCK_EX);
	write(fd, item, itemsize);
	flock(fd, LOCK_UN);
	close(fd);

	sprintf(title, "[%04d] %s 提交新订单", item->seq_num, item->userid);
	post_record(title, item);

	return 1;
}

void
post_record(char *title, struct orderitem *tmp)
{
	int num = 0, total = 0, price = 0, postfee = 0;
	int num1 = 0, num2 = 0;
	char fname[STRLEN];
	FILE *fp;

	sprintf(fname, "bbstmpfs/tmp/tshirt.%s.%05d", currentuser->userid,
		getpid());
	if ((fp = fopen(fname, "w")) != NULL) {
		fprintf(fp, "%s\n\n", title);
		fprintf(fp, "以下为订单详细信息：\n\n");
		fprintf(fp, "流 水 号\t%04d\n", tmp->seq_num);
		fprintf(fp, "订单时间\t%s\n", Ctime(tmp->time));
		fprintf(fp, "订 购 ID\t%s\n", tmp->userid);
		if (tmp->type == 0) {
			fprintf(fp, "收货方式\t邮寄\n");
			fprintf(fp, "收 件 人\t%s\n", tmp->name);
			fprintf(fp, "收件地址\t%s\n", tmp->address);
			fprintf(fp, "邮政编码\t%s\n", tmp->postcode);
		} else if (tmp->type == 1) {
			fprintf(fp, "收货方式\t自取\n");
		} else if (tmp->type == 2) {
			fprintf(fp, "收货方式\t送货上门\n");
			fprintf(fp, "收 件 人\t%s\n", tmp->name);
			fprintf(fp, "送货地址\t%s\n", tmp->address);
			fprintf(fp, "送货时间\t%s\n", tmp->postcode);
		}
		fprintf(fp, "电子邮件\t%s\n", tmp->email);
		fprintf(fp, "联系电话\t%s\n", tmp->phone);
		fprintf(fp, "订货种类\t");
		while (tshirts[num].num != 0) {
			if (tmp->amount[num] > 0) {
				total += tmp->amount[num];
				price += tmp->amount[num] * tshirts[num].price;
				if (tshirts[num].postfee == 1)
					num1 += tmp->amount[num];
				else
					num2 += tmp->amount[num];
				fprintf(fp,
					"[样式%d] %s 式 %s 号 %s 色 %d 件 %d 元\n\t\t",
					tshirts[num].num,
					tshirts[num].sex ==
					'M' ? "男" : tshirts[num].sex ==
					'F' ? "女" : "男/女", tshirts[num].size,
					tshirts[num].color, tmp->amount[num],
					tmp->amount[num] * tshirts[num].price);
			}
			num++;
		}
		if (num1 >= 2)
			postfee = (total - 2) * BASEPOSTFEE + MINPOSTFEE;
		else {
			if (num2 > 0)
				postfee =
				    (total - 1) * BASEPOSTFEE + MINPOSTFEE;
			else
				postfee = MINPOSTFEE;
		}
		fprintf(fp, "\n\t\t共计：%d 件 %d 元。", total, price);
		if (tmp->type == 0)
			fprintf(fp, "另有邮资 %d 元。总金额 %d 元。", postfee,
				postfee + price);
		if (tmp->type == 2)
			fprintf(fp, "另有送货费 10 元。总金额 %d 元。", 10 + price);
		fprintf(fp, "\n\n当前状态\t");
		switch (tmp->status) {
		case -1:
			fprintf(fp, "订单已被删除。");
			break;
		case 0:
			fprintf(fp, "订单等待处理中。");
			break;
		case 1:
			fprintf(fp, "货款已收到，即将邮寄。");
			break;
		case 2:
			if (tmp->type == 0)
				fprintf(fp, "站衫已寄出，请注意查收。");
			else
				fprintf(fp, "交易成功。");
			break;
		default:
			break;
		}
		fclose(fp);
		post_article(TSHIRTBOARD, title, fname, currentuser->userid,
			     currentuser->username, fromhost, -1, 0, 0,
			     currentuser->userid, -1);
		post_mail(tmp->userid, title, fname, currentuser->userid,
			  currentuser->username, fromhost, 0, -1);
		unlink(fname);
	}

}

void
printstatus(struct orderitem *item)
{
	int status = item->status;
	int type = item->type;

	if (status == -1)
		printf("本单已作废");
	else if (status == 0)
		printf("订单已提交");
	else if (status == 1)
		printf("已收到货款，即将发货");
	else if (status == 2 && type == 0)
		printf("已发货，请注意查收");
	else if (status == 2 && type == 1)
		printf("已交易成功");
	else if (status == 2 && type == 2)
		printf("交易成功, 请等待送货!\n");
	else
		printf("状态错误，请联系系统维护");
}

void
printorderdetail(struct orderitem *item)
{
	int num = 0, amount = 0, price = 0, i = 0, postfee = 0;
	int num1 = 0, num2 = 0, total = 0;

	printf("<table width=100%% align=center>");
	printf("<tr><td width=25%%>流水号<td width=25%%>%04d", item->seq_num);
	printf("<td width=25%%>订单时间<td width=25%%>%s</tr>",
	       Ctime(item->time));
	printf("<tr><td>订购ID<td>%s", item->userid);
	if (item->type == 0) {
		printf("<td>收货方式<td>邮寄</tr>");
		printf("<tr><td>收件人<td>%s", item->name);
		printf("<td>邮政编码<td>%s</tr>", item->postcode);
		printf("<tr><td>收件地址<td colspan=3>%s</tr>", item->address);
	} else if (item->type == 1) {
		printf("<td>收货方式<td>自取</tr>");
	} else {
		printf("<td>收货方式<td>送货</tr>");
		printf("<tr><td>收件人<td>%s", item->name);
		printf("<td>送货时间<td>%s</tr>", item->postcode);
		printf("<tr><td>送货地址<td colspan=3>%s</tr>", item->address);
	}

	printf("<tr><td>电话<td colspan=3>%s</tr>", item->phone);
	printf("<tr><td>订货种类<td colspan=3><p>");

	while (tshirts[num].num != 0) {
		if (item->amount[num] > 0) {
			total += item->amount[num];
			printf
			    ("[款式%d] %s 式 %s 号 %s 色 %d 件，价值 %d 元。<br>",
			     tshirts[num].num,
			     tshirts[num].sex ==
			     'M' ? "男" : tshirts[num].sex ==
			     'B' ? "男/女" : "女", tshirts[num].size,
			     tshirts[num].color, item->amount[num],
			     tshirts[num].price * item->amount[num]);
			amount += item->amount[num];
			price += item->amount[num] * tshirts[num].price;
			if (tshirts[num].postfee == 1)
				num1 += item->amount[num];
			else
				num2 += item->amount[num];
		}
		num++;
	}

	printf("</p></tr>");
	if (num1 >= 2)
		postfee = (total - 2) * BASEPOSTFEE + MINPOSTFEE;
	else {
		if (num2 > 0)
			postfee = (total - 1) * BASEPOSTFEE + MINPOSTFEE;
		else
			postfee = MINPOSTFEE;
	}
	printf("<tr><td>累计<td colspan=3>站衫 <font color=red>%d</font> 件，"
	       "共计 <font color=red>%d</font> 元。", amount, price);
	if (item->type == 0)
		printf
		    ("另有邮资 <font color=red>%d</font> 元。总金额 <font color=red>%d</font> 元。</tr>",
		     postfee, postfee + price);
	if (item->type == 2)
		printf
		    ("另有送货费 <font color=red>10</font> 元。总金额 <font color=red>%d</font> 元。</tr>",
		     10 + price);
	printf("<tr><td>赠品<td colspan=3>");
	while (bonusname[i] != 0) {
		if (item->bonus[i] != 0)
			printf("%s * %d　　", bonusname[i], item->bonus[i]);
		i++;
	}
	printf("<tr><td>状态<td colspan=3>");
	printstatus(item);
	printf("</tr></table>");
}

void
printmyorders(char *userid)
{
	int fd, total, i, retv = 0;
	struct orderitem *tmp;

	printf("<table width=100%% align=center>");
	printf("<tr><td>流水号<td>用户名<td>订购时间<td>状态<td>操作</tr>");

	if ((tmp = (struct orderitem *) malloc(itemsize)) == NULL)
		http_fatal("No Memory?");

	if ((total = (file_size(REGFORM) / itemsize)) <= 0) {
		printf("<tr><td colspan=5>没有任何订单信息。</tr>");
		return;
	}

	if ((fd = open(REGFORM, O_RDWR | O_CREAT, 0600)) < 0)
		return;

	for (i = 0; i < total; i++) {
		if (lseek(fd, i * itemsize, SEEK_SET) < 0)
			http_fatal("Memory Error.");
		if (read(fd, tmp, itemsize) != itemsize)
			http_fatal("Memory Error.");
		if (!strcasecmp(tmp->userid, userid)) {
			printitemline(tmp);
			retv = 1;
		}
	}

	close(fd);

	if (!retv)
		printf("<tr><td colspan=5>没有任何订单信息。</tr>");
	printf("</table><br><br>");
}

int
isadmin()
{
	if (USERPERM(currentuser, PERM_SYSOP))
		return 1;
	if (file_has_word(TSHIRTADMIN, currentuser->userid))
		return 1;
	return 0;
}

int
ismine(int seq, struct orderitem *item)
{
	int fd;

	if (!item)
		http_fatal("Memory Error");
	if ((fd = open(REGFORM, O_RDWR | O_CREAT, 0600)) < 0)
		http_fatal("File Error");
	if (lseek(fd, itemsize * seq, SEEK_SET) < 0) {
		printf
		    ("<div class=msgbox>订单不存在或您无权查看此订单信息。</div>");
		http_quit();
	}
	if (read(fd, item, itemsize) != itemsize) {
		printf
		    ("<div class=msgbox>订单不存在或您无权查看此订单信息。</div>");
		http_quit();
	}
	close(fd);

	if (strcmp(item->userid, currentuser->userid))
		return 0;

	return 1;
}

void
query(int seq)
{
	struct orderitem *item;

	item = (struct orderitem *) malloc(itemsize);
	if (!item)
		http_fatal("Memory Error");

	if (!ismine(seq, item) && !isadmin()) {
		printf
		    ("<div class=msgbox>订单不存在或您无权查看此订单信息。</div>");
		http_quit();
	}

	printf("<h1>查看订单详情</h1>");
	printorderdetail(item);
}

int
update(int seq, int status)
{
	int fd, oldstatus;
	struct orderitem *item;
	char title[STRLEN];

	item = (struct orderitem *) malloc(itemsize);

	if (!item)
		http_fatal("Memory Error");
	if ((fd = open(REGFORM, O_RDWR | O_CREAT, 0600)) < 0)
		http_fatal("File Error");
	if (lseek(fd, itemsize * seq, SEEK_SET) < 0) {
		close(fd);
		printf
		    ("<div class=msgbox>订单不存在或您无权处理此订单信息。</div>");
		http_quit();
	}
	if (read(fd, item, itemsize) != itemsize) {
		close(fd);
		printf
		    ("<div class=msgbox>订单不存在或您无权处理此订单信息。</div>");
		http_quit();
	}

	if (strcmp(item->userid, currentuser->userid) && !isadmin()) {
		close(fd);
		printf
		    ("<div class=msgbox>订单不存在或您无权处理此订单。</div>");
		http_quit();
	}

	oldstatus = item->status;
	item->status = status;

	if ((status > 0 && !isadmin()) || (status == -1 && oldstatus != 0)) {
		printf
		    ("<div class=msgbox>订单不存在或您无权处理此订单。</div>");
		http_quit();
	}

	flock(fd, LOCK_EX);
	if (lseek(fd, itemsize * seq, SEEK_SET) < 0) {
		close(fd);
		printf
		    ("<div class=msgbox>订单不存在或您无权处理此订单信息。</div>");
		http_quit();
	}
	write(fd, item, itemsize);
	flock(fd, LOCK_UN);
	close(fd);

	printf
	    ("<script>alert(\"操作已成功\"); location.href='tshirt';</script>");
	if (status == 2)
		dobonus(item);

	sprintf(title, "[%04d] %s 将订单状态由 %d 变为 %d", item->seq_num,
		currentuser->userid, oldstatus, status);
	post_record(title, item);

	return 1;
}

void
admin()
{
	int fd, oper, retv = 0, newfd, i;
	int offset = atoi(getparm("offset")), pos = 0;
	int statistic[MAXTYPE + 1][4];
	char buf[STRLEN];
	struct orderitem *item;

	offset = offset >= 0 ? offset : 0;
	bzero(statistic, sizeof (statistic));

	if (!isadmin())
		http_fatal("没有管理权限或登录已超时。");

	if ((item = (struct orderitem *) malloc(itemsize)) == NULL)
		http_fatal("Memory Error.");

	if ((fd = open(REGFORM, O_RDWR | O_CREAT, 0600)) < 0)
		http_fatal("File Error.");

	printf("<table width=100%%><tr>"
	       "<td width=9%%><a href='tshirt?admin=0'>所有订单</a>"
	       "<td width=10%%><a href='tshirt?admin=1'>已删除订单</a>"
	       "<td width=10%%><a href='tshirt?admin=2'>未处理订单</a>"
	       "<td width=10%%><a href='tshirt?admin=3'>已到款订单</a>"
	       "<td width=10%%><a href='tshirt?admin=4'>已交易订单</a>"
	       "<td width=6%%><a href='tshirt?admin=8'>统计</a>"
	       "<td width=13%%><a href='tshirt?admin=7'><font color=green>修复订单信息</font></a></td>"
	       "<form action=tshirt?admin=5 method=post>"
	       "<td width=14%%><input type=text class=inputbox name=quserid"
	       " maxlength=14 style='width: 50%%'>&nbsp;"
	       "<input type=submit value='ID'>"
	       "</td></form>"
	       "<form action=tshirt?admin=6 method=post>"
	       "<td width=18%%><input type=text class=inputbox name=qseq "
	       "maxlength=4 style='width: 50%%'>&nbsp;"
	       "<input type=submit value='流水号'></td></form>"
	       "</tr></table><br><br>");

	oper = atoi(getparm("admin"));
	switch (oper) {
	default:
	case 0:
		printoffsetnav(0, offset);
		printitemhead();
		while (read(fd, item, itemsize) == itemsize) {
			if (pos >= PAGESIZE * offset
			    && pos < PAGESIZE * (offset + 1)) {
				printitemline(item);
				retv = 1;
			}
			pos++;
		}
		if (!retv)
			printf("<tr><td colspan=5>没有找到符合条件的订单</tr>");
		printitemend();
		printoffsetnav(0, offset);
		break;

	case 1:
		printitembystatus(fd, item, -1, offset, 1);
		break;

	case 2:
		printitembystatus(fd, item, 0, offset, 2);
		break;

	case 3:
		printitembystatus(fd, item, 1, offset, 3);
		break;

	case 4:
		printitembystatus(fd, item, 2, offset, 4);
		break;

	case 5:
		printmyorders(getparm("quserid"));
		break;

	case 6:
		if (lseek(fd, itemsize * atoi(getparm("qseq")), SEEK_SET) < 0) {
			close(fd);
			http_fatal("File Error");
		}
		if (read(fd, item, itemsize) != itemsize) {
			close(fd);
			http_fatal("File Error");
		}

		printitemhead();
		printitemline(item);
		printitemend();
		break;
	case 7:
		sprintf(buf, "%s.tmp", REGFORM);
		if ((newfd = open(buf, O_RDWR | O_CREAT, 0600)) < 0)
			http_fatal("File Error.");
		flock(fd, LOCK_EX);
		while (read(fd, item, itemsize) == itemsize) {
			i = 0;
			while (tshirts[i].num != 0) {
				if (item->amount[i] < 0 || item->amount[i] > 99)
					item->amount[i] = 0;
				i++;
			}
			flock(newfd, LOCK_EX);
			write(newfd, item, itemsize);
			flock(newfd, LOCK_UN);
		}
		flock(fd, LOCK_UN);
		close(newfd);
		close(fd);
		unlink(REGFORM);
		rename(buf, REGFORM);
		printf("<div class=msgbox>数据修复成功</div>");
		return;
	case 8:
		while (read(fd, item, itemsize) == itemsize) {
			i = 0;
			while (tshirts[i].num != 0) {
				if (item->amount[i] > 0)
					statistic[i][item->status + 1]++;
				i++;
			}
		}
		printf("<table width=100%%>"
		       "<tr><td width=16%%>样式"
		       "<td width=12%%>已删除[-1]"
		       "<td width=12%%>未处理[0]"
		       "<td width=12%%>已到款[1]"
		       "<td width=12%%>已交易[2]"
		       "<td width=12%%>总计" "<td width=12%%>有效[非-1]");
		i = 0;
		while (tshirts[i].num != 0) {
			printf("<tr><td>样式%d [%s %s]"
			       "<td>%d"
			       "<td>%d"
			       "<td>%d"
			       "<td>%d"
			       "<td>%d"
			       "<td>%d",
			       tshirts[i].num,
			       tshirts[i].size,
			       tshirts[i].color,
			       statistic[i][0],
			       statistic[i][1],
			       statistic[i][2],
			       statistic[i][3],
			       statistic[i][0] + statistic[i][1] +
			       statistic[i][2] + statistic[i][3],
			       statistic[i][1] + statistic[i][2] +
			       statistic[i][3]);
			i++;
		}
		i = 0;
		while (tshirts[i].num != 0) {
			statistic[MAXTYPE][0] += statistic[i][0];
			statistic[MAXTYPE][1] += statistic[i][1];
			statistic[MAXTYPE][2] += statistic[i][2];
			statistic[MAXTYPE][3] += statistic[i][3];
			i++;
		}
		printf("<tr><td>总计<td>%d<td>%d<td>%d<td>%d<td>%d<td>%d",
		       statistic[MAXTYPE][0],
		       statistic[MAXTYPE][1],
		       statistic[MAXTYPE][2],
		       statistic[MAXTYPE][3],
		       statistic[MAXTYPE][0] + statistic[MAXTYPE][1] +
		       statistic[MAXTYPE][2] + statistic[MAXTYPE][3],
		       statistic[MAXTYPE][1] + statistic[MAXTYPE][2] +
		       statistic[MAXTYPE][3]);
		printf("</table><br><br>");
		break;
	}
	close(fd);
}

void
printitemhead()
{
	printf("<table width=100%%>");
	printf("<tr><td>流水号<td>订购ID<td>订单时间<td>状态<td>操作</tr>");
}

void
printitemline(struct orderitem *item)
{
	printf("<tr><td>%04d<td>%s<td>%s<td>", item->seq_num, item->userid,
	       Ctime(item->time));
	printstatus(item);
	printf("<td>");
	printitemoper(item);
	printf("</tr>");
}

void
printitemend()
{
	printf("</table><br><br>");
}

void
printitemoper(struct orderitem *item)
{
	int admin = 0;
	int status = item->status;
	int seq = item->seq_num;

	if (isadmin())
		admin = 1;

	printf("<a href=tshirt?action=query&seq=%d>查看详情</a>　", seq);

	if (status == -1)
		printf("<a href=tshirt?action=update&seq=%d&status=0>恢复</a>",
		       seq);
	else if (status == 0) {
		printf
		    ("<a href=tshirt?action=update&seq=%d&status=-1>删除</a>　",
		     seq);
		if (item->type == 0 && admin)
			printf
			    ("<a href=tshirt?action=update&seq=%d&status=1>收到货款</a>",
			     seq);
		else if (admin)
			printf
			    ("<a href=tshirt?action=update&seq=%d&status=2>交易成功</a>",
			     seq);
	} else if (status == 1 && admin) {
		printf
		    ("<a href=tshirt?action=update&seq=%d&status=0>取消收款</a>　",
		     seq);
		printf
		    ("<a href=tshirt?action=update&seq=%d&status=2>寄出站衫</a>",
		     seq);
	} else if (status == 2 && admin) {
		if (item->type == 0)
			printf
			    ("<a href=tshirt?action=update&seq=%d&status=1>取消寄出</a>",
			     seq);
		else
			printf
			    ("<a href=tshirt?action=update&seq=%d&status=0>取消交易</a>",
			     seq);
	}

}

void
printoffsetnav(int admintype, int offset)
{
	printf("<div align=right>");
	if (offset != 0)
		printf("<a href='tshirt?admin=%d&offset=%d'>上一页</a>　",
		       admintype, offset - 1);
	else
		printf("上一页　");
	printf("<a href='tshirt?admin=%d&offset=%d'>下一页</a><br><br>",
	       admintype, offset + 1);
}

void
printitembystatus(int fd, struct orderitem *item, int status, int offset,
		  int admintype)
{
	int retv = 0;
	int pos = 0;
	printoffsetnav(admintype, offset);
	printitemhead();
	while (read(fd, item, itemsize) == itemsize) {
		if (item->status == status) {
			if (pos >= PAGESIZE * offset
			    && pos < PAGESIZE * (offset + 1)) {
				printitemline(item);
				retv = 1;
			}
			pos++;
		}
	}
	if (!retv)
		printf("<tr><td colspan=5>没有找到符合条件的订单</tr>");
	printitemend();
	printoffsetnav(status, offset);
}

//初始化个人数据
static int
initmcData(char *filepath)
{
	int fd;
	mcUserInfo ui;
	void *ptr = NULL;
	size_t filesize = 0;

	filesize = sizeof (mcUserInfo);
	bzero(&ui, filesize);
	ui.version = MONEYCENTER_VERSION;
	ptr = &ui;
	ui.con = random() % 10;

	if ((fd = open(filepath, O_CREAT | O_EXCL | O_WRONLY, 0660)) == -1)
		return -1;
	write(fd, ptr, filesize);
	close(fd);
	return 0;
}

void
dobonus(struct orderitem *item)
{
	char buf[512];
	struct userdata data;
	mcUserInfo *mcuInfo;

	sprintf(buf, MY_BBS_HOME "/home/%c/%s/mc.save",
		mytoupper(item->userid[0]), item->userid);
	if (!file_exist(buf))
		initmcData(buf);	//文件不存在，初始化
	if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
		return;

	if (item->bonus[0] != 0 && item->bonus[0] < 100) {
		mcuInfo->credit += 200000 * item->bonus[0];	//暂定存款加20万/件
	}

	if (item->bonus[1] != 0 && item->bonus[0] < 100) {
		mcuInfo->con += 2 * item->bonus[1];	//暂定根骨加2/件
	}

	if (item->bonus[2] != 0 && item->bonus[0] < 100) {
		loaduserdata(item->userid, &data);
		data.extraexp += 20 * item->bonus[2];	//暂定精华值加20/件
		saveuserdata(item->userid, &data);
	}

	unloadData(mcuInfo, sizeof (mcUserInfo));
	//...依此类推，最多到bonus[9] != 0，目前只到2

	return;
}
