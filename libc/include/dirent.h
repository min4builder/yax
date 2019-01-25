#include <_yalc.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_DIRENT_H) && defined(_YALC_NEED_POSIX)
#define _DIRENT_H

_YALCDEFTYPE(ino_t);

struct dirent {
	ino_t d_ino;
	char d_name[];
};

typedef struct {
	int fd;
	struct dirent *cur;
} DIR;

int closedir(DIR *);
int dirfd(DIR *);
DIR *fdopendir(int);
DIR *opendir(char const *);
struct dirent *readdir(DIR *);
void rewinddir(DIR *);

#endif /* _DIRENT_H */

#ifdef __cplusplus
}
#endif

