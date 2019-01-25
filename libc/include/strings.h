#include <_yalc.h>

#if !defined(_STRINGS_H) && defined(_YALC_NEED_POSIX)
#define _STRINGS_H

int strcasecmp(char const *, char const *);
int strncasecmp(char const *, char const *, size_t);

#endif /* _STRINGS_H */

