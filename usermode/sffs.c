#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <yax/errorcodes.h>
#include <yax/openflags.h>
#include <yax/stat.h>
#include "serve.h"
#include "syscall.h"

void sffsserve(int fd, char *file, size_t len)
{
	for(;;) {
		Req r = recv(fd);
		switch(r.fn) {
		case MSGDEL:
			break;
		case MSGDUP:
			r.u.dup.ret = 0;
			break;
		case MSGPREAD:
			printi(len);
			printi(r.u.rw.off);
			printi(r.u.rw.buf);
			printi(r.u.rw.len);
			if(r.u.rw.off >= len)
				r.u.rw.ret = 0;
			else {
				r.u.rw.ret = len - r.u.rw.off < r.u.rw.len ? len - r.u.rw.off : r.u.rw.len;
				memcpy(r.u.rw.buf, file + r.u.rw.off, r.u.rw.ret);
			}
			break;
		case MSGSTAT:
			exits("broken");
			break;
		case MSGWALK:
			if(!strcmp(r.u.walk.path, "file"))
				r.u.walk.ret = 0;
			else
				r.u.walk.ret = -ENOENT;
			break;
		case MSGOPEN:
			if(r.u.open.fl & (OEXCL | OTRUNC | OWRITE))
				r.u.open.ret = -EACCES;
			else
				r.u.open.ret = 0;
			break;
		case MSGPWRITE:
			r.u.rw.ret = -EACCES;
			break;
		case MSGWSTAT:
			r.u.stat.ret = -EACCES;
			break;
		}
		answer(r, fd);
	}
}

