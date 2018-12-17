#ifndef _YAX_STAT_H
#define _YAX_STAT_H

#include <stdint.h>
#include <sys/types.h>
#include <codas/macro.h>
#include <yax/stat.h>

#if UINTPTR_MAX == 0xffffffff
#define GBITPTR(b) ((void *) GBIT32(b))
#define PBITPTR(b, i) PBIT32(b, (uintptr_t) (i))
#elif UINTPTR_MAX == 0xffffffffffffffff
#define GBITPTR(b) ((void *) GBIT64(b))
#define PBITPTR(b, i) PBIT64(b, (uintptr_t) (i))
#else
#error "Unknown bitness of system"
#endif /* bitness */

typedef struct {
	uint8_t type;
	uint32_t vers;
	uint64_t path;
} Qid;

enum {
	QTDIR = 1 << 7,
	QTTMP = 1 << 6,
	QTAPPEND = 1 << 5,
	QTFILE = 0
};

typedef struct {
	Qid qid;
	uint32_t mode;
	clock_t atime;
	clock_t mtime;
	off_t length;
	char const *name;
	char const *uid;
	char const *gid;
	char const *muid;
} Dir;

enum {
	DMDIR = 1 << 31,
	DMTMP = 1 << 30,
	DMAPPEND = 1 << 29,
	DMFILE = 0
};

size_t convD2M(Dir *, char *, size_t);
size_t convM2D(char *, size_t, Dir *, char *);

#endif /* _YAX_STAT_H */

