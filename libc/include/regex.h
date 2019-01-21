#include <_yalc.h>

#if !defined(_REGEX_H) && defined(_YALC_NEED_POSIX)
#define _REGEX_H

_YALCDEFTYPE(size_t);
_YALCDEFTYPE(regoff_t);

typedef struct {
	size_t re_nsub;
} regex_t;
typedef struct {
	regoff_t rm_so, rm_eo;
} regmatch_t;

int regcomp(regex_t *__restrict, char const *__restrict, int);
int regexec(regex_t const *__restrict, char const *__restrict, size_t, regmatch_t *__restrict, int);
size_t regerror(int, regex_t const *__restrict, char *__restrict, size_t);
void regfree(regex_t *);

#endif /* _REGEX_H */

