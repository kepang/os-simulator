#define MAX_LISTS 1000 // use dummy header
#define MAX_NODES 10000

typedef struct node NODE;
struct node {
	int id;
	void *item;
	NODE *next;
	NODE *prev;
};

typedef struct list LIST;
struct list {
	//NODE node[MAX_ITEMS];
	NODE node;
	NODE *curr;
	NODE *head;
	LIST *nextlist;
	LIST *prevlist;
	int num_items;
	// boundary value:
	// -1: curr ptr before start of list
	// 0: curr ptr in the list
	// 1: curr ptr beyond end of list
	int boundary;
};

typedef struct listheads LISTHEADS;
struct listheads {
	LIST head[MAX_LISTS];
	LIST *array_head;
	LIST *free;
	NODE *freenode;
	int num_nodes;
	int num_lists;
};

LIST* ListCreate();
int ListCount(LIST*);
void* ListFirst(LIST*);
void* ListLast(LIST*);
void* ListNext(LIST*);
void* ListPrev(LIST*);
void* ListCurr(LIST*);
int ListAdd(LIST*, void*);
int ListInsert(LIST*, void*);
int ListAppend(LIST*, void*);
int ListPrepend(LIST*, void*);
void* ListRemove(LIST*);
void ListConcat(LIST*, LIST*);
void ListFree(LIST*, void(*itemFree)(void*));
void* ListTrim(LIST*);
void* ListSearch(LIST*, int(*comparator)(void*, void*), void*);
