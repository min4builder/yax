#ifndef _MULTITASK_H
#define _MULTITASK_H

#include <sys/types.h>
#include <yax/const.h>
#include <yax/rfflags.h>
#include "arch.h"
#include "fds.h"
#include "lock.h"
#include "mem/pgdir.h"
#include "name.h"

typedef struct Proc Proc;

typedef struct {
	Lock l;
	int sem;
	Proc *waiter;
} Sem;

void seminit(Sem *, int);
void semwait(Sem *, int);
void semsignal(Sem *, int);

#define condwait(s) semwait(s, 1)
#define condsignal(s) semsignal(s, 1)

struct Proc {
	pid_t pid;
	clock_t quantum;
	clock_t priority;
	Proc *parent;

	FdList *fds;
	char *cwd; /* TODO make this CoW reference counted */
	MountTab *mounttab;

	char exitstring[ERRLEN];

	void *handling;
	void (*handler)(void *, const char *);
	Sem *blocked;
	int blockedn;
	Sem *blocked2;
	int blockedn2;

	PgDir *pd;
	void *sp;
	void *syscallstack;
	void *kstack;

	Proc *next;
	Proc *bnext;
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

