/* board.c */
#ifndef __BOARD_H
#define __BOARD_H

#define BOARDAUX ".BOARDAUX"

#define BMNUM 16
struct boardheader {		/* This structure is used to hold data in */
	char filename[24];	/* the BOARDS files */
	char title[24];
	int clubnum;
	unsigned level;
	char flag;
	char secnumber1;
	char secnumber2;
	char type[5];
	char bm[BMNUM][IDLEN + 1];
	int hiretime[BMNUM];
	int board_ctime;
	int board_mtime;
	char sec1[4];
	char sec2[4];
	unsigned char limitchar;	//upper limit/100
	char flag2;
	char nouse[2];
	char unused[156];
};

struct boardmem {		/* used for caching files and boards */
	struct boardheader header;
	int lastpost;
	int total;
	short inboard;
	short bmonline;
	short bmcloak;
	int stocknum;
	int score;
	time_t wwwindext;
	int unused[8];
	char unused1[2];
	unsigned char unused3:4, wwwlink:1, wwwbkn:1, wwwicon:1, wwwindex:1;
	char ban;
};

struct hottopic {
	int thread;
	int num;		//讨论人数
	char title[60];
};

struct lastmark {
	int thread;
	char marked;
	char nouse[3];
	char title[60];
	char authors[60];	//可能有多作者
};

struct relateboard {
	char filename[24];
	char title[24];
};

#define MAXHOTTOPIC 5
#define MAXLASTMARK 9
#define MAXTOPFILE 5
#define MAXRELATE 6

struct boardaux {		// used to save lastmark/hottopic/fix top
	int nhottopic;		// # of hot topics, www only.
	struct hottopic hottopic[MAXHOTTOPIC];	// hot topics
	int nlastmark;		// # of last marked files, www only.
	struct lastmark lastmark[MAXLASTMARK];	// hot last mark files
	int ntopfile;		// # of fix top files, both www and telnet
	struct fileheader topfile[MAXTOPFILE];	// fix top files
	int nousea[5];
	int nrelate;		// relating boards, www only.
	struct relateboard relate[MAXRELATE];	// relating boards
	char intro[200];	// board introduction, www only.
	char nouse[1440];	//total 4k
};

struct boardmanager {		/* record in user directionary */
	char board[24];
	char bmpos;
	char unused;
	short bid;
};

struct myparam1 {		/* just use to pass a param to fillmboard() */
	struct userec user;
	int fd;
	short bid;
};

struct bm_eva {	// for BM evaluation weekly
	char userid[IDLEN + 1];
	char week, total_week;
	char ave_score;
	short weight;
	char leave;
	char last_pass;
	int nouse;
};

struct board_bmstat {
        char boardname[24];
        struct bm_eva bm[BMNUM];
        int boardscore;
        char sec;
        char unused[7];
};

// 定制版面的代码, 取自fb2000.dhs.org.     --ecnegrevid
// 允许自定义收藏夹目录. --enhanced by cojie  2005.6

// 收藏夹目录的基本思路  --by Fishingsnow 2006.10
// 每个用户的收藏夹放在家目录的.favorboard里面(原先的线性目录放在.goodbrd)里面
// 主要结构体是struct FolderSet，
// 每个目录是struct SubFolder folder[FOLDER_NUM]数组中的一个元素
// 其中folder[0]是根目录
// 而FolderSet中的boards[FAVOR_BRD_NUM + 1][20]是一个线性表
// 保存了所有被收藏的版面
// 当一个版面被收藏到某个目录folder[i]的时候
// 将会在folder[i]中的bidx数组最后增加一个数字
// 这个数字表示这个版面在FolderSet结构中boards[]数组中对应的位置
// 亦即，每个子目录本身并不保存版面的名称，而只有在boards[]数组中的位置
// 同样地，boards[]数组也不反应收藏夹的拓扑结构

#define FAVOR_BRD_NUM	99	//最大收藏版面数
#define FOLDER_BRD_NUM	32	//每个目录最大收藏版面数
#define FOLDER_NUM 	16	//最大目录数,其中0为根目录
#define FAVOR_UPGRADE	1	//控制是否升级旧收藏夹
#define FAVOR_DEFAULT	"BBSHelp"
/** 添加一路特色默认收藏夹 
* 预留7个位置。需要时把注释打开
**/
#define FAVOR_YILU_DEFAULT_1	"triangle"
#define FAVOR_YILU_DEFAULT_2	"joke"
#define FAVOR_YILU_DEFAULT_3	"Movie"
#define FAVOR_YILU_DEFAULT_4	"music"
#define FAVOR_YILU_DEFAULT_5	"familylife"
#define FAVOR_YILU_DEFAULT_6	"job"
#define FAVOR_YILU_DEFAULT_7	"stock"
#define FAVOR_YILU_DEFAULT_8	"AdvancedEdu"
#define FAVOR_YILU_DEFAULT_9	"Game"
#define FAVOR_YILU_DEFAULT_10	"China_news"
#define FAVOR_YILU_DEFAULT_11	"Test"
#define FAVOR_YILU_DEFAULT_12	"sysop"
//#define FAVOR_YILU_DEFAULT_13	"triangle"
//#define FAVOR_YILU_DEFAULT_14	"joke"
//#define FAVOR_YILU_DEFAULT_15	"Movie"
//#define FAVOR_YILU_DEFAULT_16	"music"
//#define FAVOR_YILU_DEFAULT_17	"familylife"
//#define FAVOR_YILU_DEFAULT_18	"job"
//#define FAVOR_YILU_DEFAULT_19	"stock"
#define FAVOR_FILE	".favor"
#define FAVOR_OLDFILE1	".goodbrd"
#define FAVOR_OLDFILE2  ".favorboard"

struct SubFolder {
	char name[20];
	char desc[32];
	short bidx[FOLDER_BRD_NUM];	//版名索引,指向FolderSet->boards[]
	int boardNum;			//该目录的版面数
	int unused[2];
};	// 128 bytes

struct FolderSet {
	int folderNum;		//子目录数(不包括编号为0的根目录)
	int boardNum;		//总版面数
	struct SubFolder folder[FOLDER_NUM];
	char boards[FAVOR_BRD_NUM + 1][20];	//版名列表, 0保留不用
	int unused[10];
};	// 4096 bytes


char *bm2str(char *buf, struct boardheader *bh);
char *sbm2str(char *buf, struct boardheader *bh);
int chk_BM(struct userec *, struct boardheader *bh, int isbig);
int chk_BM_id(char *, struct boardheader *);
int dosetbmstatus(struct boardmem *bcache, char *userid, int online, int visible);
int bmfilesync(struct userec *);
int fillmboard(struct boardheader *bh, struct myparam1 *param);
int getlastpost(char *board, int *lastpost, int *total);
struct boardaux *getboardaux(int bnum);
int addtopfile(int bnum, struct fileheader *fh);
int deltopfile(int bnum, int num);
int updateintro(int bnum, char *filename);

struct FolderSet * _loadFavorFolder(char *userid);
int _unloadFavorFolder(struct FolderSet *Favor);
int _addFavorBoard(char *bname, struct FolderSet *Favor, int currFolder);
int _addFavorFolder(char *foldername, char *desc, struct FolderSet *Favor);
int _delFavorBoard(char *bname, struct FolderSet *Favor, 
		int currFolder, int keepLastBoard);
int _delFavorFolder(struct FolderSet *Favor, int currFolder);
int _moveFavorFolder(struct FolderSet *Favor, int currFolder, int tagFolder);
int _editFavorFolder(struct FolderSet *Favor, int currFolder, 
		char *name, char *desc);
int _inCurrFolder(char *bname, struct FolderSet *Favor, int currFolder);
int _inFavorBoards(char *bname, struct FolderSet *Favor);
int _fixFavorFolder(struct FolderSet *Favor);

#endif
