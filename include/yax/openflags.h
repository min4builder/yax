#ifndef _YAX_OPENFLAGS_H
#define _YAX_OPENFLAGS_H

enum openflags {
	OREAD = 1,
	OWRITE = 2,
	ORDWR = 3,
	OEXEC = OREAD,
	OTRUNC = 4,
	OCEXEC = 8,
	OCREAT = 16,
	OEXCL = 32
};

#endif /* _YAX_OPENFLAGS_H */

