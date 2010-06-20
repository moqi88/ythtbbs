/*
调用格式： scanmark 天数，一般加入cron每天调用 scanmark 1就行
作用：查找 24小时之前 argv[1] 天以内mark的文章(一个版面最多扫描5000篇)
除去一些例外情况，给被标记的文章作者加钱，加经验值
base on happybird's markday.c 2005/1/5
develeped by koyu       2005/6/29
*/

#include "bbs.h"
#include "sysrecord.c"
#include "ythtbbs.h"

#define MARKED_FILE MY_BBS_HOME"/0Announce/bbslist/marked_article"      //每日mark文章统计
#define BASECASH        500     //基本稿费
#define EXTRACASH       100     //每EXTRAWORD字追加稿费
#define MAXCASH         8888	//单篇最多稿费
#define STARTWORD       20     	//文章字数底线
#define EXTRAWORD       100     //字数计数单位
#define MAXIDNUM	512	//一次扫描数组记录人数上限

char id[MAXIDNUM][14];
int percash[MAXIDNUM];
short perarticle[MAXIDNUM];
int perexp[MAXIDNUM];
int n=0;
int sum=0;
struct boardmem bp;
//#define DEBUG
#define DETAIL

struct bbsinfo bbsinfo; //全局变量
MC_Env *mcEnv;

//记录到bbslist文件
int append2file(char *file,char *buf)
{
        FILE *fp;
        fp= fopen(file,"a");
        if(!fp) return 0;
        fprintf(fp,"%s",buf);
        fclose(fp);
        return 1;
}

//给用户加钱
//稿费
int give_money(struct boardheader *bh,struct fileheader *fhdr,int cash)
{
        mcUserInfo *mcuInfo;
        char buf[512];
        int i, baseexp;
        time_t now_t;
        struct userdata data;
#if 1
	if(bp.score>=10000 )
		baseexp = 10;
	else if(bp.score >= 3000)
		baseexp = 5;
	else
		baseexp = 1;
#else
	baseexp = bp.score / 1000  + 1;
	baseexp = baseexp >10?10:baseexp;
#endif
	if(strstr(bh->filename, "Digest") && strstr(bh->title, "区文摘" ))	//文摘版面额外奖励
		baseexp = 20;
		
        //打开个人的moneycenter文件
        sprintf(buf,MY_BBS_HOME"/home/%c/%s/mc.save",mytoupper(fhdr->owner[0]),fhdr->owner);
        now_t = time(NULL);
        if(!file_exist(buf))
            initData(1, buf);    //文件不存在，初始化
	if((mcuInfo = loadData(buf, sizeof (mcUserInfo))) == (void*)-1)
            return -1;
#ifndef DEBUG
	//if(mcEnv->Treasury >= cash){	//国库还能给钱
	        mcuInfo->cash += cash;  //给钱。
	//      mcEnv->Treasury -= cash;
		sum += cash;
	//}
	//else
	//	cash = 0;		//国库没钱了。。。
#endif
        unloadData(mcuInfo, sizeof(mcUserInfo));

        loaduserdata(fhdr->owner, &data);		//给经验值
#ifndef DEBUG
       	data.extraexp += baseexp;        
#endif
        saveuserdata(fhdr->owner, &data);
        
        for (i=0;i<n;i++) {     //查找作者是否已在前面的扫描中有奖励记录
            if (!strcmp(id[i],fhdr->owner)) {
                perarticle[i]+=1;
                percash[i]+=cash;
                perexp[i]+= baseexp;
                goto APP;
            }
        }
        strcpy(id[n],fhdr->owner);      //创建新的记录
        perarticle[n]=1;
        percash[n]=cash;
        perexp[n]=baseexp;
        n=n+1;
        if(n >= MAXIDNUM)
        	postfile(MARKED_FILE, "deliver", "Code_Discuss", "[自动扫描]数组大小不够了！");
        goto APP;
        //记录到文件
        //版名           版主            文章标题                   作者       糊涂币  精华值
        //========== ========= ================================= =========== ========= ========
APP:    sprintf(buf,"%-10.10s \033[1;32m%-10.10s\033[m %-30.30s \033[1;33m%-10.10s\033[m \033[1;31m%-8d %-7d\033[m\n",bh->filename,bh->bm[0],fhdr->title,fhdr->owner,cash, baseexp);                 
        append2file(MARKED_FILE,buf);
        
        return 1;
}

int
denyUser(char *author)
{
        char buf[256], *ptr;
        FILE *fp;
        int retv = 0;
        if (!strcmp(author, "Anonymous"))       //匿名用户不计
                return 1;

        fp = fopen("deny_users", "rt");         //被封禁用户不计
        if (!fp)
                return 0;
        while (fgets(buf, sizeof (buf), fp)) {
                ptr = strtok(buf, " \t\n\r");
                if (!ptr)
                        continue;
                if (!strcasecmp(ptr, author)) {
                        retv = 1;
                        break;
                }
        }
        fclose(fp);
        return retv;
}

int
searchLastMark(struct boardheader *bh, int days)
{
        char filename[256];
        struct fileheader fhdr;
        int fd, total, n, old = 0;
        time_t now;
        int cash;

        sprintf(filename, MY_BBS_HOME "/boards/%s/.DIR", bh->filename);

        if ((total = file_size(filename) / sizeof (fhdr)) <= 0)
                return 0;

        time(&now);
        if ((fd = open(filename, O_RDONLY, 0)) == -1) {
                return 0;
        }
		
        for (n = total - 1; n >= 0 && total - n < 5000; n--) {  //最多只扫描5000篇
                if (lseek(fd, n * sizeof (fhdr), SEEK_SET) < 0)
                        break;
                if (read(fd, &fhdr, sizeof (fhdr)) != sizeof (fhdr))
                        break;
                if (now - fhdr.filetime < 3600 * 24 * 1)        //24小时之内文章跳过
                    continue;
                if (now - fhdr.filetime > 3600 * 24 *(days + 1)) { //没有n天内的新文章
                        old++;
                        if (old > 4)    //考虑到有时会有从删除区等恢复的旧文章
                                break;
                        continue;
                }
                if (!strcmp(fhdr.owner, "deliver")      //系统发文不计
                    && !(fhdr.accessed & FH_DIGEST)     
                    && !(fhdr.accessed & FH_MARKED))    //非标记
                        continue;
                if (fhdr.owner[0] == '-'                //这个是什么？
                    || !strcmp(fhdr.owner, "post") 
                    || !strcmp(fhdr.owner, "deliver") 
                    || strstr(fhdr.title, "[警告]")
                //    || strstr(fhdr.title, "[公告]")
                //    || strstr(fhdr.title, "[维护]")
                //    || strstr(fhdr.title, "[任命]")     //公告性质
                    || strstr(fhdr.title, "[合集]")     //版主合集
                    || (fhdr.accessed & FH_DANGEROUS))  //危险标记文章
                        continue;
                if (bytenum(fhdr.sizebyte) <= STARTWORD)        //获得奖励最低文章字数限制
                        continue;
                if (denyUser(fh2owner(&fhdr)))          //作者匿名或者被封禁不给
                        continue;
                if ((fhdr.accessed & FH_DIGEST) || (fhdr.accessed & FH_MARKED)) {       //是M或者G的文章
                        cash = BASECASH + EXTRACASH*(bytenum(fhdr.sizebyte)-STARTWORD)/EXTRAWORD;  //计算稿费
                        cash = cash > MAXCASH ? MAXCASH : cash;         //最大稿费限制
                        give_money(bh,&fhdr,cash);                      //给作者发钱，发经验值
                        
#ifdef DEBUG
                        printf("%s %s size:%d cash:%d %d %d %d %d %d\n",fhdr.owner,fhdr.title,bytenum(fhdr.sizebyte),cash,BASECASH,STARTWORD,EXTRAWORD,EXTRACASH,MAXCASH);
#endif
                }
        }
        close(fd);
        return 1;
}

int main( int argc, char *argv[] )
{

        int b_fd, bnum = -1;
        int i,days;
        char title[80];
        char content[256];
        struct boardheader bh;
        char filepath[256];
        char day[40], buf[256];
        time_t now_t;
        int size;
        FILE *fp;
        size = sizeof (bh);
        
        if( argc != 2 ){
            printf("Wrong numbers of paramaters!\n");
            return -1;
        }
	if (initbbsinfo(&bbsinfo) < 0){
		printf("Init bbs info failed.\n");
		return -1;
	}
	sprintf(filepath, "%s", DIR_MC "mc.env");
        if(!file_exist(filepath)){
		printf("no file:%s\n",filepath);
		return -1;
	}
        if((mcEnv = loadData(filepath, sizeof(MC_Env))) == (void*)-1)
                return -1;

        //扫描 24小时之前 days 天之内的文章
        days = atoi(argv[1])>=1?atoi(argv[1]):1;

	if ((b_fd = open(MY_BBS_HOME"/"BOARDS, O_RDONLY)) == -1){
			perror("open BOARDS");
			return -1;
	}

	//初始化统计列表文件
	fp = fopen(MARKED_FILE,"w");
	if(!fp){
		perror("open MARKED_FILE.");
		exit(-1);
	}
	fprintf(fp, "\n%s%d%s%s", "\
          \033[1;37m===========  \033[32m  最近",days,"天内各版标记文章及作者稿费 \033[37m   ============ \n\n\
版名         版主           文章标题                作者      "MONEY_NAME" 精华值","\n\
========== ========= ============================= ========== ====== ======\n\033[0m\
");

	fclose(fp);

	for (bnum = 0; read(b_fd, &bh, size) == size && bnum < MAXBOARD; bnum++) {
		if (!bh.filename[0])
			continue;
		if (!(bh.level & PERM_POSTMASK) && !(bh.level & PERM_NOZAP)	//跳过读写限制的版面
		    && bh.level != 0)
			continue;
		if ( (bh.flag & CLUB_FLAG) && !(strstr(bh.filename, "Digest" ) && strstr(bh.title, "区文摘" )) ) 		//跳过俱乐部版面，但不包括文摘版面
			continue;
		bp = (bbsinfo.bcacheshm)->bcache[bnum];
		searchLastMark(&bh, days);	//开始扫描
	}
	close(b_fd);
	unloadData(mcEnv, sizeof (MC_Env));
	for (i=0;i<n;i++) {
		sprintf(title,"%s系统给您的稿费",MY_BBS_NAME);
		sprintf(content,"您最近%d日内共有%d篇文章被标记。\n系统发给您稿费%d %s,以及%d 额外经验值(精华值)奖励。\n\n被标记文章和稿费发放情况，详见bbslists版。 \n稿费HTB的使用及大富翁游戏，详见millionaire版。\n\n--\n",days,perarticle[i],percash[i],MONEY_NAME,perexp[i]);
		#ifdef DETAIL
		printf("%3d id:%s,article:%d,cash:%d,exp:%d\n",i,id[i],perarticle[i],percash[i],perexp[i]);
		#endif
		#ifndef DEBUG
		if (!inoverride("稿费通知", id[i], "rejects"))
			system_mail_buf(content, strlen(content), id[i], 
					title, "稿费通知");
		#endif
	}
	now_t=time(NULL);
	sprintf(day,"[%.10s] 系统发放稿费一览",ctime(&now_t));
	sprintf(buf,"\n今日累计发放稿费 \033[1;32m%d\033[m %s。\n\n",sum, MONEY_NAME);
	append2file(MARKED_FILE,buf);
#ifndef DEBUG
	postfile(MARKED_FILE, "deliver", "bbslists", day);
#else
	strcat(day,"(DEBUG)");
	postfile(MARKED_FILE, "deliver", "Code_Discuss", day);
#endif
	return 0;
}
