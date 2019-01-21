#ifndef _FCNTL_H
#define _FCNTL_H

#include <yax/openflags.h>

#define O_NOCTTY 0

#define AT_FDCWD -4
#define AT_EACCESS 0
#define AT_SYMLINK_NOFOLLOW 0
#define AT_SYMLINK_FOLLOW 0

int open(const char *, int, ...);

#endif /* _FCNTL_H */

