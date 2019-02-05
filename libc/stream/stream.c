#define _POSIX_SOURCE
#include <stdio.h>
#include <unistd.h>
#include "stream.h"

static char inbuf[BUFSIZ], outbuf[BUFSIZ], errbuf[BUFSIZ];
static FILE _stdin = {
	.read = read,
	.rbuf = inbuf,
	.rbuflen = sizeof(inbuf),
	.rpos = inbuf,
	.rend = inbuf,
	.fd = 0
}, _stdout = {
	.write = write,
	.wbuf = outbuf,
	.wbuflen = sizeof(outbuf),
	.wstart = outbuf,
	.wpos = outbuf,
	.fd = 1
}, _stderr = {
	.write = write,
	.wbuf = errbuf,
	.wbuflen = sizeof(errbuf),
	.wstart = errbuf,
	.wpos = errbuf,
	.fd = 2
};

FILE *const stdin = &_stdin, *const stdout = &_stdout, *const stderr = &_stderr;

