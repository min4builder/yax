#ifndef _LOCK_H
#define _LOCK_H

#include "macro.h"

/* atomic spinlocks */
typedef int Lock;

void locklock(Lock *);
void lockunlock(Lock *);

#define ATOMIC(l) MACRO_WRAP_(Lock *MACRO_LABEL_(ATOMIC, 1) = (l); locklock(MACRO_LABEL_(ATOMIC, 1)), lockunlock(MACRO_LABEL_(ATOMIC, 1)))

#endif /* _LOCK_H */

