#define __YAX__
#include <codas/ref.h>
#include "conn.h"
#include "mem/malloc.h"
#include "multitask.h"
#include "fds.h"

static void ffree(const RefCounted *rc)
{
	unsigned int i;
	FdList *fl = (FdList *) rc;
	if(fl->list) {
		for(i = 0; i < fl->len; i++) {
			if(fl->list[i])
				unref(fl->list[i]);
		}
		free(fl->list);
	}
	free(fl);
}
FdList *fdlistnew(void)
{
	FdList *fl = malloc(sizeof(FdList));
	mkref(fl, ffree);
	fl->len = 0;
	fl->list = 0;
	return fl;
}
FdList *fdlistcopy(FdList *fl)
{
	unsigned int i;
	FdList *nfl = malloc(sizeof(FdList));
	mkref(nfl, ffree);
	nfl->len = fl->len;
	nfl->list = malloc(sizeof(Conn *) * nfl->len);
	for(i = 0; i < nfl->len; i++) {
		if(fl->list[i])
			ref(fl->list[i]);
		nfl->list[i] = fl->list[i];
	}
	return nfl;
}

int fdalloc(Conn *c)
{
	unsigned int i;
	FdList *fl = curproc->fds;
	for(i = 0; i < fl->len; i++) {
		if(!fl->list[i]) {
			fl->list[i] = c;
			return i;
		}
	}
	fl->len++;
	fl->list = realloc(fl->list, fl->len * sizeof(Conn *));
	fl->list[i] = c;
	return i;
}

void fddealloc(int fd)
{
	FdList *fl = curproc->fds;
	if((unsigned int) fd >= fl->len || !fl->list[fd])
		return;
	unref(fl->list[fd]);
	fl->list[fd] = 0;
}

