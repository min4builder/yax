#define NDEBUG
#define __YAX__
#include <string.h>
#include <codas/ref.h>
#include <yax/mountflags.h>
#include <yax/openflags.h>
#include "fs/fds.h"
#include "fs/name.h"
#include "mem/malloc.h"
#include "multitask.h"
#include "printk.h"

static void mfree(const RefCounted *rc)
{
	MountTab *mt = (MountTab *) rc;
	Mount *m, *n;
	for(m = mt->first; m;) {
		if(m->from)
			unref(m->from);
		unref(m->to);
		n = m;
		m = m->next;
		free(n);
	}
}
MountTab *mountnew(void)
{
	MountTab *mt = calloc(1, sizeof(*mt));
	mkref(mt, mfree);
	mt->first = 0;
	return mt;
}
MountTab *mountcopy(MountTab *mt)
{
	MountTab *nmt = mountnew();
	Mount *m = mt->first, **n = &nmt->first;
	while(m) {
		*n = calloc(1, sizeof(**n));
		(*n)->from = m->from;
		if(m->from)
			ref(m->from);
		(*n)->to = m->to;
		ref(m->to);
		n = &(*n)->next;
		m = m->next;
	}
	return nmt;
}

char *nameclean(const char *name)
{
	char *newname, *curn;
	if(*name == '/') {
		curn = newname = malloc(strlen(name)+1);
		*curn++ = *name++;
	} else {
		int len = strlen(curproc->cwd->s);
		curn = newname = malloc(strlen(name) + len + 2);
		memcpy(newname, curproc->cwd->s, len);
		newname[len] = '/';
		curn += len + 1;
	}
	while(*name) {
		if(name[0] == '/' || (name[0] == '.' && (name[1] == '/' || !name[1]))) {
			while(*name && *name != '/')
				name++;
			if(*name)
				name++;
		} else if(name[0] == '.' && name[1] == '.' && (name[2] == '/' || !name[2])) {
			while(*name && *name != '/')
				name++;
			if(*name)
				name++;
			curn -= 2;
			while(curn > newname && *curn != '/')
				curn--;
			curn++;
			if(curn <= newname)
				curn++;
		} else {
			while(*name && *name != '/')
				*curn++ = *name++;
			if(*name)
				*curn++ = *name++;
		}
	}
	*curn++ = '\0';
	return newname;
}

static int qideq(Conn *a, Conn *b)
{
	return a == b || (a && b && a->dev == b->dev && a->inst == b->inst
		&& a->ino == b->ino);
}

Conn *vfsgetfid(const char *name, int nolastwalk)
{
	Mount *mount = 0;
	Conn *m = 0, *prevm = 0;
	char *newname = nameclean(name);
	const char *tmp;
	Str *sname = strmk(newname);
	long long err;
	name = newname + 1; /* skip initial / */
	printk("[getfid ");
	printk(newname);
	printk("]");
	do {
		if(!mount) /* at the beginning or last walk succeeded */
			mount = curproc->mounttab->first;
		while(mount) {
			if(qideq(m, mount->from)) {
				if(prevm)
					unref(prevm);
				prevm = m;
				m = conndup(mount->to, sname);
				mount = mount->next; /* in case of an union */
				break;
			}
			mount = mount->next;
		}
		if(PTRERR(m)) {
			err = PTR2ERR(m);
			goto bail;
		}
		tmp = name;
		while(*tmp && *tmp != '/') tmp++;
		if(m && name[0] && !nolastwalk && (err = connfunc(m, MWALK, 0, (char *) name, tmp - name, 0)) < 0) {
			if(!mount)
				goto bailclose;
			unref(m);
			m = prevm;
			prevm = 0;
			continue; /* might be a union mount; retry */
		} else {
			if(prevm)
				unref(prevm);
			prevm = 0;
			mount = 0;
		}
		name = tmp;
		if(name[0])
			name++;
	} while(name[0]);
	if(prevm)
		unref(prevm);
	unref(sname);
	return m;
bailclose:
	unref(m);
bail:
	if(prevm)
		unref(prevm);
	unref(sname);
	return ERR2PTR((int) err);
}

Conn *vfsopen(const char *name, enum openflags fl, int mode)
{
	Conn *c;
	int err;
	printk("[find ");
	printk(name);
	printk(" = ");
	c = vfsgetfid(name, !!(fl & O_CREAT));
	uxprintk((uintptr_t) c);
	printk("]\n");
	if(PTRERR(c) || (fl & O_PATH))
		return c;
	if((err = connfunc(c, MOPEN, fl | (mode << 16), 0, 0, 0)) < 0) {
		return ERR2PTR(err);
	}
	return c;
}

int vfspoll(int *fds, size_t nfds, clock_t timeout);

int vfsmount(const char *name, Conn *newc, enum mountflags fl)
{
	Mount **m, *n = calloc(1, sizeof(Mount));
	Conn *c = vfsgetfid(name, 0);
	if(PTRERR(c)) {
		free(n);
		return PTR2ERR(c);
	}
	n->next = 0;
	n->from = c;
	ref(newc);
	n->to = newc;
	for(m = &curproc->mounttab->first; *m; m = &(*m)->next) {
		if(qideq((*m)->to, n->from)) {
			unref(n->from);
			if(fl == MREPL) {
				free(n);
				unref((*m)->to);
				(*m)->to = newc;
				return 0;
			} else {
				n->from = (*m)->from;
				if(n->from)
					ref(n->from);
				if(fl == MAFTER) {
					while(*m && qideq((*m)->from, n->from))
						m = &(*m)->next;
				}
				if(*m)
					n->next = (*m)->next;
				*m = n;
				return 0;
			}
		}
	}
	n->next = curproc->mounttab->first;
	curproc->mounttab->first = n;
	(void) fl;
	return 0;
}

int vfschdir(const char *name)
{
	char *newname = nameclean(name);
	unref(curproc->cwd);
	curproc->cwd = strmk(newname);
	return 0;
}

