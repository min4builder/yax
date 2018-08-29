#ifndef _SYS_INCANTATIONS_H
#define _SYS_INCANTATIONS_H

#if defined(__GNUC__) || defined(__clang__)
#define __noreturn __attribute__((noreturn))
#else /* not (gcc or clang) */
#define __noreturn
#endif /* gcc or clang */

#endif /* _SYS_INCANTATIONS_H */

