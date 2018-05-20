#ifndef _PIPE_H
#define _PIPE_H

#include <sys/types.h>
#include "conn.h"
#include "multitask.h"

#define BUFLEN 4096

typedef struct {
	Lock l;
	size_t begin, len;
	char buf[BUFLEN];
} Buf;

typedef struct Pipe Pipe;
struct Pipe {
	Conn c;
	Pipe *other;
	Buf *a, *b;
};

void pipenew(Pipe *[2]);
void pipedel(Pipe *);

ssize_t piperead(Pipe *, void *, size_t);
ssize_t pipewrite(Pipe *, const void *, size_t);
ssize_t pipestat(Pipe *, void *, size_t);
ssize_t pipewstat(Pipe *, const void *, size_t);

#endif /* _PIPE_H */

