#ifndef _MC_H_
#define _MC_H_
#include "bbs.h"
#include "bbstelnet.h"
#include "ythtbbs.h"

#define MAX_MAP_ROW	18
#define MAX_MAP_COL	36

#define NODE_LEFT	0x1
#define NODE_RIGHT	0x2
#define NODE_UP		0x4
#define NODE_DOWN	0x8
#define NODE_QUEUED	0x10

#define MAP_RESTART	0x1
#define MAP_LOCKED	0x2

#define TASK_DONE	0x1

typedef struct {
	char title[16], name[16];
        char info[64];
	int lvl:8, role:4, sex:4, aggres:4, nouse1:12;
	char cou, agi, luc, sta, wis, nouse2[3];
	int exp;
	short hp, max_hp;
	short mp, max_mp;
	unsigned int performs;
	unsigned int skills;
	short weapon_id, armor_id;
} NPC;

typedef struct {
	int id;
} McItem;

typedef struct {
	unsigned short flags, exit;
	int npc_num:4, box_num:4, player_num:8, item_num:16;
	NPC npc[8];
	McItem item[16];
	char player[8]; // idx referenced to map
	pid_t controller;
	int nouse2;
} MapPoint, *pMapPoint;
	
typedef struct {
	time_t c_time;
	int stage:4, id:4, cur:8, prev:8, next:8;
	int lvl:8, row:8, col:8, nouse1:8;
	unsigned int flags;
	MapPoint map[18][32];
	int prev_x:8, prev_y:8, next_x:8, next_y:8;
	int trans_x:8, trans_y:8, nouse2:16;
	//mcUserInfo *players;
} McMap, *pMcMap;

extern MC_Env *mcEnv;
extern mcUserInfo *myInfo;
extern McMap *maps;

void moneycenter_welcome(void);
void moneycenter_byebye(void);
void money_show_stat(char *position);
void nomoney_show_stat(char *position);
int money_bank(void);
int money_lottery(void);
int money_shop(void);
int money_robber(void);
int money_gamble(void);
int money_stock(void);
int money_admin();
int money_cop();
int money_beggar();
int money_task();
int check_allow_in();
int money_hall();
int getOkUser(char *msg, char *uident, int line, int col);
int userInputValue(int line, int col, char *act, char *name, int inf, int sup);
void showAt(int line, int col, char *str, int wait);
void whoTakeCharge(int pos, char *boss);
int ismaster(char *uid);
int randomevent(void);
void update_health(void);
void check_top(void);
int after_tax(int income);
void banID(void);
void forcetax(void);
void all_user_money(void);
void show_top(void);
int check_chance(int attack, int defence, int weapon, int armor, int effect,
		 int bonus);
int check_health(int need_health, int x, int y, char *str, int wait);
void policeCheck(void);
void EquipShop(int type);
int forceGetMoney(int type);
int RobPeople(int type);
void stealbank();
void print_map(McMap *mcMap, int flag);
int gen_map(McMap *mcMap, double rate);
int walk_map(McMap *mcMap);
int map_valid(McMap *mcMap);
int ghoul();
int kill_dragon();
int check_task(int flag);
int load_maps();
int money_exp();
int kill_npc();
int kill_plyer();
int newSalary();

#endif
