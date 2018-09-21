#include <_yalc.h>

#if !defined(_SYS_LOCK_H) && defined(_YALC_NEED_YAX)
#define _SYS_LOCK_H

#include <sys/macro.h>

/* atomic spinlocks */
typedef int Lock;

void locklock(Lock *);
void lockunlock(Lock *);

#define ATOMIC(l) MACRO_WRAP_(Lock *MACRO_LABEL_(ATOMIC, 1) = (l); locklock(MACRO_LABEL_(ATOMIC, 1)), lockunlock(MACRO_LABEL_(ATOMIC, 1)))

#endif /* _SYS_LOCK_H */

