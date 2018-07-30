#ifndef _YAX_STAT_H
#define _YAX_STAT_H

#include <stdint.h>
#include <sys/types.h>

typedef struct {
	uint8_t type;
	uint32_t vers;
	uint64_t path;
} Qid;

typedef struct {
	Qid qid;
	uint32_t mode;
	clock_t atime;
	clock_t mtime;
	off_t length;
	char *name;
	char *uid;
	char *gid;
	char *muid;
} Dir;

#endif /* _YAX_STAT_H */

