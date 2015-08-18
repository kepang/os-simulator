/*
 *  structures.h
 *  A3
 *
 *  Created by kp on 21/03/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "list.h"

typedef struct proc_msg PROC_MSG;
struct proc_msg {
	int dest, src, type;
	char *body;
};

typedef struct pcb PCB;
struct pcb {
	int pid, priority, state;
	PROC_MSG *msg;
};

typedef struct sem SEM;
struct sem {
	int sid, value;
	LIST *slist;
};
