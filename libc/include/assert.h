#include <_yalc.h>

#ifdef assert
#undef assert
#endif

#ifndef NDEBUG
#define assert(x) _Yalcassert((x), __func__, " at "__FILE__":"__LINE__ #x)
#else
#define assert(x) ((void)0)
#endif

#ifndef static_assert
#define static_assert _Static_assert
#endif

