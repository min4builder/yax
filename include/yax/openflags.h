#ifndef _YAX_OPENFLAGS_H
#define _YAX_OPENFLAGS_H

enum openflags {
	OREAD = 1,
	O_RDONLY = 1,
	OWRITE = 2,
	O_WRONLY = 2,
	ORDWR = 3,
	O_RDWR = 3,
	OEXEC = OREAD,
	OTRUNC = 4,
	O_TRUNC = 4,
	OCEXEC = 8,
	O_CLOEXEC = 8,
	OCREAT = 16,
	O_CREAT = 16,
	OEXCL = 32,
	O_EXCL = 32
};

#endif /* _YAX_OPENFLAGS_H */

