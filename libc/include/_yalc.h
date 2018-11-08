#ifndef __YALC_H
#define __YALC_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _YALC_NEED_YAX
#ifdef __YAX__
#define _YALC_NEED_YAX 1
#endif
#endif /* _YALC_NEED_YAX */

#ifndef _YALC_NEED_BSD
#ifdef _BSD_SOURCE
#define _YALC_NEED_BSD 1
#elif defined(_YALC_NEED_YAX)
#define _YALC_NEED_BSD 1
#endif
#endif /* _YALC_NEED_BSD */

#ifndef _YALC_NEED_POSIX
#if defined(_POSIX_SOURCE) || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE == 1)
#define _YALC_NEED_POSIX 1
#elif defined(_YALC_NEED_BSD)
#define _YALC_NEED_POSIX 1
#elif defined(_YALC_NEED_YAX)
#define _YALC_NEED_POSIX 1
#endif
#endif /* _YALC_NEED_POSIX */

#if __STDC_VERSION__ >= 199901L
#define __restrict restrict
#elif !defined(__GNUC__)
#define __restrict
#endif

#if __STDC_VERSION__ >= 199901L || defined(__cplusplus)
#define __inline inline
#elif !defined(__GNUC__)
#define __inline
#endif

#if __STDC_VERSION__ >= 201112L
#elif defined(__GNUC__)
#define _Noreturn __attribute__((__noreturn__))
#else
#define _Noreturn
#endif

#define _YalcERRMAX 32

#ifdef __cplusplus
#define _YalcNULL 0L
#else
#define _YalcNULL ((void *)0)
#endif

/* C11 allows repeated identical typedefs */
#define _YALCDEFTYPE(t) typedef _Yalc##t t

#define _YALCTYPESZ(t, u) typedef u _Yalc##t

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

#ifdef __cplusplus
}
#endif

#endif /* __YALC_H */

