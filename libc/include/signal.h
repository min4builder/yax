#include <_yalc.h>

#ifndef _SIGNAL_H_ANSI
#define _SIGNAL_H_ANSI

#define SIG_DFL _YALCSIG_DFL
#define SIG_ERR _YALCSIG_ERR
#define SIG_IGN _YALCSIG_IGN

_YALCDEFTYPE(sig_atomic_t);

void (*signal(int, void (*)(int)))(int);
int raise(int);

#endif /* _SIGNAL_H_ANSI */

#if !defined(_SIGNAL_H_POSIX) && _YALC_NEED_POSIX
#define _SIGNAL_H_POSIX

_YALCDEFTYPE(size_t);
_YALCDEFTYPE(uid_t);
_YALCDEFTYPE(pid_t);

#endif /* _SIGNAL_H_POSIX */

