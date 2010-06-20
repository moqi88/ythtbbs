#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ght_hash_table.h>
#include <signal.h>
#include "bbs.h"
#include "queue.h"


#define EACH_MSG_LEN	64
#define MAX_MSG_NUM	2048
#define FRIENDS_TMPDIR	MY_BBS_HOME "/bbstmpfs/tmp/friends"

/* the initial distance of two nodes. it must be a bit larger than NO_PATH so that
 * any pair of two nodes are not connected untill they have someting to be done
 * to reduce the diatance value.
 */ 
#define INITIAL_PATH	2000000000

/* if the distance between two nodes is larger than NO_PATH, it is considered
 * that the two nodes are not connected, that is "no path".
 */
#define NO_PATH		(INITIAL_PATH - 300)

/* this is a "long long" type(64it) value which is much larger than even the sum of
 * all the nodes' distance values.
 */
#define INFINITY	((long long)2000000000 * (long long)2000000000)


#define PUN_FLAG_ESTRANGE	0x1	//if set, es "to; else es "from"

typedef struct {
	char userid[IDLEN+1], flags;
	short div;
	pLinkList from, to;
} UserNode;

typedef struct {
	UserNode *ptr;
	int distance;
} Relationship;


ght_hash_table_t *fidx = NULL;
int lockfd = -1, msqid = -1, sync_flag = 0;

//------------------------------msg queue-----------------------------//
static int initFriendsMSQ()
{
	int msqid;
	struct msqid_ds mds;
	
	msqid = msgget(getBBSKey(FRIENDS_MSQ), IPC_CREAT | 0664);
	if (msqid < 0)
		return -1;
	msgctl(msqid, IPC_STAT, &mds);
	mds.msg_qbytes = MAX_MSG_NUM * EACH_MSG_LEN;
	msgctl(msqid, IPC_SET, &mds);
	return msqid;
}

static char *
rcvlog(int msqid, int nowait)
{
	static char buf[EACH_MSG_LEN];
	struct mymsgbuf *msgp = (struct mymsgbuf *) buf;
	int retv;
	
	retv = msgrcv(msqid, msgp, sizeof (buf) - sizeof (msgp->mtype) - 2, 
			0, (nowait ? IPC_NOWAIT : 0) | MSG_NOERROR);
	while (retv > 0 && msgp->mtext[retv - 1] == 0)
		retv--;
	if (retv <= 0)
		return NULL;
	msgp->mtext[retv] = 0;
	return msgp->mtext;
}

//--------------------------------kernel--------------------------------//
static int cmp_user(void *pr, void *userid) {
	return (strcasecmp(
		(((Relationship *)pr)->ptr)->userid, (char*)userid))
		? 0 : 1;
}

static void free_afriend(UserNode *punode) {
	if(punode == NULL)
		return;
	if(punode->from)
		free_queue(punode->from, 1);
	if(punode->to)
		free_queue(punode->to, 1);
	free(punode);
}

/* init and insert a user into hash table. */
static int img_user(const char *userid) {
	UserNode *punode;
	int retv = -1;
	
	punode = calloc(1, sizeof(UserNode));
	if(punode == NULL)
		return -1;
	strncpy(punode->userid, userid, sizeof(punode->userid));
	punode->userid[sizeof(punode->userid)-1] = '\0';
	punode->from = init_queue();
	punode->to = init_queue();
	if(punode->from == NULL || punode->to == NULL)
		goto END;
	retv = ght_insert(fidx, punode, strlen(punode->userid), punode->userid);
    END:
	if(retv == -1)
		free_afriend(punode);
	return retv;
}

/* get a user from hash table. if not exist, create one by img_user(). */
static UserNode* get_afriend(char *userid) {
	UserNode *punode;
	
	punode = ght_get(fidx, strlen(userid), userid);
	if(punode)
		return punode;
	if(img_user(userid) == -1)
		return NULL;
	punode = ght_get(fidx, strlen(userid), userid);
	return punode;
}


/* dump文件格式:
 * #str_user_A
 * str_user_1[SPACE]int_distance_1['\n']
 * str_user_2[SPACE]int_distance_2['\n']
 * ...
 * #str_user_B
 * ...
*/

/* Because this function is called at the beginning, and if it returns -1 
 * the program will exit soon. So there is no need to free resource here.
 */
static int load_friends() {
	int distance = 0, line = 0;
	FILE *fp;
	char buf[EACH_MSG_LEN*2], curr_user[IDLEN+1], tar_user[IDLEN+1];
	char *ptr;
	UserNode *punode_op = NULL, *punode_ta = NULL;
	Relationship *pr;

	if(!file_exist(MY_BBS_HOME "/friends.dump")) //可运行时dump
		return 0;

	fp = fopen(MY_BBS_HOME "/friends.dump", "r");
	if(fp == NULL)
		return -1;

	curr_user[0] = '\0';
	while(fgets(buf, sizeof(buf)-1, fp)) {
		if(*buf == '\n' || *buf == '\0') //空行，文件结束
			break;		
		if(*buf == '#') { //新的user
			if((ptr = strchr(buf, '\n')) != NULL)
				*ptr = '\0';
			strncpy(curr_user, buf+1, sizeof(curr_user));
			punode_op = get_afriend(curr_user);
			if(punode_op == NULL)
				return -1;
		} else {
			if(punode_op == NULL) //bad file-format
				return -1;
			ptr = strchr(buf, ' ');
			if(ptr == NULL) //bad file-format
				return -1;
			*ptr = '\0';
			strncpy(tar_user, buf, sizeof(tar_user));
			distance = atoi(ptr+1);
			
			punode_ta = get_afriend(tar_user);
			if(punode_ta == NULL)
				return -1;
			
			pr = calloc(1, sizeof(Relationship));
			if(pr == NULL)
				return -1;
			pr->ptr = punode_ta;
			pr->distance = distance;
			if(insert_node(punode_op->to, pr) == -1)
				return -1;
			
			pr = calloc(1, sizeof(Relationship));
			if(pr == NULL)
				return -1;
			pr->ptr = punode_op;
			pr->distance = distance;
			if(insert_node(punode_ta->from, pr) == -1)
				return -1;
		}
		line++;
	}
	fclose(fp);
	if(line > 0)
		copyfile(MY_BBS_HOME "/friends.dump", MY_BBS_HOME "/friends.dump.good");
	
	return 0;
}

static int is_exceptional_user(char *user1, char *user2) {
	int i;
	char *eu[] = {"deliver", "guest", "post", "auto", NULL};

	for(i = 0; eu[i] != NULL; i++) {
		if(!strcasecmp(eu[i], user1) || !strcasecmp(eu[i], user2))
			return 1;
	}
	return 0;
}


/*msg格式:
 * int_type[SPACE]str_op_userid[SPACE]str_target_userid
 * type类型	距离变化	含义
 * 1		-1		re文/查询/
 * 3		-3		blog留言/评论
 * 4		-4		信件/消息
 * 50		-50		友谊测试
 * 10000	-10000		加为好友
 * -10000	10000		加为坏人
*/
static int update_friends(char *msg) {
	int type;
	char *opuid, *tauid, *ptr;
	UserNode *punode_op, *punode_ta;
	Relationship *pr;
	
	ptr = strchr(msg, ' ');
	if(ptr == NULL)
		return -1;
	type = atoi(msg);
	
	msg = ptr+1;
	ptr = strchr(msg, ' ');
	if(ptr == NULL)
		return -1;
	*ptr = '\0';
	opuid = msg;

	msg = ptr+1;
	if(*msg == '\0')
		return -1;
	tauid = msg;

	if(!strcasecmp(opuid, tauid)) // the same user
		return 0;
	if(is_exceptional_user(opuid, tauid)) //exceptional user
		return 0;

	punode_op = get_afriend(opuid);
	if(punode_op == NULL)
		return -1;
	
	punode_ta = get_afriend(tauid);
	if(punode_ta == NULL)
		return -1;
	
	//修改指向距离
	pr = (Relationship *)search_queue(punode_op->to, cmp_user, tauid);
	if(pr == NULL) {
		pr = calloc(1, sizeof(Relationship));
		if(pr == NULL)
			return -1;
		pr->ptr = punode_ta;
		pr->distance = INITIAL_PATH;
		if(insert_node(punode_op->to, pr) == -1)
			return -1;
	}
	pr->distance -= type;
	
	//同时修改反向距离
	pr = (Relationship *)search_queue(punode_ta->from, cmp_user, opuid);
	if(pr == NULL) {
		pr = calloc(1, sizeof(Relationship));
		if(pr == NULL)
			return -1;
		pr->ptr = punode_op;
		pr->distance = INITIAL_PATH;
		if(insert_node(punode_ta->from, pr) == -1)
			return -1;
	}
	pr->distance -= type;
	return 0;
}

static int del_invalid_path(void *p1, void *p2) {
	Relationship *pr;

	pr = p1;
	if(pr->distance >= INITIAL_PATH || pr->distance < 0)
		return 1;
	return 0;
}

static int estrange_user(void *p1, void *p2) {
	((Relationship *)p1)->distance += 3;
	return 0;
}

/* increase the distance value daily which means the relationship betwwen two
 * users are estranged. when the value is >= INITIAL_PATH, the pr will
 * be free() in order to minium memory usage.
 */
static void estrange_by_time() {
	ght_iterator_t iterator;
	void *key;
	UserNode *punode;

	for (punode = ght_first(fidx, &iterator, &key); punode;
			punode = ght_next(fidx, &iterator, &key))
	{
		apply_queue(punode->to, estrange_user, NULL);
		delete_node3(punode->to, del_invalid_path, NULL, 1);
		apply_queue(punode->from, estrange_user, NULL);
		delete_node3(punode->from, del_invalid_path, NULL, 1);
					
	}
	return;
}

static int dump_user(void *pr, void *fp) {
	fprintf((FILE*)fp, "%s %d\n", (((Relationship *)pr)->ptr)->userid,
				((Relationship *)pr)->distance);
	return 0;
}

static int save_friends() {
	ght_iterator_t iterator;
	void *key;
	UserNode *punode;
	FILE *fp;
	char *filename = MY_BBS_HOME "/bbstmpfs/tmp/friends.dump.tmp";

	fp = fopen(filename, "w+");
	if(fp == NULL)
		return -1;
	for (punode = ght_first(fidx, &iterator, &key); punode;
			punode = ght_next(fidx, &iterator, &key))
	{
		if(queue_len(punode->to) > 0) {
			fprintf(fp, "#%s\n", punode->userid);
			apply_queue(punode->to, dump_user, fp);
		}
	}
	fclose(fp);
	crossfs_rename(filename, MY_BBS_HOME "/friends.dump");
	return 0;
}


/* --------------------------------api-----------------------------------*/
typedef struct {
	char userid[IDLEN+1];
	int distance;
} Neighbour;

typedef struct {	
	int max, count;
	int distance;
	Neighbour *list;
} QueryCondt;

static int get_neighbour(void *p1, void *p2) {
	QueryCondt *qc;
	Relationship *pr;
	
	qc = p2;
	pr = p1;
	if(qc->count < qc->max && pr->distance < qc->distance) {
		strcpy(qc->list[qc->count].userid, pr->ptr->userid);
		qc->list[qc->count].distance = pr->distance;
		qc->count++;
	}
	return 0;
}

static int cmp_dist(void *n1, void *n2) {
	return ((Neighbour*)n1)->distance > ((Neighbour*)n2)->distance;
}

/* 查询userid的邻居，最多查找1024个，按照距离升序排列，并最多显示topn个。
 * 由于子进程执行完该函数很快退出，因此就不一一释放内存了。
 */
static void query_neighbour(int flag, char *userid, int topn, int distance) {
	int i;
	UserNode *punode;
	QueryCondt qc;
	FILE *fp;
	char tmpf[256], fpath[256];
	
	if(topn <= 0 || distance <= 0)
		return;
	
	sprintf(tmpf, FRIENDS_TMPDIR "/%s.%d.tmp", userid, flag);
	if((fp = fopen(tmpf, "w+")) == NULL)
		return;

	punode = ght_get(fidx, strlen(userid), userid);
	if(punode == NULL) {
		fprintf(fp, "Query return null result.");
		goto END_WITH_RESULT;
	}
	qc.max = flag ? queue_len(punode->to) : queue_len(punode->from);
	qc.list = malloc(sizeof(Neighbour)*(qc.max+1));
	if(qc.list == NULL)
		goto END_WITH_ERROR;
	qc.count = 0;
	qc.distance = distance;
	if(flag)
		apply_queue(punode->to, get_neighbour, &qc);
	else
		apply_queue(punode->from, get_neighbour, &qc);
	if(qc.count == 0)
		fprintf(fp, "Query return null result.");

	qsort(qc.list, qc.count, sizeof(Neighbour), (void*)cmp_dist);
	for(i = 0; i < qc.count && i < topn; i++) {
		fprintf(fp, "%s%c", qc.list[i].userid, 
			(i % 6 == 5) ? '\n' : '\t');
	}
	
END_WITH_RESULT:
	fclose(fp);
	sprintf(fpath, FRIENDS_TMPDIR "/%s.%d", userid, flag);
	rename(tmpf, fpath);
	return;
END_WITH_ERROR:
	fclose(fp);
	unlink(tmpf);
	return;
}



typedef struct {
	pLinkList pList;
	short div, flag;
	int distance;
} DivParam;

static int add_node(void *p1, void *p2) {	
	Relationship *pr;
	DivParam *dp;
	int retv = -1;

	pr = p1;
	dp = p2;
	switch(dp->flag) {
	case 1: //add "to" list
		if(pr->ptr->div <= 0 && pr->ptr->div != -dp->div &&
		   pr->distance < dp->distance) {
			retv = insert_node(dp->pList, pr->ptr);
			pr->ptr->div = -dp->div;
		}
		break;
	case 0: //add "from" list
		if(pr->ptr->div == -dp->div &&
		   pr->distance < dp->distance) {
			retv = insert_node(dp->pList, pr->ptr);
			pr->ptr->div = dp->div;
		}
		break;
	}
	return retv;
}

static int div_one(UserNode *punode, short div, int distance) {
	UserNode *tmp;
	DivParam dp;
	
	tmp = punode;
	
	dp.pList = init_queue();
	if(dp.pList == NULL)
		return -1;

	dp.div = div;
	dp.distance = distance;
	
	dp.flag = 1; //"to"
	punode->div = -dp.div;
	apply_queue(punode->to, add_node, &dp);
	while(!is_empty(dp.pList)) {
		punode = read_first(dp.pList);
		apply_queue(punode->to, add_node, &dp);
		delete_node(dp.pList, 0, 0);
	}
	
	punode = tmp;
	dp.flag = 0; //"from"
	punode->div = dp.div;
	apply_queue(punode->from, add_node, &dp);
	while(!is_empty(dp.pList)) {
		punode = read_first(dp.pList);
		apply_queue(punode->from, add_node, &dp);
		delete_node(dp.pList, 0, 0);
	}
	free_queue(dp.pList, 1);
	return 0;
}

/* 划分朋友圈。成功返回0, 否则返回-1.
 * 由于该函数对fidx写，并且在错误恢复时强烈依赖于fidx内部顺序。
 * 因此必须避免竞争条件。
 *
 * 朋友圈的定义：
 * A. 若集合中元素大于1, 则至少存在一个元素可访问其它所有元素。
 * B. 若集合中元素大于1，则每一元素至少可被另一元素访问。
 * C. 不同朋友圈无访问关系。
 *
 * 划分结果存储于UserNode结构div中，最大可划分32767个朋友圈。
 * div有效范围[1, 32767]
 */
static int div_friends() {
	short div, retv;
	short *tmp_div;
	int i, j;
	UserNode *punode;
	ght_iterator_t it;
	void *key;

	tmp_div = malloc(sizeof(short) * MAXUSERS);
	if(tmp_div == NULL)
		return -1;
	for(i = 0, punode = ght_first(fidx, &it, &key); punode;
			punode = ght_next(fidx, &it, &key), i++) {
		tmp_div[i] = punode->div;
		punode->div = 0;
	}
	div = 1;
	retv = 0;
	for(punode = ght_first(fidx, &it, &key); punode && div < 32767;
			punode = ght_next(fidx, &it, &key)) {
		if(punode->div <= 0) {
			retv = div_one(punode, div++, NO_PATH);
			if(retv == -1)
				break;
		}
	}
	if(retv == -1) { //recovery
		for(j = 0, punode = ght_first(fidx, &it, &key); punode && j < i;
		    punode = ght_next(fidx, &it, &key), j++) {
			punode->div = tmp_div[j];
		}
	}
	free(tmp_div);
	return -1;
}

static void query_adiv(char *userid) {
	ght_iterator_t it;
	void *key;
	int div, count;
	UserNode *punode;
	FILE *fp;
	char tmpf[256], fpath[256];

	sprintf(tmpf, FRIENDS_TMPDIR "/%s.div.tmp", userid);
	if((fp = fopen(tmpf, "w+")) == NULL)
		return;
	punode = ght_get(fidx, strlen(userid), userid);
	if(punode == NULL) {
		fprintf(fp, "Query return null result.");
		goto END_WITH_RESULT;
	}
	
	div = punode->div;
	count = 0;
	for(punode = ght_first(fidx, &it, &key); punode;
			punode = ght_next(fidx, &it, &key)) {
		if(div == punode->div && div > 0) {
			count++;
			fprintf(fp, "%s%c", punode->userid, 
				(count % 6 == 5) ? '\n' : '\t');
		}
	}
END_WITH_RESULT:
	fclose(fp);
	sprintf(fpath, FRIENDS_TMPDIR "/%s.div", userid);
	rename(tmpf, fpath);
	return;
}



typedef struct {
	UserNode *pun;
	short midx, pidx;
} PathNode;

typedef struct {
	PathNode *me, *prev;
	long long len;
} Path;

typedef struct {
	Path *dist;
	short curr, nu;
	int distance;
	long long len;
	PathNode *pn;
} PathParam;


static int init_dist(void *p1, void *p2) {
	PathNode *pn;
	PathParam *pp;
	Relationship *pr;

	pn = p1; //queue node
	pp = p2; //path parameter
	pp->dist[pp->curr].me = pn;
	pn->midx = pp->curr;
	pp->dist[pp->curr].me->midx = pp->curr;
	pr = search_queue(pp->pn->pun->to, cmp_user, pn->pun->userid);
	if(pr) { //直接邻居
		if(pr->distance < pp->distance) { //有可达路径
			pp->dist[pp->curr].len = pr->distance;
			pp->dist[pp->curr].prev = pp->pn; //dist[0]
			pn->pidx = 0;
			pp->dist[pp->curr].me->pidx = 0;
			pp->curr++;
			return 1;
		}
	}
	pp->dist[pp->curr].len = INFINITY; //其它设为无穷值
	pp->dist[pp->curr].prev = NULL;
	pn->pidx = -1;
	pp->dist[pp->curr].me->pidx = -1;
	pp->curr++;
	return 0;
}

static int find_min(void *p1, void *p2) {
	PathNode *pn;
	PathParam *pp;

	pn = p1; //queue node
	pp = p2; //path parameter
	if(pp->dist[pn->midx].len < pp->len) {
		pp->pn = pn;
		pp->len = pp->dist[pn->midx].len;
		return 1;
	}
	return 0;
}	
	
static int renew_dist(void *p1, void *p2) {
	PathNode *pn;
	PathParam *pp;
	Relationship *pr;
	int i, min;

	pn = p1; //queue node
	pp = p2; //path parameter
	pr = search_queue(pp->pn->pun->to, cmp_user, pn->pun->userid);
	if(pr == NULL)
		return 0;
	if(pr->distance >= pp->distance)
		return 0;

	i = pn->midx;
	min = pp->pn->midx;
	if(pp->dist[i].len > pp->dist[min].len + pr->distance) {
		pp->dist[i].len = pp->dist[min].len + pr->distance;
		pp->dist[i].prev = pp->dist[min].me;
		return 1;
	}
	return 0;
}

/* 按照Dijkstra算法寻找从from到to的最短路径。对fidx只读。
 * 由于子进程执行完该函数很快结束，因此就不一一释放内存了。
 */
static void query_path(char *from, char *to, int distance)
{
	ght_iterator_t it;
	void *key;
	UserNode *pun_from, *pun_to, *tmp;
	PathNode *pn;
	pLinkList S;
	Path *dist, *ptr;
	int nu, i;
	PathParam pp;
	FILE *fp;
	char tmpf[256], fpath[256];

	pun_from = ght_get(fidx, strlen(from), from);
	if(pun_from == NULL)
		return;
	pun_to = ght_get(fidx, strlen(to), to);
	if(to == NULL)
		return;
	sprintf(tmpf, FRIENDS_TMPDIR "/%s.%s.path.tmp", from ,to);
	fp = fopen(tmpf, "w+");
	if(fp == NULL)
		return;
	//节省起见， 如果不在一个圈子或者还没有划分，就不找了
	if(pun_from->div != pun_to->div || pun_from->div == 0) {
		fprintf(fp, "Query return null result.");
		goto END_WITH_RESULT;
	}
	
	if((S = init_queue()) == NULL) //待查找集
		goto END_WITH_ERROR;
	nu = 1;
	for(tmp = ght_first(fidx, &it, &key); tmp;
			tmp = ght_next(fidx, &it, &key)) {
		if(tmp->div == pun_from->div && tmp != pun_from) {
			pn = malloc(sizeof(PathNode));
			if(pn == NULL)
				goto END_WITH_ERROR;
			pn->pun = tmp;
			pn->midx = pn->pidx = 0;
			if(insert_node(S, pn) == -1)
				goto END_WITH_ERROR;
			nu++;
		}
	}
	if(nu > 1024) {
		fprintf(fp, "朋友圈过于庞大，查询被拒绝。\n");
		goto END_WITH_RESULT;
	}

	dist = malloc(sizeof(Path)*nu);
	if(dist == NULL)
		goto END_WITH_ERROR;

	pn = malloc(sizeof(PathNode));
	if(pn == NULL)
		goto END_WITH_ERROR;
	pn->pun = pun_from;	//put "from" in dist[0]
	pn->midx = pn->pidx = 0;
	dist[0].me = pn;
	dist[0].prev = pn;
	dist[0].len = 0;	//myself
	pp.dist = dist;
	pp.curr = 1;	//curr=0 is "from"
	pp.distance = distance; //limited distance
	pp.nu = nu;	//num friends
	pp.pn = pn;
	apply_queue(S, init_dist, &pp); //init dist array
	
	while(!is_empty(S)) {	//S是未计算完毕的集合
		pp.pn = NULL;
		pp.len = INFINITY;
		apply_queue(S, find_min, &pp); //找出距离值最小的，放在pp.pn
		if(pp.pn == NULL)
			goto END_WITH_ERROR;
		apply_queue(S, renew_dist, &pp); //用pp.pn更新距离值
		delete_node2(S, (void*)pp.pn, 0); //只从队列中删除，pp.pn不释放
	}
	
	//find "to"
	for(i = 0; i < nu; i++) {
		if(dist[i].me->pun == pun_to)
			break;
	}
	if(i == nu)
		goto END_WITH_ERROR;
	ptr = &dist[i];
	for(i = 0; ptr->me && ptr->me->pun != pun_from && i < nu; i++) {
		fprintf(fp, "%s <--%c", ptr->me->pun->userid, (i%6==5) ? '\n' : ' ');
		ptr = &dist[ptr->prev->midx];
	}
	fprintf(fp, "%s", dist[0].me->pun->userid);
END_WITH_RESULT:
	fclose(fp);
	sprintf(fpath, FRIENDS_TMPDIR "/%s.%s.path", from ,to);
	rename(tmpf, fpath);
	return;
END_WITH_ERROR:
	fclose(fp);
	unlink(tmpf);
	return;
}



typedef struct {
	char userid[IDLEN+1];
	int to_min, from_dist;
	int to_num, from_num;
	int div_num;
} TopSort;

static int cmp_tonum(void *p1, void *p2) {
	return ((TopSort*)p1)->to_num < ((TopSort*)p2)->to_num;
}
		
static int cmp_fromnum(void *p1, void *p2) {
	return ((TopSort*)p1)->from_num < ((TopSort*)p2)->from_num;
}
	
static int cmp_divnum(void *p1, void *p2) {
	return ((TopSort*)p1)->div_num < ((TopSort*)p2)->div_num;
}

static int cmp_fromdist(void *p1, void *p2) {
	return ((TopSort*)p1)->from_dist < ((TopSort*)p2)->from_dist;
}

static int cmp_tomin(void *p1, void *p2) {
	return ((TopSort*)p1)->to_min > ((TopSort*)p2)->to_min;
}

static int sum_of_queue(void *p1, void *p2) {
	*((int *)p2) += (INITIAL_PATH - ((Relationship *)p1)->distance);
	return 0;
}

static int min_of_queue(void *p1, void *p2) {
	Relationship *pr;
	int *p;
	
	pr = p1;
	p = p2;
	if(pr->distance < *p)
		*p = pr->distance;
	return 0;
}

static void friends_top10() {
	ght_iterator_t it;
	void *key;
	UserNode *tmp;
	int i, j;
	TopSort *ts;
	FILE *fp;

	ts = calloc(MAXUSERS, sizeof(TopSort));
	if(ts == NULL)
		return;
	
	for(i = 0, tmp = ght_first(fidx, &it, &key); tmp;
			tmp = ght_next(fidx, &it, &key)) {
		strcpy(ts[i].userid, tmp->userid);
		ts[i].to_num = queue_len(tmp->to);
		ts[i].from_num = queue_len(tmp->from);
		ts[i].to_min = INITIAL_PATH;
		apply_queue(tmp->to, min_of_queue, &ts[i].to_min);
		apply_queue(tmp->from, sum_of_queue, &ts[i].from_dist);
		ts[i].from_dist /= (ts[i].from_num ? ts[i].from_num : 1);
		i++;
	}

	//十大大众情人，"from"队列长度
	qsort(ts, i, sizeof(TopSort), (void *)cmp_fromnum);
	fp = fopen(FRIENDS_TMPDIR "/top10.idolA.tmp", "w+");
	if(fp == NULL)
		goto END_FREE_TS;
	for(j = 0; j < 10 && j < i; j++) {
		if(ts[j].from_num == 0)
			break;
		fprintf(fp, "\t\t%12s\t%4d\n", ts[j].userid, ts[j].from_num);
	}
	fclose(fp);
	rename(FRIENDS_TMPDIR "/top10.idolA.tmp", FRIENDS_TMPDIR "/top10.idolA");

	//十大博爱粉丝，"to"队列长度
	qsort(ts, i, sizeof(TopSort), (void *)cmp_tonum);
	fp = fopen(FRIENDS_TMPDIR "/top10.fansA.tmp", "w+");
	if(fp == NULL)
		goto END_FREE_TS;
	for(j = 0; j < 10 && j < i; j++) {
		if(ts[j].to_num == 0)
			break;
		fprintf(fp, "\t\t%12s\t%4d\n", ts[j].userid, ts[j].to_num);
	}
	fclose(fp);
	rename(FRIENDS_TMPDIR "/top10.fansA.tmp", FRIENDS_TMPDIR "/top10.fansA");

	//十大实力偶像, "from"队列值平均距离
	qsort(ts, i, sizeof(TopSort), (void *)cmp_fromdist);
	fp = fopen(FRIENDS_TMPDIR "/top10.idolB.tmp", "w+");
	if(fp == NULL)
		goto END_FREE_TS;
	for(j = 0; j < 10 && j < i; j++) {
		if(ts[j].from_dist == 0)
			break;
		fprintf(fp, "\t\t%12s\t%4d\n", ts[j].userid, ts[j].from_dist);
	}
	fclose(fp);
	rename(FRIENDS_TMPDIR "/top10.idolB.tmp", FRIENDS_TMPDIR "/top10.idolB");

	
	//十大忠诚粉丝, "to"队列最小值
	qsort(ts, i, sizeof(TopSort), (void *)cmp_tomin);
	fp = fopen(FRIENDS_TMPDIR "/top10.fansB.tmp", "w+");
	if(fp == NULL)
		goto END_FREE_TS;
	for(j = 0; j < 10 && j < i; j++) {
		if(INITIAL_PATH-ts[j].to_min == 0)
			break;
		fprintf(fp, "\t\t%12s\t%4d\n", ts[j].userid, INITIAL_PATH-ts[j].to_min);
	}
	fclose(fp);
	rename(FRIENDS_TMPDIR "/top10.fansB.tmp", FRIENDS_TMPDIR "/top10.fansB");
	
	//十大人气圈子
	bzero(ts, sizeof(TopSort)*MAXUSERS);
	for(tmp = ght_first(fidx, &it, &key); tmp;
			tmp = ght_next(fidx, &it, &key)) {
		if(tmp->div > 0 && tmp->div < MAXUSERS) {
			if(ts[tmp->div].userid[0] == '\0') //first user as representive
				strcpy(ts[tmp->div].userid, tmp->userid);
			ts[tmp->div].div_num++;
		}
	}
	qsort(ts, MAXUSERS, sizeof(TopSort), (void *)cmp_divnum);
	fp = fopen(FRIENDS_TMPDIR "/top10.bigdiv.tmp", "w+");
	if(fp == NULL)
		goto END_FREE_TS;
	for(j = 0; j < 10 && ts[j].div_num > 0; j++)
		fprintf(fp, "\t\t%12s\t%4d\n", ts[j].userid, ts[j].div_num);
	fclose(fp);
	rename(FRIENDS_TMPDIR "/top10.bigdiv.tmp", FRIENDS_TMPDIR "/top10.bigdiv");

END_FREE_TS:
	free(ts);
	return;
}
		


// -----------------------------daemon----------------------------------//
static void
set_sync_flag(int signno)
{
	sync_flag = 1;
}

static void
write_back_all(int signno)
{
	save_friends(fidx);
	close(msqid);
	close(lockfd);
	exit(0);
}

static void sig_child(int signo)
{
	while(waitpid(-1, NULL, WNOHANG) > 0)
		;
}

static void handle_msg(char *msg) {
	char *opuid, *tauid, *ptr;
	int flag, distance, topn;
	
	switch(*msg) {
	case 'i': //msg格式  i[SPACE]opuid['\0']
	case 'f':
		if(fork() == 0) {
			flag = (*msg == 'i') ? 1 : 0;
			opuid = msg + 2;
			topn = 999;
			distance = NO_PATH;
			query_neighbour(flag, opuid, topn, distance);
			exit(0);
		}
		break;
	case 'd': //msg格式  d[SPACE]opuid['\0']
		if(fork() == 0) {
			opuid = msg + 2;
			query_adiv(opuid);
			exit(0);
		}
		break;
	case 'p': //msg格式  p[SPACE]opuid[SPACE]tauid['\0']
		if(fork() == 0) {
			opuid = msg + 2;
			ptr = strchr(opuid, ' ');
			if(ptr) {
				*ptr = '\0';
				tauid = ptr + 1;
				distance = NO_PATH;
				query_path(opuid, tauid, distance);
			}
			exit(0);
		}
		break;
	default:
		update_friends(msg);
		break;
	}
}		
		

int main(int argc, char *argv[]) {
	char *str;
	int i = 0, day = 0, hour = 4;

	umask(027);

	lockfd = open(MY_BBS_HOME "/friends.lock", O_CREAT | O_RDONLY, 0660);
	if (flock(lockfd, LOCK_EX | LOCK_NB) < 0)
		exit(1);
	
	if(argc == 2) {
		if(!strcmp(argv[1], "-d")) { //run as daemon
			if (fork())
				return 0;
			setsid();
			if (fork())
				return 0;
			close(0);
			close(1);
			close(2);
		}
	}
	
	msqid = initFriendsMSQ();
	if (msqid < 0)
		exit(1);
	
	fidx = ght_create(MAXUSERS + 3);
	if (fidx == NULL)
		exit(1);

	mkdir(FRIENDS_TMPDIR, 0770);	//try making tmp dir
	if(load_friends() == -1)
		exit(1);	
	div_friends();
	friends_top10();
	
	signal(SIGHUP, set_sync_flag);	//sync if disconnected
	signal(SIGTERM, write_back_all);
	signal(SIGCHLD, sig_child);
	while (1) {
		while ((str = rcvlog(msqid, 1))) {
		//	printf("%s\n", str);
			handle_msg(str);
		}
		sleep(5);
		i++;
		if (i == hour*12*60 || sync_flag) { //write back from time to time
			system("rm -f " FRIENDS_TMPDIR "/*");
			if(++day == 24/hour) {//daily
				day = 0;
				estrange_by_time();
			}
			save_friends();
			div_friends();
			friends_top10();
			i = 0;
			sync_flag = 0;
		}
	}
	return 0;
}

