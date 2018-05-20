#ifndef _NAME_H
#define _NAME_H

#include "mux.h"

typedef struct {
	char *name;
	Muxmaster *mm;
} Mount;

typedef struct {
	int refcnt;
	size_t len;
	struct Mount
} Mounttab;

Mounttab *mtnew(void);
Mounttab *mtcopy(Mounttab *);
void mtref(Mounttab *);
void mtunref(Mounttab *);

void mtadd(Muxmaster *, const char *);
Muxmaster *mtfind(const char *, char *);

#endif /* _NAME_H */

