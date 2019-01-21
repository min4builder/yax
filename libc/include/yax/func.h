#ifndef __YAX__
#error "Declare your use of __YAX__ before using YAX-specific headers"
#endif

#ifndef YAX_FUNC_H_
#define YAX_FUNC_H_

#include <sys/types.h>
#include <yax/fn.h>

long long func(int, int, int, void *, size_t, void *, size_t, off_t);

#endif /* YAX_FUNC_H_ */

