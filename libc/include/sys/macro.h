#include <_yalc.h>

#if !defined(_SYS_MACRO_H) && defined(_YALC_NEED_YAX)
#define _SYS_MACRO_H

/*
I wish I could do this:
#define SOMETHING(l) { block }
	{
		somethingon(l);
		block;
		somethingoff(l);
	}
*/
/* Here is the solution */
#define MACRO_TOKPASTE2_(a, b) a ## b
#define MACRO_TOKPASTE_(a, b) MACRO_TOKPASTE2_(a, b)
#define MACRO_LABEL_(l, n) MACRO_TOKPASTE_(MACRO_##l##_, MACRO_TOKPASTE_(__LINE__, _##n##_))
#define MACRO_WRAP_(fb, fa) if(1) { \
	fb; \
	goto MACRO_LABEL_(WRAP, 1); \
	MACRO_LABEL_(WRAP, 2): \
	fa; \
} else \
	while(1) \
		if(1) goto MACRO_LABEL_(WRAP, 2); \
		else MACRO_LABEL_(WRAP, 1):

#endif /* _SYS_MACRO_H */

