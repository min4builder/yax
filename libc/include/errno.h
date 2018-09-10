#include <_yalc.h>

#ifndef _ERRNO_H_ANSI
#define _ERRNO_H_ANSI

#include <yax/errorcodes.h>

extern int errno;

#endif /* _ERRNO_H_ANSI */

#if !defined(_ERRNO_H_YAX) && _YALC_NEED_YAX
#define _ERRNO_H_YAX

#define ERRMAX _YalcERRMAX

#endif /* _ERRNO_H_YAX */

