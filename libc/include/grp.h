#include <_yalc.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_GRP_H) && defined(_YALC_NEED_POSIX)
#define _GRP_H

_YALCDEFTYPE(gid_t);
_YALCDEFTYPE(size_t);

struct group {
	char *gr_name;
	gid_t gr_gid;
	char **gr_mem;
};

struct group *getgrnam(char const *);

#endif /* _GRP_H */

#ifdef __cplusplus
}
#endif

