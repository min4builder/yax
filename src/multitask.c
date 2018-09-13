#define NDEBUG
#define __YAX__
#include <string.h>
#include <sys/types.h>
#include <yax/const.h>
#include <yax/rfflags.h>
#include "arch.h"
#include "boot.h"
#include "int.h"
#include "lock.h"
#include "macro.h"
#include "mem/malloc.h"
#include "mem/pgdir.h"
#include "mem/usrboundary.h"
#include "printk.h"
#include "multitask.h"

typedef struct Sleep Sleep;
struct Sleep {
	Sleep *next;
	clock_t time;
	enum { SSLEEP, SALARM } tag;
	union {
		Sem s;
		Proc *p;
	} u;
};

static void notify(Proc *, const char *);

Proc *curproc = 0;
static Lock proclock;
static Proc *procbypid = 0;
static Proc *nullproc = 0;
static Sleep *sleeping = 0;

#define CRITSEC MACRO_WRAP_(int ints_were_enabled_ = entercs(), leavecs(ints_were_enabled_))
#define LEAVECS continue

static int entercs(void)
{
	int i = intoff();
	locklock(&proclock);
	return i;
}

static void leavecs(int i)
{
	lockunlock(&proclock);
	if(i)
		inton();
}

static Proc *findprev(Proc *p)
{
	Proc *prev;
	for(prev = p; prev->next != p; prev = prev->next) {}
	return prev;
}

static Proc *findpid(int pid)
{
	Proc *p = procbypid;
	while(p) {
		if(p->pid == pid)
			return p;
		p = p->nextpid;
	}
	return 0;
}

void procsetup(void)
{
	char *slash;
	nullproc = calloc(1, sizeof(Proc));
	nullproc->pid = 0;
	nullproc->quantum = 0;
	nullproc->priority = 5;
	nullproc->parent = nullproc;
	nullproc->fds = fdlistnew();
	slash = malloc(sizeof "/");
	strcpy(slash, "/");
	nullproc->cwd = strmk(slash);
	nullproc->mounttab = mountnew();
	nullproc->handling = 0;
	nullproc->handler = 0;
	nullproc->blocked = nullproc->blocked2 = 0;
	nullproc->pd = getpgdir();
	nullproc->syscallstack = kernel_stack_bottom;
	nullproc->kstack = kernel_stack_low;
	nullproc->next = nullproc;
	nullproc->bnext = 0;
	nullproc->nextpid = 0;
	curproc = procbypid = nullproc;
}

void timer_handler(void)
{
	Sleep *os;
	if(sleeping) {
		sleeping->time--;
		while(sleeping && sleeping->time <= 0) {
			if(sleeping->tag == SSLEEP)
				condsignal(&sleeping->u.s);
			else
				notify(sleeping->u.p, "alarm");
			os = sleeping;
			sleeping = sleeping->next;
			free(os);
		}
	}
	curproc->quantum--;
	if(curproc->quantum <= 0)
		procswitch();
}

void *procswitchgut(void *sp)
{
	Proc *prev;
	CRITSEC {
		prev = curproc;
		prev->quantum = prev->priority;
		curproc = curproc->next;
		/* if the next one is the null task, skip it;
		 * if *that* one is also the null task, we've got nothing else
		 * to run, therefore, we run the null task anyways. */
		if(curproc == nullproc)
			curproc = curproc->next;
		if(curproc != prev) {
			prev->sp = sp;
			prev->syscallstack = switchsyscallstack(curproc->syscallstack);
			prev->pd = switchpgdir(curproc->pd);
		}
	}
	cprintk('%');
	iprintk(curproc->pid);
	cprintk('%');
	return curproc->sp;
}

void seminit(Sem *s, int n)
{
	s->sem = n;
	s->waiter = 0;
}
void semwait(Sem *s, int n)
{
	Proc *p;
	int leftcs = 0;
	CRITSEC {
		s->sem -= n;
		if(s->sem >= 0) {
			leftcs = 1;
			LEAVECS;
		}
		curproc->bnext = s->waiter;
		s->waiter = curproc;
		if(!curproc->blocked && !curproc->blocked2) {
			curproc->blocked = s;
			curproc->blockedn = n;
		} else if(!curproc->blocked2) {
			curproc->blocked2 = s;
			curproc->blockedn2 = n;
		} else {
			printk("TRIPLE BLOCK: ");
			iprintk(curproc->pid);
			cprintk('\n');
			halt();
		}
		p = findprev(curproc);
		p->next = curproc->next;
	}
	if(!leftcs)
		procswitch();
}
void semsignal(Sem *s, int n)
{
	CRITSEC {
		s->sem += n;
		while(s->waiter && n > 0) {
			if(s->waiter->blocked != s && s->waiter->blocked2 != s) {
				printk("BLOCKING INCONSISTENCY: ");
				iprintk(s->waiter->pid);
				cprintk(' ');
				uxprintk((uintptr_t) s->waiter->blocked);
				cprintk(' ');
				uxprintk((uintptr_t) s->waiter->blocked2);
				cprintk('\n');
				halt();
			}
			if(!(s->waiter->blocked == s && s->waiter->blocked2 != 0)) {
				Proc *p = curproc;
				while(p->next != curproc)
					p = p->next;
				s->waiter->next = curproc;
				p->next = s->waiter;
			}
			if(s->waiter->blocked == s) {
				s->waiter->blocked = 0;
				n -= s->waiter->blockedn;
			} else if(s->waiter->blocked2 == s) {
				s->waiter->blocked2 = 0;
				n -= s->waiter->blockedn2;
			}
			s->waiter = s->waiter->bnext;
		}
	}
}

/* procrfork is in multitasks.s; it sets up a fake interrupt frame for this
 * routine to copy on the new task */
pid_t procrforkgut(void *sp, enum rfflags fl)
{
	static int nextpid = 1;
	Proc *p;
	Proc *new = calloc(1, sizeof(Proc));
	new->pid = nextpid++; /* XXX FIXME XXX */
	new->parent = curproc;
	new->pd = vpgcopy(fl & RFMEM);
	new->quantum = 0;
	new->priority = curproc->priority;
	new->kstack = vpgkmap(0x10000, PROT_READ | PROT_WRITE);
	new->syscallstack = (uint8_t *) new->kstack + 0x10000;
	new->cwd = curproc->cwd;
	ref(new->cwd);
	if(fl & RFNAMEG)
		new->mounttab = mountcopy(curproc->mounttab);
	else {
		new->mounttab = curproc->mounttab;
		ref(new->mounttab);
	}
	if(fl & RFFDG)
		new->fds = fdlistcopy(curproc->fds);
	else {
		ref(curproc->fds);
		new->fds = curproc->fds;
	}
	if(curproc->handling)
		new->handling = (uint8_t *) curproc->handling - (uint8_t *) curproc->syscallstack + (uint8_t *) new->syscallstack;
	else
		new->handling = 0;
	new->handler = curproc->handler;
	new->sp = (uint8_t *) sp - (uint8_t *) curproc->syscallstack + (uint8_t *) new->syscallstack;
	memcpy(new->sp, sp, (uint8_t *) curproc->syscallstack - (uint8_t *) sp);
	RFORKREGADJUST(new->sp, curproc->syscallstack, new->syscallstack);

	CRITSEC {
		new->next = curproc->next;
		curproc->next = new;
		for(p = procbypid; p->nextpid && p->nextpid->pid < new->pid; p = p->nextpid) {}
		new->nextpid = p->nextpid;
		p->nextpid = new;
	}

	return new->pid;
}

/* procexits is in asmschedule.s:
void procexits(const char *s)
{
	Proc *p;
	cli
	esp = procexitsgut1(&p, s);
	switchstacks(esp);
	procexitsgut2(p);
	sti
	iret
}
*/
void *procexitsgut1(Proc **cp, const char *s)
{
	Proc *p;
	Sleep **sl, *tmp;
	strlcpy(curproc->exitstring, s, ERRLEN);
	locklock(&proclock);
	p = findprev(curproc);
	p->next = curproc->next;
	unref(curproc->fds);
	unref(curproc->mounttab);
	for(sl = &sleeping; *sl;) {
		if((*sl)->tag == SALARM && (*sl)->u.p == curproc) {
			tmp = *sl;
			*sl = tmp->next;
			free(tmp);
		} else
			sl = &(*sl)->next;
	}
	if(curproc->blocked2) {
		printk("EXIT ON DOUBLE BLOCK\n");
		halt();
	}
	if(curproc->blocked) {
		curproc->blocked->waiter = 0;
		curproc->blocked = 0;
	}
	lockunlock(&proclock);
	*cp = curproc;
	vpgclear();
	return procswitchgut(0);
}

void procexitsgut2(Proc *cp)
{
	vpgdel(cp->pd);
	vpgunmap((uint8_t *) cp->kstack, 0x10000);
}

void proclightnew(void (*f)(void *), void *arg)
{
	Proc *new = calloc(1, sizeof(Proc));
	Regs r;
	new->pid = 0;
	new->pd = nullproc->pd;
	new->parent = nullproc;
	new->syscallstack = 0;
	new->handling = 0;
	new->handler = 0;
	new->fds = 0;
	new->quantum = 0;
	new->priority = nullproc->priority;
	new->kstack = vpgkmap(0x1000, PROT_READ | PROT_WRITE);
	new->sp = (uint8_t *) new->kstack + 0x1000;
	STACKPUSH(new->sp, &arg, sizeof(void *));
	STACKPUSH(new->sp, 0, sizeof(void *)); /* return addr */
	r.eip = (uint32_t) f;
	r.eflags = 0x202;
	r.ebp = 0;
	STACKPUSH(new->sp, &r, sizeof(Regs));

	CRITSEC {
		new->next = curproc->next;
		curproc->next = new;
	}
}

/* proclightexit is in asmschedule.s:
void proclightexit(void)
{
	Proc *p;
	cli
	esp = proclightexitgut1(&p, s);
	proclightexitgut2(p);
	sti
	iret
}
*/

void *proclightexitgut1(Proc **cp)
{
	Proc *p;
	locklock(&proclock);
	p = findprev(curproc);
	p->next = curproc->next;
	vpgunmap((uint8_t *) curproc->kstack, 0x1000);
	lockunlock(&proclock);
	*cp = curproc;
	return procswitchgut(0);
}

void proclightexitgut2(Proc *cp)
{
	vpgunmap((uint8_t *) cp->kstack, 0x1000);
	free(cp);
}

static void sleepalarm(clock_t time, int a)
{
	Sleep **cs, *ns = calloc(1, sizeof(Sleep));
	clock_t totaltime = 0;
	int leftcs = 0;
	CRITSEC {
		for(cs = &sleeping;; cs = &(*cs)->next) {
			if(!*cs) {
				ns->time = time - totaltime;
				ns->next = 0;
				*cs = ns;
				break;
			}
			totaltime += (*cs)->time;
			if(totaltime >= time) {
				ns->time = time - (totaltime - (*cs)->time);
				(*cs)->time -= totaltime - time;
				ns->next = *cs;
				*cs = ns;
				break;
			}
		}
		if(a) {
			ns->tag = SALARM;
			ns->u.p = curproc;
			leftcs = 1;
			LEAVECS;
		}
		ns->tag = SSLEEP;
		seminit(&ns->u.s, 1);
	}
	if(!leftcs)
		condwait(&ns->u.s);
}

void procsleep(clock_t time)
{
	sleepalarm(time, 0);
}

void procalarm(clock_t time)
{
	sleepalarm(time, 1);
}

void procusrcall(void (*)(void *, const char *), void *);
void procusrret(void);

/* not called with a normal calling convention :P */
void prochandle();

void *prochandlegut(Regs *r, const char *note)
{
	Proc *p;
	void *ns, *nsp, *notead, *regsad, *oldsp = curproc->handling;
	size_t nlen = strlen(note) + 1;
	int leftcs = 0;

	iprintk(curproc->pid);
	printk(": prochandle(");
	printk(note);
	printk(")\n");

	if(!curproc->handler || strcmp(note, "kill") == 0)
		procexits(note);
	else {
		ns = vpgumap(0, sizeof(Regs) + nlen + 0x10000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS);
		nsp = (uint8_t *) ns + 0x10000;
		notead = STACKPUSH(nsp, note, nlen);
		if(r && r->cs == 0x1b)
			regsad = STACKPUSH(nsp, r, sizeof(Regs));
		else
			regsad = 0;
		STACKPUSH(nsp, &notead, sizeof(void *));
		STACKPUSH(nsp, &regsad, sizeof(void *));
		STACKPUSH(nsp, &regsad, sizeof(void *)); /* return addr */
		procusrcall(curproc->handler, nsp);
		if(regsad && verusrptr(regsad, sizeof(Regs), PROT_READ)) {
			memcpy(r, regsad, sizeof(Regs));
			r->cs = 0x1b;
			r->eflags |= 0x202;
		}
	}

	CRITSEC {
		curproc->handling = 0;
		if(curproc->blocked) {
			p = findprev(curproc);
			p->next = curproc->next;
			leftcs = 1;
			LEAVECS;
		}
	}
	if(leftcs)
		procswitch();
	return oldsp;
}

void (*proconnotify(void (*f)(void *, const char *)))(void *, const char *)
{
	void (*oldf)(void *, const char *) = curproc->handler;
	curproc->handler = f;
	return oldf;
}

void procnoted(int v)
{
	procusrret();
	(void) v;
}

static void notify(Proc *p, const char *note)
{
	Regs r;
	void *sp, *snote;
	int leftcs = 0;
	CRITSEC {
		iprintk(curproc->pid);
		printk(": procnotify(");
		iprintk(p->pid);
		printk(", ");
		printk(note);
		printk(")\n");

		if(p->handling) {
			leftcs = 1;
			LEAVECS;
		}

		if(p == curproc) {
			leftcs = 2;
			LEAVECS;
		}

		p->handling = sp = p->sp;
		STACKPEEK(p->sp, &r, sizeof(Regs));
		snote = STACKPUSH(p->sp, note, strlen(note) + 1);
		STACKPUSH(p->sp, &snote, sizeof(void *));
		STACKPUSH(p->sp, &sp, sizeof(void *));

		r.eip = (uintptr_t) prochandle;
		r.cs = 0x08;
		r.ds = r.es = r.fs = r.gs = 0x10;
		r.eflags = 0x202;
		STACKPUSH(p->sp, &r, sizeof(Regs));

		if(p->blocked) {
			p->next = curproc->next;
			curproc->next = p;
		}
	}
	if(leftcs == 1)
		printk("notifying impossible ATM\n");
	else if(leftcs == 2)
		prochandlegut(0, note);
}

void procnotify(int pid, const char *note)
{
	Proc *p = findpid(pid);
	if(!p)
		return;
	notify(p, note);
}

