#include<stdlib.h>
#include<stdio.h>
#include "list.h"

static LISTHEADS listheads;
static NODE node[MAX_NODES];

// UTILITY FUNCTIONS
/* init arrays */
void static init_listheads_array(LISTHEADS *listheads) {
	int i;
	listheads->num_lists = 0;
	listheads->array_head = &listheads->head[0]; // dummy header
	listheads->array_head->nextlist = listheads->array_head; // point to dummy header
	listheads->array_head->prevlist = listheads->array_head;
	listheads->free = &listheads->head[1]; // start of free list
	// init free list ptrs
	for (i=1; i<MAX_LISTS; i++) {
		if (i == MAX_LISTS - 1) {
			listheads->head[i].nextlist = NULL;
		}
		else {
			listheads->head[i].nextlist = &listheads->head[i+1];
		}
	}
}

void static init_nodes_array(NODE *node) {
	int i;
	//node->num_nodes = 0;
	//node->free = &node[0];
	listheads.num_nodes = 0;
	listheads.freenode = &node[0];
	// init ptrs
	for (i=0; i<MAX_NODES; i++) {
		node[i].id = i;
		if (i == MAX_NODES - 1) {
			node[i].next = NULL;
		}
		else {
			node[i].next = &node[i+1];
		}
	}
}
/* end init */

// free (release) list
void static free_list(LIST *list) {
	LIST *prevlisthead;
	LIST *nextlisthead;

	// free list
	prevlisthead = list->prevlist;
	nextlisthead = list->nextlist;
	
	prevlisthead->nextlist = nextlisthead;
	nextlisthead->prevlist = prevlisthead;

	list->num_items = 0;
	list->boundary = 0;
	list->prevlist = NULL;
	list->nextlist = listheads.free;
	
	listheads.free = list;
	listheads.num_lists--;
}

// START LIST OPERATION FUNCTIONS

// return list ptr or NULL
LIST *ListCreate() {
	static int listheads_array_init = 0;
	static int nodes_array_init = 0;
	LIST *list;
	list = NULL; // return ptr to created list or null if full

	// run once
	// init listheads array:
	if (!listheads_array_init) {		
		init_listheads_array(&listheads);
		listheads_array_init = 1;
	}
	// init nodes array:
	if (!nodes_array_init) {
		init_nodes_array(node);
		nodes_array_init = 1;
	}

	// init and create list:
	if (listheads.num_lists < MAX_LISTS - 1) {
		// add list
		LIST *nextfree;
		nextfree = listheads.free->nextlist;
		listheads.array_head->prevlist->nextlist = listheads.free;
		listheads.free->prevlist = listheads.array_head->prevlist;
		listheads.array_head->prevlist = listheads.free;
		listheads.free->nextlist = listheads.array_head;
		listheads.free = nextfree;

		// init list
		list = listheads.array_head->prevlist;
		list->head = &list->node;		
		list->head->prev = list->head;
		list->head->next = list->head;
		list->curr = list->head;
		
		list->boundary = 0;
		list->num_items = 0;
		listheads.num_lists++;
	}
	return list;
}

// return # items in list
int ListCount(LIST *list) {
	return list->num_items;
}

// return ptr
void *ListFirst(LIST *list) {
	list->boundary = 0;
	list->curr = list->head->next;
	return list->curr->item;
}

// return ptr
void *ListLast(LIST *list) {
	list->boundary = 0;
	list->curr = list->head->prev;
	return list->curr->item;
}

// return ptr or NULL
void *ListNext(LIST *list) {
	list->curr = list->curr->next;

	if (list->curr != list->head) {
		return list->curr->item;
	}
	else {
		list->boundary = 1;
		return NULL;
	}
}

// return ptr or NULL
void *ListPrev(LIST *list) {
	list->curr = list->curr->prev;
	
	if (list->curr != list->head) {
		return list->curr->item;
	}
	else {
		list->boundary = -1;
		return NULL;
	}
}

// return ptr
void *ListCurr(LIST *list) {
	return list->curr->item;
}

// return 0 (success) or -1 (fail)
// adds item after curr_ptr
int ListAdd(LIST *list, void *item) {
	NODE *nextfree;

	// boundary condition
	if (list->curr == list->head) {
		// beyond end
		if (list->boundary == 1) {
			list->curr = list->head->prev;
			list->boundary = 0;
		}
		// do nothing if:
		// list size = 0
		// boundary condition before start
	}

	// set up free node
	if (listheads.num_nodes < MAX_NODES) {
		nextfree = listheads.freenode->next;
		listheads.num_nodes++;
		listheads.freenode->item = item;
		listheads.freenode->next = list->curr->next;
		listheads.freenode->prev = list->curr;
		list->curr->next->prev = listheads.freenode;
		list->curr->next = listheads.freenode;
		listheads.freenode = nextfree;
		// make item curr item
		list->curr = list->curr->next;
	}
	else {
		return -1;
	}

	// chk
	if (list->curr->item == item) {
		list->num_items++;
		return 0;
	}
	else {
		return -1;
	}
}

// return 0 (success) or -1 (fail)
int ListInsert(LIST *list, void *item) {
	list->curr = list->curr->prev;
	if (list->curr == list->head) {
		list->boundary = -1;
	}
	return ListAdd(list, item);
}

// return 0 (success) or -1 (fail)
int ListAppend(LIST *list, void *item) {
	//list->curr = list->head->prev;
	ListLast(list);
	return ListAdd(list, item);
}

// return 0 (success) or -1 (fail)
int ListPrepend(LIST *list, void *item) {
	list->curr = list->head;
	list->boundary = 0;
	return ListAdd(list, item);
}

// return ptr
void *ListRemove(LIST *list) {
	NODE *prevnode, *nextnode, *freenode;

	if (list->curr == list->head) {
		return NULL;
	}

	prevnode = list->curr->prev;
	nextnode = list->curr->next;

	// add free node to free list
	freenode = list->curr;
	freenode->next = listheads.freenode;
	listheads.freenode = freenode;

	// set ptrs
	prevnode->next = nextnode;
	nextnode->prev = prevnode;
	list->curr = nextnode;
	if (list->curr == list->head) {
		list->boundary = 1;
	}
	
	list->num_items--;
	listheads.num_nodes--;
	return listheads.freenode->item;
}

// return void
void ListConcat(LIST *list1, LIST *list2) {
	void *item;
	// add to list1
	//list1->curr = list1->head->prev;
	//list2->curr = list2->head->next;
	ListLast(list1);
	ListFirst(list2);
	while (list2->curr != list2->head) {
		item = ListRemove(list2);
		ListAdd(list1, item);
	}

	// rmv list2
	free_list(list2);
}

// return void
void ListFree(LIST *list, void(*itemFree)(void*)) {
	// free every item in list
	//list->curr = list->head->next;
	ListFirst(list);
	while (list->curr != list->head) {
		(*itemFree)(list->curr->item);
		ListRemove(list);
	}

	// list goes to free list
	free_list(list);
}

// return ptr
void *ListTrim(LIST *list) {
	void *item;
	//list->curr = list->head->prev;
	ListLast(list);
	item = ListRemove(list);
	ListLast(list);
	return item;
}

// return ptr
void *ListSearch(LIST *list, int(*comparator)(void*, void*), void* comparisonArg) {

	while (list->curr != list->head) {
		if ((*comparator)(list->curr->item, comparisonArg)) {
			return list->curr;
		}
		list->curr = list->curr->next;
	}
	// match not found
	list->boundary = 1; // beyond end
	return NULL;
}
