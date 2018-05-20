#ifndef _LOCK_H
#define _LOCK_H

typedef int Lock;

void locklock(Lock *);
void lockunlock(Lock *);

#endif /* _LOCK_H */

