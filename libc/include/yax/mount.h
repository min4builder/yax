#ifndef __YAX__
#error "Declare your use of __YAX__ before using YAX-specific headers"
#endif

#ifndef YAX_MOUNT_H_
#define YAX_MOUNT_H_

#include <sys/stat.h>
#include <yax/mountflags.h>

int mkmnt(int *, ino_t);
int mount(const char *, int, int);
int __getprintk(void);

#endif /* YAX_MOUNT_H_ */

