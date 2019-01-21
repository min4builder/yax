#include <_yalc.h>

#if !defined(_DIRENT_H) && defined(_YALC_NEED_POSIX)
#define _DIRENT_H

_YALCDEFTYPE(ino_t);

typedef struct {
	int fd;
} DIR;

struct dirent {
	ino_t d_ino;
	char d_name[];
};

int closedir(DIR *);
int dirfd(DIR *);
DIR *fdopendir(int);
DIR *opendir(char const *);
struct dirent *readdir(DIR *);
void rewinddir(DIR *);

#endif /* _DIRENT_H */

