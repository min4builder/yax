#ifndef __YAX__
#error "Declare your use of __YAX__ before using YAX-specific headers"
#endif

#ifndef _YAX_LOCK_H
#define _YAX_LOCK_H

#include <codas/macro.h>

/* atomic spinlocks */
typedef int Lock;

void locklock(Lock *);
void lockunlock(Lock *);

#define ATOMIC(l) MACRO_WRAP_(Lock *MACRO_LABEL_(ATOMIC, 1) = (l); locklock(MACRO_LABEL_(ATOMIC, 1)), lockunlock(MACRO_LABEL_(ATOMIC, 1)))

#endif /* _YAX_LOCK_H */

