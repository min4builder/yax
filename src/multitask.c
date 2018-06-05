#include <sys/types.h>
#include <yax/const.h>
#include <yax/rfflags.h>
#include "arch.h"
#include "boot.h"
#include "int.h"
#include "libk.h"
#include "lock.h"
#include "malloc.h"
#include "pgdir.h"
#include "printk.h"
#include "multitask.h"

typedef struct Sleep Sleep;
struct Sleep {
	Sleep *next;
	clock_t time;
	enum { SSLEEP, SALARM } tag;
	union {
		Condition c;
		Proc *p;
	} u;
};

static void notify(Proc *, const char *);

Proc *curproc = 0;
static Lock proclock;
static Proc *procbypid = 0;
static Proc *nullproc = 0;
static Sleep *sleeping = 0;

#define CRITSEC MACRO_DECLARE_(int ints_were_enabled_) MACRO_WRAP_(ints_were_enabled_ = entercs(), leavecs(ints_were_enabled_))
#define LEAVECS leavecs(ints_were_enabled_)

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
	nullproc = calloc(1, sizeof(Proc));
	nullproc->pid = 0;
	nullproc->pd = getpgdir();
	nullproc->quantum = 0;
	nullproc->handler = 0;
	nullproc->handling = 0;
	nullproc->syscallstack = kernel_stack_bottom;
	nullproc->kstack = kernel_stack_low;
	nullproc->next = nullproc;
	nullproc->parent = nullproc;
	nullproc->priority = 5;
	nullproc->fds = fdlistnew();
	curproc = procbypid = nullproc;
}

void timer_handler(void)
{
	Sleep *os;
	if(sleeping) {
		sleeping->time--;
		while(sleeping && sleeping->time <= 0) {
			if(sleeping->tag == SSLEEP)
				condsignal(&sleeping->u.c);
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
		printk("procswitch()\n");
		curproc->quantum = curproc->priority;
		curproc->sp = sp;
		prev = curproc;
		curproc = curproc->next;
		/* if the next one is the null task, skip it;
		 * if *that* one is also the null task, we've got nothing else to do,
		 * therefore, we run the null task anyways. */
		if(curproc == nullproc) {
			curproc = curproc->next;
		}
		prev->pd = switchpgdir(curproc->pd);
		prev->syscallstack = switchsyscallstack(curproc->syscallstack);
	}
	return curproc->sp;
}

void condwait(Condition *c)
{
	Proc *p;
	CRITSEC {
		if(c->waiter) {
			printk("DOUBLE WAIT: ");
			iprintk(c->waiter->pid);
			cprintk(' ');
			iprintk(curproc->pid);
			cprintk('\n');
			halt();
		}
		if(c->done) {
			c->done = 0;
			LEAVECS;
			return;
		}
		c->waiter = curproc;
		if(!curproc->blocked && !curproc->blocked2)
			curproc->blocked = c;
		else if(!curproc->blocked2)
			curproc->blocked2 = c;
		else {
			printk("TRIPLE BLOCK: ");
			iprintk(curproc->pid);
			cprintk('\n');
			halt();
		}
		p = findprev(curproc);
		p->next = curproc->next;
	}
	procswitch();
}

void condsignal(Condition *c)
{
	CRITSEC {
		if(c->done) {
			printk("DOUBLE SIGNAL: ");
			iprintk(curproc->pid);
			cprintk('\n');
			halt();
		}
		if(c->waiter) {
			if(c->waiter->blocked != c && c->waiter->blocked2 != c) {
				printk("BLOCKING INCONSISTENCY: ");
				iprintk(c->waiter->pid);
				cprintk(' ');
				uxprintk((uintptr_t) c->waiter->blocked);
				cprintk('\n');
				halt();
			}
			if(!(c->waiter->blocked == c && c->waiter->blocked2 != 0)) {
				c->waiter->next = curproc->next;
				curproc->next = c->waiter;
			}
			if(c->waiter->blocked == c)
				c->waiter->blocked = 0;
			else if(c->waiter->blocked2 == c)
				c->waiter->blocked2 = 0;
			c->waiter = 0;
		} else {
			c->done = 1;
		}
	}
}

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
	new->kstack = vpgmap(0, 0x10000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, 0);
	new->syscallstack = (uint8_t *) new->kstack + 0x10000;
	if(curproc->handling)
		new->handling = (uint8_t *) curproc->handling - (uint8_t *) curproc->syscallstack + (uint8_t *) new->syscallstack;
	else
		new->handling = 0;
	new->handler = curproc->handler;
	new->sp = (uint8_t *) sp - (uint8_t *) curproc->syscallstack + (uint8_t *) new->syscallstack;
	memcpy(new->sp, sp, (uint8_t *) curproc->syscallstack - (uint8_t *) sp);
	RFORKREGADJUST(new->sp, curproc->syscallstack, new->syscallstack);

	CRITSEC {
		if(fl & RFFDG)
			new->fds = fdlistcopy(curproc->fds);
		else {
			fdlistref(curproc->fds);
			new->fds = curproc->fds;
		}

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
	fdlistunref(curproc->fds);
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
	new->kstack = vpgmap(0, 0x1000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, 0);
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
			LEAVECS;
			return;
		}
		ns->tag = SSLEEP;
	}
	condwait(&ns->u.c);
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

	iprintk(curproc->pid);
	printk(": prochandle(");
	printk(note);
	printk(")\n");

	if(!curproc->handler || strcmp(note, "kill") == 0)
		procexits(note);
	else {
		ns = vpgmap(0, sizeof(Regs) + nlen + 0x10000, PROT_READ | PROT_WRITE | PROT_USER, MAP_ANONYMOUS, 0);
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
			LEAVECS;
			procswitch();
			continue;
		}
	}
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
}

static void notify(Proc *p, const char *note)
{
	Regs r;
	void *sp, *snote;
	CRITSEC {
		iprintk(curproc->pid);
		printk(": procnotify(");
		iprintk(p->pid);
		printk(", ");
		printk(note);
		printk(")\n");

		if(p->handling) {
			LEAVECS;
			printk("notifying impossible ATM\n");
			return;
		}

		if(p == curproc) {
			LEAVECS;
			prochandlegut(0, note);
			return;
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
}

void procnotify(int pid, const char *note)
{
	Proc *p = findpid(pid);
	if(!p)
		return;
	notify(p, note);
}

