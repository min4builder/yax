#include <_yalc.h>

#ifndef _LIMITS_H_ANSI
#define _LIMITS_H_ANSI

#define CHAR_BIT 8
#define SCHAR_MIN -128
#define SCHAR_MAX 127
#define UCHAR_MAX 255
#define MB_LEN_MAX 4
#define SHRT_MIN -0x8000
#define SHRT_MAX 0x7fff
#define USHRT_MAX 0xffff
#define INT_MIN -0x80000000
#define INT_MAX 0x7fffffff
#define UINT_MAX 0xffffffffU
#define LONG_MIN -0x80000000
#define LONG_MAX 0x7fffffffL
#define ULONG_MAX 0xffffffffUL
#define LLONG_MIN -0x8000000000000000LL
#define LLONG_MAX 0x7fffffffffffffffLL
#define ULLONG_MAX 0xffffffffffffffffULL

#if '\xff' > 0
#define CHAR_MIN 0
#define CHAR_MAX UCHAR_MAX
#else
#define CHAR_MIN SCHAR_MIN
#define CHAR_MAX SCHAR_MAX
#endif

#endif /* _LIMITS_H_ANSI */

#if !defined(_LIMITS_H_YAX) && _YALC_NEED_YAX
#define _LIMITS_H_YAX

#define PAGE_SIZE 4096

#endif /* _LIMITS_H_YAX */

