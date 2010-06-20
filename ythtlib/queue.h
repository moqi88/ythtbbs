// the following declares queue APIs.
// see the implementation in queue.c
// Author:	cojie
// date:	2006.02.26
#ifndef _QUEUE_H_
#define _QUEUE_H_

typedef struct Node {
	void *val;	// the content
	struct Node *next;
}Node, *pNode;

typedef struct LinkList {
	pNode head;
} LinkList, *pLinkList;

/* init a queue. You can NOT use other APIs before init.
On success, return the pointer of queue-head, return -1 if failure. */
pLinkList init_queue();

/* insert the 2nd parameter val into pList. Better use malloc() to get
val but not static declaration.
On success, return 0, else return -1. */
int insert_node(pLinkList pList, void *val);
/* search and return the first node in pList, using search_func to compare key with
node->val.
On find, return the pointer of first node that matches; else return NULL. */
void* search_queue(pLinkList pList, int (*search_func)(void *, void *), void *key);

/* read the 1st node from pList and return the pointer of val. if pList is
empty, NULL is returned. */
void* read_first(pLinkList pList);

/* delete the node specified by index. If index is beyond the region of pList, 
nothing will be done and -1 returned. If you want ro free val, use flag=1, else
flag=0.
On delete, return 0, else return -1 */
int delete_node(pLinkList pList, int index, int flag);

/* delete the node specified by address. If you want to free val, use flag=1, else flag=0.
On delete, return 0, else return -1 */
int delete_node2(pLinkList pList, void *addr, int flag);

/* delete the node specified by the return value of func(return non-zero to del).
 * If you want to free val, use flag=1, else flag=0.
On delete, return 0, else return -1 */
int delete_node3(pLinkList pList, int (*func)(void*, void *), void *param, int flag);

/* return whether the queue is empty. */
int is_empty(pLinkList pList);

/* return the length of queue */
int queue_len(pLinkList pList);

/* free the queue. If you want free all val-s, use flag=1, else flag = 0. */
void free_queue(pLinkList pList, int flag);

/* apply a queue. func is a callback function and deal with each val(1st parameter). 
the 2nd parameter of func is param. */
int apply_queue(pLinkList pList, int (*func)(void *, void *), void *param);

/* reverse the nodes of queue. */
void reverse_queue(pLinkList pList);

#endif
