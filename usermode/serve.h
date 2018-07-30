#ifndef _SERVE_H
#define _SERVE_H

#include <sys/types.h>

typedef struct {
	enum { MSGDEL, MSGDUP, MSGPREAD, MSGPWRITE, MSGSTAT, MSGWSTAT, MSGWALK, MSGOPEN } fn;
	int fid;
	union {
		struct {
			int ret;
		} dup;
		struct {
			void *buf;
			size_t len;
			off_t off;
			ssize_t ret;
		} rw;
		struct {
			void *buf;
			size_t len;
			ssize_t ret;
		} stat;
		struct {
			char *path;
			int ret;
		} walk;
		struct {
			int fl;
			int mode;
			int ret;
		} open;
	} u;
} Req;

Req recv(int fd);
void answer(Req m, int fd);

#endif /* _SERVE_H */

