#define _POSIX_SOURCE
#include <stdio.h>
#include "stream.h"

static char inbuf[BUFSIZ], outbuf[BUFSIZ], errbuf[BUFSIZ];
static FILE _stdin = {
	.rbuf = inbuf,
	.rbuflen = sizeof(inbuf),
	.rpos = inbuf,
	.rend = inbuf,
	.fd = 0
}, _stdout = {
	.wbuf = outbuf,
	.wbuflen = sizeof(outbuf),
	.wstart = outbuf,
	.wpos = outbuf,
	.fd = 1
}, _stderr = {
	.wbuf = errbuf,
	.wbuflen = sizeof(errbuf),
	.wstart = errbuf,
	.wpos = errbuf,
	.fd = 2
};

FILE *const stdin = &_stdin, *const stdout = &_stdout, *const stderr = &_stderr;

