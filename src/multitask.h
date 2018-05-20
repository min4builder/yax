#ifndef _MULTITASK_H
#define _MULTITASK_H

#include <sys/types.h>
#include <yax/const.h>
#include <yax/rfflags.h>
#include "arch.h"
#include "fds.h"
#include "lock.h"
#include "virtmman.h"

typedef struct Proc Proc;

typedef struct {
	Lock l;
	int done;
	Proc *waiter;
} Condition;

void condwait(Condition *);
void condsignal(Condition *);

struct Proc {
	pid_t pid;
	clock_t quantum;
	clock_t priority;
	Proc *parent;

	FdList *fds;

	char exitstring[ERRLEN];

	void *handling;
	void (*handler)(void *, const char *);
	Condition *blocked;
	Condition *blocked2;

	PgDir *pd;
	void *sp;
	void *syscallstack;
	void *kstack;

	Proc *next;
	Proc *nextpid;
};

extern Proc *curproc;

void procsetup(void);
void procswitch(void);
pid_t procrfork(enum rfflags);
void procexits(const char *);
void proclightnew(void (*)(void *), void *);
void proclightexit(void);
void procsleep(clock_t);
void procalarm(clock_t);
void procnotify(int, const char *);
void (*proconnotify(void (*)(void *, const char *)))(void *, const char *);
void procnoted(int);

#endif /* _MULTITASK_H */

