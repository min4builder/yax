#ifndef _LOCK_H
#define _LOCK_H

typedef int Lock;

void locklock(Lock *);
void lockunlock(Lock *);

/*
I wish I could do this:
#define ATOMIC(l) { block }
	{
		locklock(l);
		block;
		lockunlock(l);
	}
*/
/* sorry */
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
#define MACRO_DECLARE_(v) for(v;;) \
	if(1) { \
		goto MACRO_LABEL_(DECLARE, 1); \
		MACRO_LABEL_(DECLARE, 2): \
		break; \
	} else \
		while(1) \
			if(1) goto MACRO_LABEL_(DECLARE, 2); \
			else MACRO_LABEL_(DECLARE, 1):
#define ATOMIC(l) MACRO_WRAP_(locklock(l), lockunlock(l))

#endif /* _LOCK_H */

