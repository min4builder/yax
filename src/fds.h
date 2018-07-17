#ifndef _FDS_H
#define _FDS_H

#include <sys/types.h>
#include "conn.h"
#include "ref.h"

typedef struct {
	RefCounted refcounted;
	size_t len;
	Conn **list;
} FdList;

FdList *fdlistnew(void);
FdList *fdlistcopy(FdList *);

int fdalloc(Conn *); 
void fddealloc(int);

#define FD2CONN(fd) ((unsigned int) (fd) >= curproc->fds->len ? (Conn *) 0 : curproc->fds->list[fd])
#define FDSET(fd, c) ((unsigned int) (fd) >= curproc->fds->len ? -1 : (fddealloc(fd), curproc->fds->list[fd] = (c), (fd)))

#endif /* _FDS_H */

