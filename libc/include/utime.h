#include <_yalc.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_UTIME_H) && defined(_YALC_NEED_POSIX)
#define _UTIME_H

int utime(char const *, struct utimbuf const *);

#endif /* _UTIME_H */

#ifdef __cplusplus
}
#endif

