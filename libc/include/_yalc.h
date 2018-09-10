#ifndef __YALC_H
#define __YALC_H

#ifdef __YAX__
#define _YALC_NEED_YAX 1
#define _YALC_NEED_POSIX 1
#endif

/* C11 allows repeated identical typedefs */
#define _YALCDEFTYPE(t) typedef _Yalc##t t

#define _YALCTYPESZ(t, u) typedef u _Yalc##t

#define _YalcERRMAX 32
#define _YalcNULL ((void *)0)

void _Yalcassert(int, const char *, const char *);

_YALCTYPESZ(blkcnt_t, long long);
_YALCTYPESZ(blksize_t, long long);
_YALCTYPESZ(clock_t, long long);
_YALCTYPESZ(gid_t, long);
_YALCTYPESZ(id_t, long);
_YALCTYPESZ(mode_t, int);
_YALCTYPESZ(off_t, long long);
_YALCTYPESZ(pid_t, long);
_YALCTYPESZ(ptrdiff_t, long);
_YALCTYPESZ(size_t, unsigned long);
_YALCTYPESZ(ssize_t, long);
_YALCTYPESZ(time_t, unsigned long long);
_YALCTYPESZ(uid_t, long);
_YALCTYPESZ(wchar_t, unsigned long);

#endif /* __YALCALLFN_H */

