#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <yax/bit.h>
#include <yax/serve.h>
#include <yax/stat.h>

Req recv(int fd)
{
	char buf[21];
	Req m;
	read(fd, buf, 21);
	m.fn = buf[0];
	m.fid = GBIT32(buf+1);
	switch(m.fn) {
	case MSGDEL:
		break;
	case MSGDUP:
		break;
	case MSGPREAD:
	case MSGPWRITE:
		m.u.rw.len = GBIT32(buf+5);
		m.u.rw.buf = (void *) GBIT32(buf+9);
		m.u.rw.off = GBIT64(buf+13);
		break;
	case MSGSTAT:
		exits("Stat");
	case MSGWSTAT:
		exits("Wstat");
	case MSGWALK: {
		size_t len = GBIT32(buf+5);
		m.u.walk.path = malloc(len + 1);
		if(len + 9 <= 21)
			memcpy(m.u.walk.path, buf + 9, len);
		else
			read(fd, m.u.walk.path, len);
		m.u.walk.path[len] = '\0';
		break;
	}
	case MSGOPEN:
		m.u.open.fl = GBIT32(buf+5);
		m.u.open.mode = GBIT32(buf+9);
		break;
	}
	return m;
}

void answer(Req m, int fd)
{
	char buf[8];
	PBIT32(buf, m.fid);
	switch(m.fn) {
	case MSGDEL:
		write(fd, buf, 4);
		return;
	case MSGDUP:
		PBIT32(buf+4, m.u.dup.ret);
		break;
	case MSGPREAD:
	case MSGPWRITE:
		PBIT32(buf+4, m.u.rw.ret);
		break;
	case MSGSTAT:
	case MSGWSTAT:
		exits("nonsense");
	case MSGWALK:
		free(m.u.walk.path);
		PBIT32(buf+4, m.u.walk.ret);
		break;
	case MSGOPEN:
		PBIT32(buf+4, m.u.open.ret);
		break;
	}
	write(fd, buf, 8);
}

