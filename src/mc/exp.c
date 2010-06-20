#include "mc.h"
/*
typedef struct {
	NPC npc[8];
*/

#define MAX_FIGHT_USER_NUM 16

int continue_fight(NPC *fighters, int na, int nb) {
	if(na > 0 && nb > 0) //双方都还有人
		return 1;
	return 0;
}

void compute_attack_chance_array(NPC *npc, int na, int nb, int *acarray) {
	int i, sum, all;
	
	all = na + nb;
	for(i = 0, sum = 0; i < all; i++) {
		acarray[i] = sum;
		sum += npc[i].cou;
	}
	acarray[MAX_FIGHT_USER_NUM] = sum;
}

			
int race_for_chance(NPC *npc, int na, int nb, int *acarray) {
	int i, r, all;

	r = random() % acarray[MAX_FIGHT_USER_NUM];
	all = na + nb;
	for(i = 0; i < all-1; i++) {
		if(r >= acarray[i] && r <= acarray[i+1])
			return i;
	}
	return all-1;
}


#define PERFORM_SET	0x1
#define COUNTER_SET	0x2
#define HEAL_SET	0x4

void
decide_attackt_mode(NPC *npc, int me, int na, int nb, int *attack_mode, int *attack_obj, char *npc_set_array)
{
	int r;

	r = random() % 11;
	if(r == 0) {
		*attack_mode = 1;
		*attack_obj = -1;	//假定特技对全体
	} else if(r < 8) {
		*attack_mode = 0;
		if(me < na)	// npc
			*attack_obj = (random() % nb) + na;
		else	//player
			*attack_obj = (random() % na);
	} else {
		*attack_mode = 2;
		*attack_obj = -1;
	}
}

void move_fighters(NPC *npc, int *num) {
	int i, j, new;

        new = 0;
        for(i = 0; i < *num; i++) {
                if(npc[0].hp <= 0) { // dead
                        for(j=i+1; j < *num; j++) {
                                if(npc[j].hp > 0) {
                                        memcpy(&npc[i], &npc[j], sizeof(npc[i]));
                                        npc[j].hp = -1; //invalid
                                        break;
                                }
                        }
                        if(j == *num)
                                break; // no more alive
                }
                new++;
        }
	*num = new;
}


void update_fighters(NPC *npc, int *na, int *nb)
{

	move_fighters(&npc[0], na);
	move_fighters(&npc[*na], nb);
	//...
	
}

void init_npc(NPC *p, char *name, int cou)
{
	bzero(p, sizeof(NPC));
	strcpy(p->title, "勇士");
	strcpy(p->name, name);
	strcpy(p->info, "丐帮弟子");
	p->lvl = 14;
	p->role = 1;
	p->aggres = 1;
	p->cou = cou;
	p->agi = p->luc = p->sta = p->wis = 16;
	p->exp = 370;
	p->hp =  p->max_hp = 100;
	p->mp = p->max_mp = 80;
	p->weapon_id = p->armor_id = 1;
}

void compute_attack(NPC *npc, int me, int na, int nb, int attack_mode, int attack_obj) {
	int i, r, enemy, num;

	r = (random() % 10 + 3) * (attack_mode + 1);

	if(attack_obj == -1) {
		enemy = (me < na) ? na : 0;
		num = (me < na) ? nb : na;
		for(i = enemy; i < num; i++)
			npc[i].hp -= r;
	} else {
		npc[attack_obj].hp -= r;
	}
}
		
int kill_npc() {
	NPC fighters[MAX_FIGHT_USER_NUM];
	int na, nb, me, row, pk;
	int attack_chance, attack_mode, attack_obj;
	int attack_chance_array[MAX_FIGHT_USER_NUM + 1];
	char npc_set_array[MAX_FIGHT_USER_NUM];

	pk = 0;
	na = 2;	// npc
	nb = 1; // player

	bzero(fighters, sizeof(fighters));

	init_npc(&fighters[0], "wuqq", 20);
	init_npc(&fighters[1], "they", 20);
	init_npc(&fighters[2], currentuser->userid, 100);

	row = 6;
	while(continue_fight(fighters, na, nb)) {
		attack_chance = (random() % (na + nb)) + 1;
		compute_attack_chance_array(fighters, na, nb, attack_chance_array);

		while(attack_chance-- > 0 && continue_fight(fighters, na, nb)) {
			me = race_for_chance(fighters, na, nb, attack_chance_array);
			decide_attackt_mode(fighters, me, na, nb, &attack_mode, &attack_obj, npc_set_array);
			if(attack_mode == 0) {	//normal attack
				compute_attack(fighters, me, na, nb, attack_mode, attack_obj);
				move(row++, 0);
				prints("%s进攻! %d\n", fighters[me].name, fighters[me].hp);
			} else if(attack_mode == 1) { //perform
				compute_attack(fighters, me, na, nb, attack_mode, attack_obj);
				move(row++, 0);
				prints("%s特技! %d\n", fighters[me].name, fighters[me].hp);
			}
			update_fighters(fighters, &na, &nb);
			refresh();
			sleep(1);
		}
		if(row >= 20) {
			clear();
			row = 6;
		}
	}
	move(row++, 0);
	if(na > 0) 
		prints("NPC胜利!");
	else
		prints("玩家胜利!");
	pressanykey();
        return 0;
}


int kill_player() {
        return 0;
}



int money_exp() {
	return 0;
}


