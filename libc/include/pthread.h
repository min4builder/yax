#include <_yalc.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_PTHREAD_H)
#define _PTHREAD_H

typedef int (pthread_key_t);
typedef int (pthread_mutex_t);
typedef int (pthread_once_t);

enum {
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_ONCE_INIT,
};

#endif /* _PTHREAD_H */

#ifdef __cplusplus
}
#endif
