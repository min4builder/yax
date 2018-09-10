#include <_yalc.h>

#ifndef _STDDEF_H
#define _STDDEF_H

#ifndef NULL
#define NULL _YalcNULL
#endif

#ifndef offsetof
#define offsetof _Yalcoffsetof
#endif

_YALCDEFTYPE(ptrdiff_t);
_YALCDEFTYPE(wchar_t);
_YALCDEFTYPE(size_t);

#endif /* _STDDEF_H */

