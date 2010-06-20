#include "bbslib.h"

#define FB_TEAMDATA	MY_BBS_HOME "/wwwtmp/fb_team"
#define FB_NEWSDATA	MY_BBS_HOME "/wwwtmp/fb_news"
#define FB_GRAPHDATA	MY_BBS_HOME "/wwwtmp/fb_graph"
#define FB_ADMINDATA	MY_BBS_HOME "/wwwtmp/fb_admin.dat"

#define FB_NAME		"百事球王争霸赛"

#define FBFLAG_DEL	0x1	//删除
#define FBFLAG_HIDE	0x2	//隐藏
#define FBFLAG_DISABLE	0x4	//禁止添加子节点

#define FB_GHSIZE	sizeof(struct graphheader)
#define FB_GSIZE	sizeof(struct graph)
#define FB_TSIZE	sizeof(struct team)

struct team {
	char name[30];		//队名
	char univ[30];		//学校
	char member[10][12];	//成员，第一个为队长
	int win;		//胜
	int netwinballs;	//净胜球
	unsigned int status;	//状态
	char initialposstr[10];	//初始位置
	char posstr[10];	//在比赛图中的位置
	char unused[26];
};

struct graph {
	char name[80];		//名字
	char posstr[10];		//用字符串表示位置，类似于sec的表示法
	int team;		//当前这个位置的队伍，属于胜出者
	int team1;
	int team2;
	unsigned int status;
	unsigned int score1;
	unsigned int score2;
	char unused[25];
};

struct graphheader {
	struct graph *main;
	struct graphheader *parent;
	struct graphheader *firstsub;
	struct graphheader *next;
	unsigned int status;
	int nsub;
	int team;
	int team1;
	int team2;
	int score1;
	int score2;
} graphroot;

void *graphmf = NULL;
size_t graphsize;
void *teammf = NULL;
size_t teamsize;
char sec = 0;


struct graphheader * bbsfb_graph_create(struct graphheader *parent, 
		struct graph *main);
int bbsfb_graph_init(void);
int bbsfb_graph_uninit(void);
int bbsfb_graph_addroot(char secstr, char *name);
int bbsfb_graph_add(char *parent, char *name);
struct graphheader *bbsfb_graph_search(char *posstr);
int bbsfb_graph_save(void);
int bbsfb_graph_rename(char *posstr);
int bbsfb_graph_free(struct graphheader *rootptr);
int bbsfb_graph_del(char *posstr);
int bbsfb_graph_flag(struct graphheader *rootptr, unsigned int flag);
int bbsfb_graph_show(FILE *fp, char *posstr, int admin);
int bbsfb_graph_getdottype(char *posstr);
int bbsfb_graph_team_sync(void);

struct team *bbsfb_team_getteam(int num);
char *bbsfb_team_getteamname(int num);
int bbsfb_team_add(struct team *save, int num);
int bbsfb_team_del(int num);
int bbsfb_team_init(void);
int bbsfb_team_uninit(void);

int bbsfb_uninit_all(void);
int bbsfb_init_all(void);
void bbsfb_alert(char *pos);
int bbsfb_team_import(char *content, int start);
int bbsfb_report_add(void);

int bbsfb_graph_init() {
	int fd, i, j, pos;
	char *posstr, graphfile[STRLEN];
	struct graphheader *ptr;
	struct graph *tmpptr;

	if (sec) {
		sprintf(graphfile, FB_GRAPHDATA "%c.dat", sec);
		if (!file_exist(graphfile)) {
			bbsfb_uninit_all();
			http_fatal("Cannot access database.(03)(%s)", 
					graphfile);
		}
	} else {
		strcpy(graphfile, FB_GRAPHDATA ".dat");
	}
	if (!file_exist(graphfile) || !file_size(graphfile)) {
		bbsfb_graph_addroot(0, NULL);
		redirect("bbsfb?A=9");
	}
	graphsize = file_size(graphfile);
	bzero(&graphroot, FB_GHSIZE);
	if ((fd = open(graphfile, O_RDWR | O_CREAT, 0600)) < 0) {
		http_fatal("Cannot access database.(01)");
	}
	flock(fd, LOCK_EX);
	graphmf = mmap(0, graphsize, PROT_READ, MAP_SHARED, fd, 0);
	graphroot.main = (struct graph *) graphmf;
	flock(fd, LOCK_UN);
	close(fd);

	for (i = 0; i < graphsize / FB_GSIZE; i++) {
		ptr = &graphroot;
		tmpptr = (struct graph *) (graphmf + FB_GSIZE * i);
		posstr = tmpptr->posstr;
		j = 0;
		while (posstr[j]) {
			if (NULL == ptr->firstsub) {
				ptr->firstsub = (struct graphheader *)
					malloc(FB_GHSIZE);
				if (NULL == ptr->firstsub) {
					bbsfb_uninit_all();
					http_fatal("Memory lack.(01)");
				}
				bzero(ptr->firstsub, FB_GHSIZE);
				ptr->firstsub->parent = ptr;
			}
			ptr = ptr->firstsub;
			pos = posstr[j] - 'A';
			while (pos) {
				if (NULL == ptr->next) {
					ptr->next = (struct graphheader*)
						malloc(FB_GHSIZE);
					if (NULL == ptr->next) {
						bbsfb_uninit_all();
						http_fatal("Memory lack.(02)");
					}
					bzero(ptr->next, FB_GHSIZE);
					ptr->next->parent = ptr->parent;
				}
				ptr = ptr->next;
				pos--;
			}
			j++;
		}
		ptr->main = tmpptr;
		ptr->status = tmpptr->status;
		ptr->team = tmpptr->team;
		ptr->team1 = tmpptr->team1;
		ptr->team2 = tmpptr->team2;
		ptr->score1 = tmpptr->score1;
		ptr->score2 = tmpptr->score2;
		if (ptr->parent && !(tmpptr->status & FBFLAG_DEL)) {
			ptr->parent->nsub++;
		}
	}
	return 1;
}

int bbsfb_graph_uninit() {
	bbsfb_graph_free(&graphroot);
	bzero(&graphroot, FB_GHSIZE);
	return munmap(graphmf, graphsize);
}

int bbsfb_graph_addroot(char secstr, char *name) {
	struct graph tmp;
	char graphfile[STRLEN];
	int fd;

	bzero(&tmp, FB_GSIZE);
	if (!name || !strlen(name))
		strcpy(tmp.name, "Root");
	else
		strncpy(tmp.name, name, sizeof(tmp.name));
	if (secstr)
		sprintf(graphfile, FB_GRAPHDATA "%c.dat", secstr);
	else
		strcpy(graphfile, FB_GRAPHDATA ".dat");
	unlink(graphfile);
	if ((fd = open(graphfile, O_RDWR | O_CREAT, 0600)) < 0) {
		bbsfb_uninit_all();
		http_fatal("Add root graph failed.(01)");
	}
	flock(fd, LOCK_EX);
	write(fd, &tmp, FB_GSIZE);
	flock(fd, LOCK_UN);
	close(fd);
	return 1;
}

int bbsfb_graph_add(char *parent, char *name) {
	struct graph tmp;
	struct graphheader *ptr;
	char graphfile[STRLEN];
	int fd, i;
	
	if (parent && strlen(parent) >= 9) {	//最多10层
		bbsfb_uninit_all();
		http_fatal("Add new graph failed.(01)");
	}
	ptr = bbsfb_graph_search(parent);
	if (NULL == ptr) {
		bbsfb_uninit_all();
		http_fatal("Add new graph failed.(02)");
	}
	if (ptr->status & FBFLAG_DEL) {
		bbsfb_uninit_all();
		http_fatal("Add new graph failed.(03)");
	}
	if (NULL == ptr->firstsub)
		i = 0;
	else if (NULL == ptr->firstsub->next)
		i = 1;
	else {
		ptr = ptr->firstsub;
		i = 2;
		while (ptr->next->next) {
			ptr = ptr->next;
			i++;
		}
	}
	if (i > 25) {
		bbsfb_uninit_all();
		http_fatal("Add new graph failed.(02)");
	}
	bzero(&tmp, sizeof(tmp));
	strncpy(tmp.name, name, sizeof(tmp.name));
	snprintf(tmp.posstr, sizeof(tmp.posstr), "%s%c", parent, 'A' + i);
	if (sec)
		sprintf(graphfile, FB_GRAPHDATA "%c.dat", sec);
	else {
		strcpy(graphfile, FB_GRAPHDATA ".dat");
		tmp.status |= FBFLAG_DISABLE;
		bbsfb_graph_addroot('A' + i, name);
	}
	if ((fd = open(graphfile, O_RDWR | O_CREAT, 0600)) < 0) {
		bbsfb_uninit_all();
		http_fatal("Add new graph failed.(03)");
	}
	flock(fd, LOCK_EX);
	lseek(fd, 0, SEEK_END);
	write(fd, &tmp, FB_GSIZE);
	flock(fd, LOCK_UN);
	close(fd);
	return 1;
}

struct graphheader *bbsfb_graph_search(char *posstr) {
	int pos, i = 0;
	struct graphheader *ptr;
	if (NULL == posstr)
		return &graphroot;
	ptr = &graphroot;
	while (posstr[i]) {
		ptr = ptr->firstsub;
		if (NULL == ptr)
			return NULL;
		pos = posstr[i] - 'A';
		while (pos) {
			ptr = ptr->next;
			if (NULL == ptr)
				return NULL;
			pos--;
		}
		i++;
	}
	return ptr;
}

int bbsfb_graph_save() {
	int fd;
	struct graphheader *ptr, *rootptr;
	struct graph save;
	char graphfile[STRLEN];

	if (sec)
		sprintf(graphfile, FB_GRAPHDATA "%c.dat", sec);
	else
		strcpy(graphfile, FB_GRAPHDATA ".dat");
	unlink(graphfile);
	if ((fd = open(graphfile, O_RDWR | O_CREAT, 0600)) < 0) {
		bbsfb_uninit_all();
		http_fatal("Cannot save graph.(01)");
	}
	flock(fd, LOCK_EX);
	lseek(fd, 0, SEEK_SET);
	rootptr = &graphroot;
	ptr = rootptr;
	while (1) {
		memcpy(&save, ptr->main, FB_GSIZE);
		save.status = ptr->status;
		save.team = ptr->team;
		save.team1 = ptr->team1;
		save.team2 = ptr->team2;
		save.score1 = ptr->score1;
		save.score2 = ptr->score2;
		write(fd, &save, FB_GSIZE);
		if (ptr->firstsub)
			ptr = ptr->firstsub;
		else if (ptr->next)
			ptr = ptr->next;
		else if (ptr->parent && ptr->parent->next)
			ptr = ptr->parent->next;
		else {
			while (ptr->parent && !ptr->parent->next)
				ptr = ptr->parent;
			if (!ptr->parent)
				break;
			else	
				ptr = ptr->parent->next;
		}
	}
	flock(fd, LOCK_UN);
	close(fd);

	return 1;
}

int bbsfb_graph_rename(char *posstr) {
	char name[80], graphfile[STRLEN];
	int fd, fsize, i;
	struct graph save;

	if (!bbsfb_graph_search(posstr))
		return 0;
	strncpy(name, getparm("newname"), sizeof(name));
	if (sec)
		sprintf(graphfile, FB_GRAPHDATA "%c.dat", sec);
	else
		strcpy(graphfile, FB_GRAPHDATA ".dat");
	if ((fd = open(graphfile, O_RDWR | O_CREAT, 0600)) < 0) {
		bbsfb_uninit_all();
		http_fatal("Cannot rename graph.(01)");
	}
	fsize = file_size(graphfile);
	flock(fd, LOCK_EX);
	for (i = 0; i < fsize / FB_GSIZE; i++) {
		lseek(fd, i * FB_GSIZE, SEEK_SET);
		read(fd, &save, FB_GSIZE);
		if (!strcmp(save.posstr, posstr)) {
			strncpy(save.name, name, sizeof(save.name));
			lseek(fd, i * FB_GSIZE, SEEK_SET);
			write(fd, &save, FB_GSIZE);
			break;
		}
	}
	flock(fd, LOCK_UN);
	close(fd);
	return 1;
}

int bbsfb_graph_free(struct graphheader *rootptr) {
	struct graphheader *tmpptr;

	if (!rootptr)
		return 1;

	if (!rootptr->firstsub && !rootptr->next) {
		tmpptr = rootptr->parent;
		if (!tmpptr) {
			return 1;
		} else if (tmpptr->firstsub == rootptr)  {
			free(tmpptr->firstsub);
			tmpptr->firstsub = NULL;
			return bbsfb_graph_free(tmpptr);
		} else {
			if (tmpptr->firstsub->next == rootptr) {
				free(tmpptr->firstsub->next);
				tmpptr->firstsub->next = NULL;
				return bbsfb_graph_free(tmpptr->firstsub);
			}
			tmpptr = tmpptr->firstsub;
			while (tmpptr->next->next != rootptr)
				tmpptr = tmpptr->next;
			free (tmpptr->next->next);
			tmpptr->next->next = NULL;
			return bbsfb_graph_free(tmpptr->next);
		}
		
	} else if (rootptr->next) {
		tmpptr = rootptr;
		while (tmpptr->next)
			tmpptr = tmpptr->next;
		return bbsfb_graph_free(tmpptr);
	} else {
		tmpptr = rootptr->firstsub;
		return bbsfb_graph_free(tmpptr);
	}
	return 1;
}

int bbsfb_graph_del(char *posstr) {
	struct graphheader *ptr;

	ptr = bbsfb_graph_search(posstr);
	bbsfb_graph_flag(ptr, FBFLAG_DEL);
	bbsfb_graph_save();
	bbsfb_graph_team_sync();
	return 1;
}

int bbsfb_graph_flag(struct graphheader *rootptr, unsigned int flag) {
	struct graphheader *ptr;

	if (!rootptr || rootptr == &graphroot)
		return 1;
	
	ptr = rootptr;
	while (1) {
		ptr->status |= flag;
		if (ptr->firstsub)
			ptr = ptr->firstsub;
		else if (ptr->next) {
			if (ptr == rootptr)
				break;
			ptr = ptr->next;
		} else {
			if (ptr == rootptr)
				break;
			while (ptr->parent != rootptr
					&& !ptr->parent->next)
				ptr = ptr->parent;
			if (ptr->parent == rootptr)
				break;
			else
				ptr = ptr->parent->next;
		}
	}
	return 1;
}

int bbsfb_graph_show(FILE *fp, char *posstr, int admin) { //to be updated.
	int dottype;
	struct graphheader *ptr, *rootptr;
	
	rootptr = bbsfb_graph_search(posstr);
	if (NULL == rootptr) {
		bbsfb_uninit_all();
		fprintf(fp, "Cannot show graph.(01)");
	}
	ptr = rootptr;
	fprintf(fp, "<script>\ngi('%c', '%s');\n", 
			sec , posstr ? posstr : "");
	while (1) {
		dottype = bbsfb_graph_getdottype(ptr->main->posstr);
		if (!(ptr->status & FBFLAG_DEL))
			fprintf(fp, "gs(%d, '%s', '%s', %d, %d, %d, %d, "
					"'%s', %d, '%s', %d, '%s', %d, %d);\n", 
					admin, ptr->main->name, 
					ptr->main->posstr, ptr->status, 
					dottype, ptr->nsub, ptr->team, 
					bbsfb_team_getteamname(ptr->team),
					ptr->team1, 
					bbsfb_team_getteamname(ptr->team1),
					ptr->team2, 
					bbsfb_team_getteamname(
						ptr->team2), 
					ptr->score1, ptr->score2);
		if (ptr->firstsub)
			ptr = ptr->firstsub;
		else if (ptr->next && ptr != rootptr)
			ptr = ptr->next;
		else if (ptr == rootptr)
			break;
		else {
			while (ptr->parent != rootptr 
					&& !ptr->parent->next)
				ptr = ptr->parent;
			if (ptr->parent == rootptr)
				break;
			else 
				ptr = ptr->parent->next;
		}
	}
	fprintf(fp, "</script>\n");
	return 1;
}

int bbsfb_graph_getdottype(char *posstr) {
	struct graphheader *ptr, *tmpptr;
	int hasprev, hasnext, nteam;
	ptr = bbsfb_graph_search(posstr);

	if (NULL == ptr)
		return 0;

	if (NULL == ptr->parent)
		return 0;

	if (ptr == ptr->parent->firstsub)
		hasprev = 0;
	else {
		hasprev = 0;
		tmpptr = ptr->parent->firstsub;
		while (tmpptr != ptr) {
			if (!(tmpptr->status & FBFLAG_DEL)) {
				hasprev = 1;
				break;
			}
			tmpptr = tmpptr->next;
		}
	}
	hasnext = 0;
	tmpptr = ptr->next;
	while (tmpptr) {
		if (!(tmpptr->status & FBFLAG_DEL)) {
			hasnext = 1;
			break;
		}
		tmpptr = tmpptr->next;
	}
	nteam = ptr->parent->main->team1 || ptr->parent->main->team2;
	if (!hasprev) {
		if (hasnext || nteam)
			return 1;
		else
			return 0;
	} else {
		if (hasnext || nteam)
			return 2;
		else 
			return 3;
	}	
}

struct team *bbsfb_team_getteam(int num) {
	if (num > teamsize / FB_TSIZE || num < 1)
		return NULL;
	return (struct team *)(teammf + (num - 1) * FB_TSIZE);
}

char *bbsfb_team_getteamname(int num) {
	struct team *ptr;

	ptr = bbsfb_team_getteam(num);

	if (NULL == ptr)
		return "";
	return ptr->name;
}

int bbsfb_team_add(struct team *save, int num) {
	int fd, pos;
	char teamfile[STRLEN];
	
	if (!sec)
		strcpy(teamfile, FB_TEAMDATA ".dat");
	else
		sprintf(teamfile, FB_TEAMDATA "%c.dat", sec);

	if ((fd = open(teamfile, O_RDWR | O_CREAT, 0600)) < 0) {
		bbsfb_uninit_all();
		http_fatal("Cannot add new team.(01)");
	}
	flock(fd, LOCK_EX);
	if (num > 0 && num <= teamsize / FB_TSIZE) {
		lseek(fd, FB_TSIZE * (num - 1), SEEK_SET);
		pos = 0;
	} else {
		lseek(fd, 0, SEEK_END);
		pos = teamsize / FB_TSIZE + 1;
	}
	write(fd, save, FB_TSIZE);
	flock(fd, LOCK_UN);
	close(fd);
	return pos;
}

int bbsfb_team_del(int num) {
	int fd;
	char teamfile[STRLEN];
	struct team save;
	struct graphheader *ptr;

	if (num < 1 || num > teamsize / FB_TSIZE)
		return 0;
	if (!sec)
		strcpy(teamfile, FB_TEAMDATA ".dat");
	else
		sprintf(teamfile, FB_TEAMDATA "%c.dat", sec);
	if ((fd = open(teamfile, O_RDWR | O_CREAT, 0600)) < 0) {
		bbsfb_uninit_all();
		http_fatal("Cannot del team.(01)");
	}
	flock(fd, LOCK_EX);
	lseek(fd, FB_TSIZE * (num - 1), SEEK_SET);
	read(fd, &save, FB_TSIZE);
	save.status |= FBFLAG_DEL;
	ptr = bbsfb_graph_search(save.posstr);
	lseek(fd, FB_TSIZE * (num - 1), SEEK_SET);
	write(fd, &save, FB_TSIZE);
	flock(fd, LOCK_UN);
	close(fd);
	bbsfb_team_uninit();
	bbsfb_team_init();
	bbsfb_graph_team_sync();
	return 1;
}

int bbsfb_team_init() {
	int fd;
	char teamfile[STRLEN];
	
	teammf = NULL;

	if (!sec)
		strcpy(teamfile, FB_TEAMDATA ".dat");
	else
		sprintf(teamfile, FB_TEAMDATA "%c.dat", sec);
	if (!file_exist(teamfile))
		return 0;
	teamsize = file_size(teamfile);
	if (!teamsize)
		return 0;
	if ((fd = open(teamfile, O_RDWR | O_CREAT, 0600)) < 0) {
		bbsfb_uninit_all();
		http_fatal("Cannot access database.(02)");
	}
	flock(fd, LOCK_EX);
	teammf = mmap(0, teamsize, PROT_READ, MAP_SHARED, fd, 0);
	flock(fd, LOCK_UN);
	close(fd);
	return 1;
}

int bbsfb_team_uninit() {
	teamsize = 0;
	return munmap(teammf, teamsize);
}

int bbsfb_uninit_all() {
	bbsfb_graph_uninit();
	bbsfb_team_uninit();
	graphsize = 0;
	teamsize = 0;
	return 1;
}


int bbsfb_init_all() {
	bbsfb_graph_init();
	bbsfb_team_init();
	return 1;
}

int bbsfb_graph_team_sync() {
	int fd, i;
	struct team *tptr, save;
	struct graphheader *ptr;
	char teamfile[STRLEN];

	ptr = &graphroot;
	if (!sec)
		strcpy(teamfile, FB_TEAMDATA ".dat");
	else
		sprintf(teamfile, FB_TEAMDATA "%c.dat", sec);
	if ((fd = open(teamfile, O_RDWR | O_CREAT, 0600)) < 0) {
		bbsfb_uninit_all();
		http_fatal("Cannot sync data.(01)");
	}
	flock(fd, LOCK_EX);
	while (1) {	//sync from graph to team.
		if (ptr->status & FBFLAG_DEL) {
			tptr = bbsfb_team_getteam(ptr->team1);
			if (tptr && (tptr->status & FBFLAG_DEL)) {
				lseek(fd, (ptr->team1 - 1) * FB_TSIZE, 
						SEEK_SET);
				read(fd, &save, FB_TSIZE);
				save.status |= FBFLAG_DEL;
				lseek(fd, (ptr->team1 - 1) * FB_TSIZE, 
						SEEK_SET);
				write(fd, &save, FB_TSIZE);
			}
			tptr = bbsfb_team_getteam(ptr->team2);
			if (tptr && (tptr->status & FBFLAG_DEL)) {
				lseek(fd, (ptr->team2 - 1) * FB_TSIZE, 
						SEEK_SET);
				read(fd, &save, FB_TSIZE);
				save.status |= FBFLAG_DEL;
				lseek(fd, (ptr->team2 - 1) * FB_TSIZE, 
						SEEK_SET);
				write(fd, &save, FB_TSIZE);
			}

		}
		if (ptr->firstsub)
			ptr = ptr->firstsub;
		else if (ptr->next)
			ptr = ptr->next;
		else {
			while (ptr->parent && !ptr->parent->next)
				ptr = ptr->parent;
			if (!ptr->parent)
				break;
			else
				ptr = ptr->parent->next;
		}
	}
	flock(fd, LOCK_UN);
	close(fd);
	
	bbsfb_team_uninit();
	bbsfb_team_init();
	for (i = 1; i <= teamsize / FB_TSIZE; i++) {
		tptr = bbsfb_team_getteam(i);
		if (tptr->status & FBFLAG_DEL) {
			ptr = bbsfb_graph_search(tptr->posstr);
			if (ptr->team1 == i)
				ptr->team1 = 0;
			else if (ptr->team2 == i)
				ptr->team2 = 0;
			if (ptr->team == i)
				ptr->team = 0;
			while (ptr->parent && ptr->parent->team == i) {
				ptr->parent->team = 0;
				ptr = ptr->parent;
			}	
		}
	}
	bbsfb_graph_save();
	
	return 1;
}

struct graphheader * bbsfb_graph_create(struct graphheader *parent, 
		struct graph *main) {
	struct graphheader *ptr, *tmpptr;
	int i = 0;
	if (!parent || !parent->main)
		return NULL;
	ptr = (struct graphheader *) malloc (FB_GHSIZE);
	if (NULL == ptr) {
		bbsfb_uninit_all();
		http_fatal("Memory Lack.(01)");
		return 0;
	}
	bzero(ptr, FB_GHSIZE);
	ptr->parent = parent;
	if (NULL == main) {
		ptr->main = (struct graph *) malloc (FB_GSIZE);
		if (NULL == ptr->main) {
			bbsfb_uninit_all();
			http_fatal("Memory Lack.(02)");
		}
		bzero(ptr->main, FB_GSIZE);
	} else {
		ptr->main = main;
		ptr->team1 = main->team1;
		ptr->team2 = main->team2;
		ptr->team = main->team;
	}
	if (NULL == parent->firstsub)
		parent->firstsub = ptr;
	else {
		i = 1;
		tmpptr = parent->firstsub;
		while (tmpptr->next) {
			i++;
			tmpptr = tmpptr->next;
		}
		tmpptr->next = ptr;
	}
	if (NULL == main)
		snprintf(ptr->main->posstr, sizeof(ptr->main->posstr), 
			"%s%c", parent->main->posstr, 'A' + i);
	ptr->parent->nsub++;
	return ptr;
}

int bbsfb_graph_auto(int result, int total, char *posstr) {
	struct graphheader *ptr, *tmpptr;
	int maximum, level, i, size = 0, fd, num;
	struct team save;
	char teamfile[STRLEN];

	if (!sec)
		strcpy(teamfile, FB_TEAMDATA ".dat");
	else
		sprintf(teamfile, FB_TEAMDATA "%c.dat", sec);
	ptr = bbsfb_graph_search(posstr);
	if (!ptr) {
		bbsfb_uninit_all();
		http_fatal("Cannot auto design maps.(01)");
		return 0;
	}
	if (!sec || ptr->status & FBFLAG_DEL) {
		bbsfb_uninit_all();
		http_fatal("Cannot auto design maps.(06)");
	}
	if (result < 0 || total < 0 || result > 25) {
		bbsfb_uninit_all();
		http_fatal("Cannot auto design maps.(02)");
		return 0;
	}
	level = 8 - strlen(posstr);
	if (level <= 0) {
		bbsfb_uninit_all();
		http_fatal("Cannot auto design maps.(03)");
		return 0;
	}
	maximum = result * 2 << level;
	if (total > maximum || result * 2 > total || result < 2) {
		bbsfb_uninit_all();
		http_fatal("Cannot auto design maps.(04)");
		return 0;
	}
	if (ptr->firstsub || ptr->team1 || ptr->team2 || ptr->team) {
		bbsfb_uninit_all();
		http_fatal("Cannot auto design maps.(05)");
		return 0;
	}
	for (i = 0; i < result; i++) {
		bbsfb_graph_create(ptr, NULL);
		size++;
	}

	size *= 2;
	while (1) {
		tmpptr = ptr;
		while(1){
			while (tmpptr->firstsub)
				tmpptr = tmpptr->firstsub;
			bbsfb_graph_create(tmpptr, NULL);
			size += 1;
			if (size == total)
				break;
			bbsfb_graph_create(tmpptr, NULL);
			size += 1;
			if (size == total)
				break;
			if (tmpptr->next)
				tmpptr = tmpptr->next;
			else {
				while (tmpptr->parent != ptr 
						&& !tmpptr->parent->next)
					tmpptr = tmpptr->parent;
				if (tmpptr->parent == ptr)
					break;
				else
					tmpptr = tmpptr->parent->next;
			}
		}
		if (size == total)
			break;
	}
	if ((fd = open(teamfile, O_RDWR | O_CREAT, 0600)) < 0) {
		bbsfb_uninit_all();
		http_fatal("Cannot auto design maps.(06)");
		return 0;
	}
	flock(fd, LOCK_EX);
	size = file_size(teamfile) / FB_TSIZE;
	num = 1;
	lseek(fd, 0, SEEK_END);
	bzero(&save, FB_TSIZE);
	tmpptr = ptr;
	while (1) {
		if (tmpptr->nsub == 1) {
			snprintf(save.name, sizeof(save.name), "%d", num);
			strncpy(save.posstr, tmpptr->main->posstr, 
					sizeof(save.posstr));
			write(fd, &save, FB_TSIZE);
			tmpptr->team1 = size + num++;
		} else if (tmpptr->nsub == 0) {
			snprintf(save.name, sizeof(save.name), "%d", num);
			strncpy(save.posstr, tmpptr->main->posstr,
					sizeof(save.posstr));
			write(fd, &save, FB_TSIZE);
			tmpptr->team1 = size + num++;
			snprintf(save.name, sizeof(save.name), "%d", num);
			strncpy(save.posstr, tmpptr->main->posstr,
					sizeof(save.posstr));
			write(fd, &save, FB_TSIZE);
			tmpptr->team2 = size + num++;
		}
		if (tmpptr->firstsub)
			tmpptr = tmpptr->firstsub;
		else if (tmpptr->next)
			tmpptr = tmpptr->next;
		else if (tmpptr == ptr)
			break;
		else {
			while (tmpptr->parent != ptr && !tmpptr->parent->next)
				tmpptr = tmpptr->parent;
			if (tmpptr->parent == ptr)
				break;
			else
				tmpptr = tmpptr->parent->next;
		}
		
	}
	flock(fd, LOCK_UN);
	close(fd);
	bbsfb_graph_save();
	return 1;
}

int bbsfb_team_import(char *content, int start) {
	int current, fd, i, count = 1;
	struct team save;
	char *ptr, *str, *line, teamfile[STRLEN];

	if (start < 1 || start > teamsize / FB_TSIZE) {
		bbsfb_uninit_all();
		http_fatal("Cannot import teams.(01)");
	}

	if (!sec)
		strcpy(teamfile, FB_TEAMDATA ".dat");
	else
		sprintf(teamfile, FB_TEAMDATA "%c.dat", sec);

	current = start;
	if ((fd = open(teamfile, O_RDWR | O_CREAT, 0600)) < 0) {
		bbsfb_uninit_all();
		http_fatal("Cannot import teams.(02)");
	}
	flock(fd, LOCK_EX);
	line = content;
	while (line && current <= teamsize / FB_TSIZE) {
		lseek(fd, (current - 1) * FB_TSIZE, SEEK_SET);
		read(fd, &save, FB_TSIZE);
		str = line;
		if ((ptr = strchr(line, '\n'))) {
			*ptr = 0;
			line = ++ptr;
		} else
			line = 0;
		if (!(ptr = strchr(str, '\t')))
			break;
		*ptr++ = 0;
		snprintf(save.name, sizeof(save.name), "%s", str);
		str = ptr;
		if (!(ptr = strchr(ptr, '\t')))
			break;
		*ptr++ = 0;
		strncpy(save.univ, str, sizeof(save.univ));
		str = ptr;
		for (i = 0; i < 12 && ptr; i++) {
			if ((ptr = strchr(str, '\t')))
				*ptr++ = 0;
			strncpy(save.member[i], str, sizeof(save.member[i]));
			str = ptr;
		}
		lseek(fd, (current - 1) * FB_TSIZE, SEEK_SET);
		write(fd, &save, FB_TSIZE);
		current++;
		count++;
	}
	flock(fd, LOCK_UN);
	close(fd);
	return 1;
}

int bbsfb_report_add() {
	int team1, team2, score1, score2, i, force, total;
	struct graphheader *ptr;
	char buf[STRLEN], posstr[10];

	total = atoi(getparm("total"));

	for (i = 0; i < total; i++) {
		sprintf(buf, "posstr_%d", i);
		strncpy(posstr, getparm(buf), sizeof(posstr));
		sprintf(buf, "force_%d", i);
		force = atoi(getparm(buf));
		sprintf(buf, "team1_%d", i);
		team1 = atoi(getparm(buf));
		sprintf(buf, "team2_%d", i);
		team2 = atoi(getparm(buf));
		sprintf(buf, "score1_%d", i);
		score1 = atoi(getparm(buf));
		sprintf(buf, "score2_%d", i);
		score2 = atoi(getparm(buf));
		ptr = bbsfb_graph_search(posstr);
		if (NULL == ptr)
			continue;
		if (team1 < 1 || team2 < 1 || team1 >= teamsize / FB_TSIZE ||
				team2 >= teamsize / FB_TSIZE || 
				score1 < 0 || score2 < 0 ||
				(score1 == score2 && !force))
			continue;
		if (force == team1) {
			ptr->team = team1;
			ptr->score1 = 0;
			ptr->score2 = 0;
		} else if (force == team2) {
			ptr->team = team2;;
			ptr->score1 = 0;
			ptr->score2 = 0;
		} else {
			if (score1 > score2)
				ptr->team = team1;
			else
				ptr->team = team2;
			ptr->score1 = score1;
			ptr->score2 = score2;
		}
	}
	bbsfb_graph_save();
	return 1;
}

int bbsfb_report_caculate() {
	struct graphheader *ptr, *tmpptr;
	int i = 0, j;
	char buf[STRLEN];

	ptr = &graphroot;

	printf("<script>\nri('%c', '%s');\n", sec, graphroot.main->name);
	while (ptr) {
		if (!ptr->team) {
			j = 0;
			sprintf(buf, "rf(%d, '%s'", i, ptr->main->posstr);
			if (ptr->team1) {
				j++;
				sprintf(buf, "%s, %d, '%s'", buf, ptr->team1, 
					bbsfb_team_getteamname(ptr->team1));
			}
			if (ptr->team2) {
				j++;
				sprintf(buf, "%s, %d, '%s'", buf, ptr->team2, 
					bbsfb_team_getteamname(ptr->team2));
			}
			tmpptr = ptr->firstsub;
			while (j < 2 && tmpptr) {
				if (tmpptr->status & FBFLAG_DEL) {
					tmpptr = tmpptr->next;
					continue;
				}
				if (tmpptr->team) {
					j++;
					sprintf(buf, "%s, %d, '%s'", buf, 
						tmpptr->team, 
						bbsfb_team_getteamname(
							tmpptr->team));
				}
				tmpptr = tmpptr->next;
			}
			sprintf(buf, "%s);\n", buf);
			if (j == 2) {
				printf("%s", buf);
				i++;
			}
		}
		if (ptr->firstsub)
			ptr = ptr->firstsub;
		else if (ptr->next)
			ptr = ptr->next;
		else {
			while (ptr->parent && !ptr->parent->next)
				ptr = ptr->parent;
			if (!ptr->parent)
				break;
			else
				ptr = ptr->parent->next;
		}
	}
	printf("re(%d);\n</script>\n", i);
	return 1;
}
