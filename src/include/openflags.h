#ifndef _YAX_OPENFLAGS_H
#define _YAX_OPENFLAGS_H

enum openflags {
	O_RDONLY = 1,
	O_WRONLY = 2,
	O_RDWR = O_RDONLY | O_WRONLY,
	O_EXEC = O_RDONLY,
	O_TRUNC = 4,
	O_CLOEXEC = 8,
	O_CREAT = 16,
	O_EXCL = 32,
	O_PATH = 64
};

#endif /* _YAX_OPENFLAGS_H */

