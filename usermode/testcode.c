#include <stdint.h>
#include <sys/types.h>
#include <yax/errorcodes.h>
#include <yax/mapflags.h>
#include <yax/mountflags.h>
#include <yax/openflags.h>
#include <yax/rfflags.h>
#include <yax/stat.h>
#include "syscall.h"

void exits(const char *);

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

static int readc(int fd)
{
	int err;
	char c;
	if((err = read(fd, &c, 1)) <= 0)
		return err;
	return c;
}

static int writec(int fd, char c)
{
	int err;
	if((err = write(fd, &c, 1)) < 1)
		return err;
	return 0;
}

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
		//free(m.u.walk.path);
		PBIT32(buf+4, m.u.walk.ret);
		break;
	case MSGOPEN:
		PBIT32(buf+4, m.u.open.ret);
		break;
	}
	write(fd, buf, 8);
}

void _libc_main(char *argv, char *envp)
{
	int fd, mnt;

	fd = mkmnt(&mnt);
	__mountfd("/", fd, MAFTER);
	close(fd);

	if(rfork(RFPROC|RFFDG)) {
		int err;
		char buf[32];
		fd = open("/cons", ORDWR);
		write(fd, "aoeui", 5);
		/*while((err = read(fd, buf, 32)) > 0)
			write(fd, buf, err);*/
	} else {
		off_t delta = 0;
		int a;
		char *vga = mmap(0, 80 * 25 * 2, PROT_READ | PROT_WRITE, MAP_PHYS, 0, 0xB8000);
		int p64 = open("/port/0064", ORDWR);
		int p3d4 = open("/port/03d4", ORDWR);
		int p3d5 = open("/port/03d5", ORDWR);
		fd = open("/port/0060", ORDWR);
		for(a = 0; a < 80 * 25; a++) {
			vga[a*2] = ' ';
			vga[a*2+1] = 0x07;
		}
		writec(p3d4, 0x0A);
		writec(p3d5, (readc(p3d5) & 0xC0) | 0);
		writec(p3d4, 0x0B);
		writec(p3d5, (readc(p3d5) & 0xE0) | 15);
		for(;;) {
			Req r = recv(mnt);
			switch(r.fn) {
			case MSGDUP:
				r.u.dup.ret = 1;
				break;
			case MSGOPEN:
				if(r.u.open.fl & OEXCL)
					r.u.open.ret = -1;
				else
					r.u.open.ret = 0;
				break;
			case MSGPWRITE: {
				int b;
				for(a = (int) (r.u.rw.off + delta) % (80 * 25), b = 0; a < ((int) (r.u.rw.off + delta) + r.u.rw.len) % (80 * 25); a++, b++) {
					char c = ((char *)r.u.rw.buf)[b];
					if(c == '\n') {
						/* TODO */
					} else {
						vga[a*2] = ((char *)r.u.rw.buf)[b];
						vga[a*2+1] = 0x07;
					}
				}
				writec(p3d4, 0x0F);
				writec(p3d5, (uint8_t) (((int) (r.u.rw.off + delta + r.u.rw.len) % (80 * 25)) & 0xFF));
				writec(p3d4, 0x0E);
				writec(p3d5, (uint8_t) (((int) (r.u.rw.off + delta + r.u.rw.len) % (80 * 25)) >> 8));
				break;
			}
			case MSGPREAD: {
				/* IMPLEMENT KEYBOARD INPUT */
				while(!(readc(p64) & 1)) {}
				((char *)r.u.rw.buf)[0] = readc(fd);
				r.u.rw.ret = 1;
				break;
			}
			case MSGWALK:
				if(strcmp(r.u.walk.path, "cons") != 0)
					r.u.walk.ret = -ENOENT;
				else
					r.u.walk.ret = 0;
				break;
			default:
				exits("unknown");
			}
			answer(r, mnt);
		}
	}

	exits(0);
}

