#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>
#include "structures.h"


#define MAX_SIZE 5000
// states
#define READY 100
#define RUNNING 101
#define BLOCKED 102
// returnObj
#define RET_QUEUE 200
#define RET_PCB 201
// msg type
#define SEND 300
#define RECEIVE 301
#define REPLY 302



char* translate_type(int type);



LIST *readyQ[3];
LIST *sendQ, *recvQ, *msgQ, *semQ, *runQ;

int debug_msg = 1;
int enable_printf = 1;
int numproc = 0;
int proc_counter = 0;


// function pointers

void freemem(void *x) {
	printf("free mem: %c\n", *((char*)x));
}

int compareitem(void *item, void *compareArg) {
	int *pid1, *pid2;
	pid1 = ((int*)item);
	pid2 = ((int*)compareArg);
	
	if (*pid1 == *pid2) {
		return 1;
	}
	else {
		return 0;
	}
}

// functions
void display(char *str) {
	if (enable_printf) {
		printf("%s\n", str);
	}
}

void debug(char *str) {
	if (debug_msg) {
		printf("%s\n", str);
	}
}

void resetQ_ptrs() {
	ListFirst(readyQ[0]);
	ListFirst(readyQ[1]);
	ListFirst(readyQ[2]);
	ListFirst(sendQ);
	ListFirst(recvQ);
	ListFirst(runQ);
	ListFirst(semQ);
}

void reset_pm(PROC_MSG *pm) {
	pm->src = -1;
	pm->dest = -1;
	strcpy(pm->body, "");
	//memset(pm->body, (int) NULL, sizeof pm->body);
}

void display_pm(PROC_MSG *pm) {
	
	display("-------");
	display("Message Available:");
	printf("Type: %s\n", translate_type(pm->type));
	printf("From pid: %i - ", pm->src);
	display(pm->body);
	display("-------");
	
}
	

char* translate_state(int state) {
	char *str;
	
	if (state == READY) {
		str = "Ready";
	}
	else if (state == RUNNING) {
		str = "Running";
	}
	else if (state == BLOCKED) {
		str = "Blocked";
	}
	return str;
}

char* translate_type(int type) {
	char *str;
	
	if (type == SEND) {
		str = "Send";
	}
	else if (type == RECEIVE) {
		str = "Receive";
	}
	else if (type == REPLY) {
		str = "Reply";
	}
	return str;
}

void CPU_scheduler() {
	PCB *p;
	int num = 3;
	int i, status;
	
	for (i=0; i<num; i++) {
		if (ListCount(readyQ[i])) {
			// take out item
			p = ListTrim(readyQ[i]);
			// add to runQ
			if (p) {
				status = ListAppend(runQ, p);
				p->state = RUNNING;
			}
			
			// display status messages
			if (status == 0) {
				display("CPU Schedule SUCCESS");
			}
			else {
				display("CPU Schedule FAIL");
			}
			// exit loop
			i = 99;
		}
	}
	// set init proc state
	if (p = ListLast(runQ)) {
		if (p->pid == 0) {
			p->state = RUNNING;
		}
		printf("pid: %i now running. \n", p->pid);
		
		if (strlen(p->msg->body) != 0) {
			display_pm(p->msg);
		}

		reset_pm(p->msg);
		
	}
}

void* findpid_fromQ(int pid, LIST *list) {
	int (*comparator)(void*, void*);
	NODE *n;
	
	comparator = &compareitem;
	ListFirst(list);
	n = ListSearch(list, comparator, (void*)&pid);
	if (n) {
		return n->item;
	}
	else {
		return NULL;
	}
}

// returnObj = 1 // return Queue
// returnObj = 2 // return PCB
void* findpid(int pid, int returnObj) {
	int (*comparator)(void*, void*);
	LIST *list = NULL;
	NODE *n = NULL;

	comparator = &compareitem;
	resetQ_ptrs();
	if (n = ListSearch(readyQ[0], comparator, (void*)&pid)) {
		list = readyQ[0];
	}
	else if (n = ListSearch(readyQ[1], comparator, (void*)&pid)) {
		list = readyQ[1];
	}
	else if (n = ListSearch(readyQ[2], comparator, (void*)&pid)) {
		list = readyQ[2];
	}
	else if (n = ListSearch(sendQ, comparator, (void*)&pid)) {
		list = sendQ;
	}
	else if (n = ListSearch(recvQ, comparator, (void*)&pid)) {
		list = recvQ;
	}
	else if (n = ListSearch(runQ, comparator, (void*)&pid)) {
		list = runQ;
	}
	else if (ListCount(semQ)) {
		SEM *s;
		int i, count;
		count = ListCount(semQ);
		ListPrev(semQ);
		for (i=0; i<count; i++) {
			// start from first node of semQ and s->list
			s = ListNext(semQ);
			ListFirst(s->slist);
			if (n = ListSearch(s->slist, comparator, (void*)&pid)) {
				list = s->slist;
				i = count;
			}
		}
	}
	
	
	if (returnObj == RET_QUEUE) {
		return list;
	}
	if (returnObj == RET_PCB) {
		return n->item;
	}
	
	return NULL;
}

void display_queue(LIST *list) {
	void *ptr;
	char *str;
	int id;
	
	ListFirst(list);
	ListPrev(list);
	if (list == msgQ) {
		str = "dest";
	}
	else if (list == semQ) {
		str = "sid";
	}
	else {
		str = "pid";
	}
	
	printf("%s: ", str);
	while (ptr = ListNext(list)) {
		if (list == semQ) {
			SEM *s;
			s = (SEM*) ptr;
			id = s->sid;
		}
		else {
			PCB *p;
			p = (PCB*)ptr;
			id = p->pid;
		}
		printf("%i,", id);
	}
	display("");
}




// commands

char* hi() {
	return "hi";
}

char* create(int priority, PROC_MSG *pm) {
	char *report;
	char *buf;
	int size;
	//PROC_MSG *pm;
	
	if (priority < 0 || priority > 2) {
		priority = 2;
	}
	
	size = ListCount(readyQ[priority]);
	
	numproc++;
	proc_counter++;
	PCB *p = malloc(sizeof (PCB));
	p->pid = proc_counter;
	p->priority = priority;
	p->state = READY;
	
	p->msg = (PROC_MSG*) malloc(sizeof (PROC_MSG));
	p->msg->body = (char*) malloc(sizeof(char) * 40);
	

	if (pm != NULL) {
		p->msg->dest = pm->dest;
		p->msg->src = pm->src;
		p->msg->type = pm->type;
		strcpy(p->msg->body, pm->body);
	}
	else {
		reset_pm(p->msg);
	}
	
	
	ListPrepend(readyQ[priority], p);
	if (ListCount(readyQ[priority]) == size+1) {
		buf = "SUCCESS: pid";
		
		//sprintf (report, "%s%i", buf, p->pid);

		printf("SUCCESS: pid%i\n", p->pid);
	}
	else {
		report = "FAIL";
		printf("FAIL\n");
	}
	
	return report;
}

char* fork_cmd() {
	char *report;
	PCB *p;
	
	p = ListLast(runQ);
	
	if (p) {
		report = create(p->priority, p->msg);
	}
	
	return report;
}

char* kill(int pid, LIST* list) {
	char *report;
	PCB *p;
	int fail = 1;
	
	if (list) {
		p = ListRemove(list);
		fail = 0;
	}

	if (p && !fail) {
		display("SUCCESS");
		printf("pid:%i removed\n", p->pid);
		free(p->msg->body);
		free(p->msg);
		free(p);
		p->msg->body = 0;
		p->msg = 0;
		p = 0;
		numproc--;
	}
	else {
		display("FAIL");
	}
	
	return report;
}

char* exit_curr() {
	char* report;
	PCB *p;
	p = ListLast(runQ);
	if (numproc == 0 || p->pid != 0) {
		p = ListTrim(runQ);
		printf("pid:%i removed\n", p->pid);
		free(p->msg->body);
		free(p->msg);
		free(p);
		p->msg->body = 0;
		p->msg = 0;
		p = 0;
		numproc--;
		CPU_scheduler();
	}
	else {
		display("Cannot exit init process.  Other processes waiting.");
	}
	return report;
}

void quantum() {
	PCB *p;
	
	// take item out of runQ
	p = ListLast(runQ);
	if (p->pid != 0) {
		p = ListTrim(runQ);
		// descrease priority
		if (p->priority < 2) {
			p->priority++;
		}
		p->state = READY;
		// place on readyQ
		if (p) {
			ListPrepend(readyQ[p->priority], p);
			
			printf("pid: %i placed on ready queue.\n", p->pid);
			//printf("priority: %i\n", p->priority);
		}

	}
	else {
		// reset init proc state
		p->state = READY;
	}

	// run cpu scheduler
	CPU_scheduler();
}

void send(int pid, char* msg) {
	PCB *run, *p;
	PROC_MSG *pm;
	int src;
	int fail = 0;
	
	run = ListLast(runQ);
	src = run->pid;
	
	if (p = findpid_fromQ(pid, recvQ)) {
		p->msg->src = src;
		p->msg->dest = pid;
		p->msg->type = SEND;
		strcpy(p->msg->body, msg);
		// take out of recvQ
		p = ListRemove(recvQ);
		// add to readyQ
		p->state = READY;
		ListPrepend(readyQ[p->priority], p);
	}
	else if (findpid(pid, RET_QUEUE)){
		pm = malloc(sizeof pm);
		pm->body = (char*) malloc(sizeof(char) * 40);
		pm->src = src;
		pm->dest = pid;
		pm->type = SEND;
		strcpy(pm->body, msg);
		// put msg on msgQ
		ListAdd(msgQ, pm);
		run = ListLast(runQ);
		if (run->pid != 0) {
			//blk sender
			p = ListTrim(runQ);
			p->state = BLOCKED;
			ListAdd(sendQ, p);
			CPU_scheduler();
		}
	}
	else {
		fail = 1;
	}
	
	if (!fail) {
		display("SEND SUCCESSFUL");
	}
	else {
		display("SEND FAIL");
	}
		
}

void rx() {
	PROC_MSG *pm;
	PCB *p;
	int dest;
	p = ListLast(runQ);
	dest = p->pid;
	// chk msgQ
	// show msg, src
	if (pm = findpid_fromQ(dest, msgQ)) {
		display_pm(pm);
		ListRemove(msgQ);
	}
	// block
	else {
		// put process on recvQ
		// cpu sched
		if (p->pid != 0) {
			p = ListTrim(runQ);
			p->state = BLOCKED;
			ListPrepend(recvQ, p);
			CPU_scheduler();
		}
	}
}
	
void reply_cmd(int pid, char *msg) {
	PCB *run, *p;
	
	run = ListLast(runQ);
	
	if (p = findpid_fromQ(pid, sendQ)) {
		// copy msg to sender's pcb
		
		p->msg->dest = pid;
		p->msg->src = run->pid;
		p->msg->type = REPLY;
		strcpy(p->msg->body, msg);
		
		// unblock
		p = ListRemove(sendQ);
		p->state = READY;
		ListPrepend(readyQ[p->priority], p);
		
		display("REPLY SUCCESS");
	}
	else {
		// fail
		display("REPLY FAIL");
		
	}
}
	
void new_sem(int sid) {
	// init sem
	SEM *s;
	int fail = 1;
	//if sid 0-5 AND does not already exist;
	if (sid >= 0 && sid <= 4) {
		if (!findpid_fromQ(sid, semQ)) {
			s = malloc(sizeof(SEM));
			s->sid = sid;
			s->value = 0;
			s->slist = ListCreate();
			ListAdd(semQ, s);
			
			fail = 0;
		}
	}
	
	if (!fail) {
		display("SUCCESS");
		printf("SEM %i initialized\n", sid);
	}
	else {
		display("FAIL");
	}
}
	
void P(int sid) {
	SEM *s;
	PCB *p;
	int fail = 1;
	int block = 0;
	
	p = ListLast(runQ);
	s = findpid_fromQ(sid, semQ);	
	if (s && p->pid != 0) {
		s->value--;
		if (s->value < 0) {
			p = ListTrim(runQ);
			p->state = BLOCKED;
			ListPrepend(s->slist, p);
			fail = 0;
			block = 1;
		}
	}

	if (fail) {
		display("FAIL");
	}
	else {
		display("SUCCESS");
		if (block) {
			printf("pid: %i BLOCKED\n", p->pid);
		}
		CPU_scheduler();
	}		
}

void V(int sid) {
	SEM *s;
	PCB *p;
	int fail = 1;
	int ready_q = 0;
	
	s = findpid_fromQ(sid, semQ);
	if(s) {
		s->value++;
		if (s->value <= 0) {
			if (p = ListTrim(s->slist)) {
				ListPrepend(readyQ[p->priority], p);
				p->state = READY;
				ready_q = 1;
			}
		}
		fail = 0;		
	}
	
	if (fail) {
		display("FAIL");
	}
	else {
		display("SUCCESS");
		if (ready_q) {
			printf("pid: %i into READY QUEUE\n", p->pid);
		}
	}
}
			
void procinfo(int pid) {
	PCB *p;
	char *state;
	
	p = (PCB*) findpid(pid, RET_PCB);
	if (p) {

		state = translate_state(p->state);
		display("=========");
		printf("Proc Info for pid: %i\n", p->pid);
		printf("Priority: %i\n", p->priority);
		printf("State: %s\n", state);
		printf("Msg: %s\n", p->msg->body);
		display("=========");
	}
	else {
		printf("Cannot get Proc Info");
	}
}
	
void totalinfo() {
	PCB *p;
	char *state;
	int i, count;
	
	// runQ
	p = ListLast(runQ);
	state = translate_state(p->state);
	display("=========");
	display("RUN QUEUE");
	printf("pid: %i\n", p->pid);
	
	// readyQ
	display("READY QUEUES");
	for (i=0; i<3; i++) {
		printf("Priority [%i]:\n", i);
		display_queue(readyQ[i]);
	}
	
	// sendQ
	display("SEND QUEUE");
	display_queue(sendQ);
	display("RECEIVE QUEUE");
	display_queue(recvQ);
	display("MSG QUEUE");
	display_queue(msgQ);
	
	display("SEM QUEUES");
	if (count = ListCount(semQ)) {
		SEM *s;
		
		ListFirst(semQ);
		ListPrev(semQ);
		for (i=0; i<count; i++) {
			s = ListNext(semQ);
			printf("sid%i: ", s->sid);
			display_queue(s->slist);
		}
	}	
	
	
	//display("P() QUEUE");
	//display("V() QUEUE");
	display("=========");

}
	









int main (int argc, const char * argv[]) {
	char *str;
	char buf[40];
	int pid, priority, sid, status;
	
	
	//void (*itemFree)(void*);
	//int (*comparator)(void*, void*);
	
	char cmd;
	//char param;
	char *report;
	PCB *init;
	LIST *list;
	

	
	
	// create lists

	runQ = ListCreate();
	sendQ = ListCreate();
	recvQ = ListCreate();
	readyQ[0] = ListCreate();
	readyQ[1] = ListCreate();
	readyQ[2] = ListCreate();	
	msgQ = ListCreate();
	semQ = ListCreate();
	
	
	// init init process
	init = malloc(sizeof(PCB));
	init->pid = 0;
	init->priority = 0;
	init->state = RUNNING;
	
	init->msg = (PROC_MSG*) malloc(sizeof (PROC_MSG));
	init->msg->body = (char *) malloc(sizeof(char) * 40);

	reset_pm(init->msg);

	//strlen(init->msg->body);

	
	ListAdd(runQ, init);
	
	
	

	
	str = hi();
	while (ListCount(runQ)) {
		
		printf("Enter Command: ");
		scanf("%c", &cmd);
		cmd = toupper(cmd);

		switch (cmd) {
			// create
			case 'C':
				printf("Enter Priority (0, 1, 2): ");
				scanf("%d", &priority);
				report = create(priority, NULL);
				break;
			// fork
			case 'F':
				report = fork_cmd();
				break;
			case 'K':
				status = 0;
				printf("Enter pid: ");
				scanf("%d", &pid);
				if (pid == 0 && numproc != 0) {
					display("Cannot kill init process.");
					break;
				}
				// 
				list = findpid(pid, RET_QUEUE);
				report = kill(pid, list);
				if (list && list == runQ) {
					CPU_scheduler();
				}
				
				break;
			case 'E':
				exit_curr();
				break;
			case 'Q':
				quantum();
				break;
			case 'S':
				printf("Enter pid: ");
				scanf("%i", &pid);
				printf("Enter message: ");
				//scanf(" %s", buf);
				scanf(" %[^\n]", buf);
				send(pid, buf);
				break;
			case 'R':
				rx();
				break;
			case 'Y':
				printf("Enter pid: ");
				scanf("%i", &pid);
				printf("Enter reply: ");
				scanf(" %[^\n]", buf);
				reply_cmd(pid, buf);
				break;
			case 'N':
				printf("Enter SEM id: ");
				scanf("%i", &sid);
				new_sem(sid);
				break;
			case 'P':
				printf("Enter SEM id: ");
				scanf("%i", &sid);
				P(sid);
				break;
			case 'V':
				printf("Enter SEM id: ");
				scanf("%i", &sid);
				V(sid);
				break;
			case 'I':
				printf("Enter pid: ");
				scanf("%i", &pid);
				procinfo(pid);
				break;
			case 'T':
				totalinfo();
				break;

				
			default:
				break;
		}
		scanf("%c", &cmd);
		memset(&cmd, (int) NULL, sizeof cmd);

		//printf("%s\n", report);
		
	}
	
    return 0;
}
	
