#ifndef _NAME_H
#define _NAME_H

#include <codas/ref.h>
#include <yax/mountflags.h>
#include <yax/openflags.h>
#include "conn.h"

typedef struct Mount Mount;
struct Mount {
	Conn *from, *to;
	Mount *next;
};

typedef struct {
	RefCounted refcounted;
	Mount *first;
} MountTab;

MountTab *mountnew(void);
MountTab *mountcopy(MountTab *);
char *nameclean(const char *);
Conn *vfsgetfid(const char *, int nolastwalk);
Conn *vfsopen(const char *, enum openflags fl, int mode);
int vfsmount(const char *, Conn *, enum mountflags fl);
int vfschdir(const char *);

#endif /* _NAME_H */

