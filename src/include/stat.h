#ifndef _YAX_STAT_H
#define _YAX_STAT_H

#include <stdint.h>
#include <sys/types.h>

typedef struct {
	uint8_t type;
	uint32_t vers;
	uint64_t path;
} Qid;

enum {
	QTDIR = 1 << 7,
	QTAUTH = 1 << 6,
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
	DMAUTH = 1 << 30,
	DMFILE = 0
};

#endif /* _YAX_STAT_H */

