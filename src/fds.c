#include "conn.h"
#include "malloc.h"
#include "multitask.h"
#include "fds.h"

FdList *fdlistnew(void)
{
	FdList *fl = malloc(sizeof(FdList));
	fl->refcnt = 1;
	fl->len = 0;
	fl->list = 0;
	return fl;
}
FdList *fdlistcopy(FdList *fl)
{
	unsigned int i;
	FdList *nfl = malloc(sizeof(FdList));
	nfl->refcnt = 1;
	nfl->len = fl->len;
	nfl->list = malloc(sizeof(Conn *) * nfl->len);
	for(i = 0; i < nfl->len; i++) {
		if(fl->list[i])
			connref(fl->list[i]);
		nfl->list[i] = fl->list[i];
	}
	return nfl;
}
void fdlistref(FdList *fl)
{
	fl->refcnt++;
}
void fdlistunref(FdList *fl)
{
	unsigned int i;
	fl->refcnt--;
	if(fl->refcnt <= 0) {
		if(fl->list) {
			for(i = 0; i < fl->len; i++) {
				if(fl->list[i])
					connunref(fl->list[i]);
			}
			free(fl->list);
		}
		free(fl);
	}
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
	connunref(fl->list[fd]);
	fl->list[fd] = 0;
}

