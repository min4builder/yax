#define _POSIX_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <codas/bit.h>
#include <yax/serve.h>

Req recv(int fd)
{
	char buf[28], *bufr = buf;
	Req m;
	ssize_t len = read(fd, buf, 28);
	if(len < 0)
		exit(-len);
	m.fn = GBIT32(bufr);
	bufr += 4;
	m.fid = GBIT32(bufr);
	bufr += 4;
	m.submsg = GBIT32(bufr);
	bufr += 4;
	if(m.fn & MWANTSPTR) {
		m.len = GBIT32(bufr);
		bufr += 4;
		m.buf = (void *) GBIT32(bufr);
		bufr += 4;
	}
	if(m.fn & MWANTSPTR2) {
		m.len2 = GBIT32(bufr);
		bufr += 4;
		m.buf2 = (void *) GBIT32(bufr);
		bufr += 4;
	}
	if(m.fn & MWANTSOFF) {
		m.off = GBIT64(bufr);
		bufr += 8;
	}
	return m;
}

void answer(Req m, int fd)
{
	char buf[12];
	PBIT32(buf, m.fid);
	PBIT64(buf+4, m.ret);
	write(fd, buf, 12);
	return;
}

