#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "queue.h"


pLinkList init_queue() {
	pLinkList ptr;
	
	ptr = calloc(1, sizeof(LinkList));
	if(ptr) {
		ptr->head = calloc(1, sizeof(Node));
		if(ptr->head) {
			ptr->head->val = calloc(1, sizeof(int));
			if(ptr->head->val == NULL) {
				free(ptr->head);
				free(ptr);
				ptr = NULL;
			}
		} else {
			free(ptr);
			ptr = NULL;
		}
	}
	return ptr;
}

int is_empty(pLinkList pList) {
	return (pList->head->next == NULL);
}

int queue_len(pLinkList pList) {
	return *((int*)pList->head->val);
}

int insert_node(pLinkList pList, void *val) {
	pNode ptr, node;

	ptr = pList->head;
	while(ptr->next) {
		ptr = ptr->next;
	}
	node = calloc(1, sizeof(Node));
	if(node == NULL)
		return -1;
	node->val = val;
	ptr->next = node;
	(*((int*)pList->head->val))++;
	return 0;
}

int apply_queue(pLinkList pList, int (*func)(void *, void *), void *param) {
	pNode ptr, tmp;
	ptr = pList->head;

	while(ptr->next) {
		tmp = ptr->next->next;
		func(ptr->next->val, param);
		ptr = ptr->next;
	}
	return 0;
}

void* search_queue(pLinkList pList, int (*search_func)(void *, void *), void *key) {
	pNode ptr;
	ptr = pList->head;
	
	while(ptr->next) {
		if(search_func(ptr->next->val, key) == 1)
			return ptr->next->val;
		ptr = ptr->next;
	}
	return NULL;
}

void* read_first(pLinkList pList) {
	pNode ptr;
	ptr = pList->head;

	if(ptr)
		return ptr->next->val;
	else
		return NULL;
}

int delete_node(pLinkList pList, int index, int flag) {
	int i;
	pNode ptr, tmp;
	
	ptr = pList->head;
	for(i=0; ptr->next; i++, ptr = ptr->next) {
		if(i == index) {
			tmp = ptr->next->next;
			if(flag)
				free(ptr->next->val);
			free(ptr->next);
			ptr->next = tmp;
			(*((int*)pList->head->val))--;
			return 0;
		}
	}
	return -1;
}

int delete_node2(pLinkList pList, void *addr, int flag) {
	pNode ptr, tmp;
	
	ptr = pList->head;
	while(ptr->next) {
		if(ptr->next->val == addr) {
			tmp = ptr->next->next;
			if(flag)
				free(ptr->next->val);
			free(ptr->next);
			ptr->next = tmp;
			(*((int*)pList->head->val))--;
			return 0;
		}
		ptr = ptr->next;
	}
	return -1;
}

int delete_node3(pLinkList pList, int (*func)(void*, void *), void *param, int flag) {
	int i;
	pNode ptr, tmp;
	
	ptr = pList->head;
	i = -1;
	while(ptr->next) {
		if(func(ptr->next->val, param)) {
			tmp = ptr->next->next;
			if(flag)
				free(ptr->next->val);
			free(ptr->next);
			ptr->next = tmp;
			(*((int*)pList->head->val))--;
			i = 0;
		} else
			ptr = ptr->next;
	}
	return i;
}

void free_queue(pLinkList pList, int flag) {
	pNode ptr, tmp;
	ptr = pList->head;
	
	while(ptr->next) {
		tmp = ptr->next->next;
		if(flag)
			free(ptr->next->val);
		free(ptr->next);
		ptr->next = tmp;
	}
	free(pList->head->val);
	free(pList->head);
	free(pList);
}

void reverse_queue(pLinkList pList) {
	pNode pre, curr, next;

	pre = NULL;
	curr = pList->head->next;
	next = NULL;
	
	while(curr && curr->next) {
		next  = curr->next;
		curr->next = pre;
		pre = curr;
		curr = next;
	}
	if(next) {
		next->next = pre;
		pList->head->next = next;
	} else
		pList->head->next = curr;
}


