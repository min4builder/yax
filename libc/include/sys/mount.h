#ifndef _SYS_MOUNT_H
#define _SYS_MOUNT_H

#include <yax/mountflags.h>

int mkmnt(int *);
int __mountfd(const char *, int, int);

#endif /* _SYS_MOUNT_H */
