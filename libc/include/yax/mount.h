#ifndef _YAX_MOUNT_H
#define _YAX_MOUNT_H

#include <yax/mountflags.h>
#include <yax/stat.h>

int mkmnt(int *, Qid);
int mount(const char *, int, int);
int __getprintk(void);

#endif /* _YAX_MOUNT_H */

