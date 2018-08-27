#define _YAX_
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/port.h>
#include <sys/serve.h>
#include <sys/types.h>
#include <unistd.h>
#include <yax/errorcodes.h>

static char map[] = {
	0x00, 0x1b, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '[', ']', '\b',
	'\t', '\'', ',', '.', 'p', 'y', 'f', 'g', 'c', 'r', 'l', '/', '=', '\n',
	0x00, 'a', 'o', 'e', 'u', 'i', 'd', 'h', 't', 'n', 's', '-', '\\',
	0x00, '<', ';', 'q', 'j', 'k', 'x', 'b', 'm', 'w', 'v', 'z', 0x00, 0x00,
	0x00, ' '
};

int main(int argc, char **argv)
{
	int fd, mnt;
	off_t off = 0;
	int a;
	char *vga = mmap(0, 80 * 25 * 2, PROT_READ | PROT_WRITE, MAP_PHYS, 0, 0xB8000);
	int irq1 = open("/irq/1", OREAD);

	fd = mkmnt(&mnt);
	__mountfd("/dev", fd, MAFTER);
	close(fd);

	__iopl();

	for(a = 0; a < 80 * 25; a++) {
		vga[a*2] = ' ';
		vga[a*2+1] = 0x07;
	}

	outb(0x03d4, 0x0A);
	outb(0x03d5, (inb(0x03d5) & 0xC0) | 0);
	outb(0x03d4, 0x0B);
	outb(0x03d5, (inb(0x03d5) & 0xE0) | 15);

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
		case MSGWRITE:
			r.u.rw.off = off;
		case MSGPWRITE: {
			int b;
			a = r.u.rw.off;
			for(b = 0; b < r.u.rw.len; b++) {
				char c = ((char *) r.u.rw.buf)[b];
				if(c == '\n') {
					int endl = a + 80 - a % 80;
					for(; a < 80 * 25 && a < endl; a++) {
						vga[a*2] = ' ';
						vga[a*2+1] = 0x07;
					}
				} else if(c == '\b') {
					a = (a - 1) < 0 ? 0 : a - 1;
					vga[a*2] = ' ';
					vga[a*2+1] = 0x07;
				} else {
					vga[a*2] = ((char *) r.u.rw.buf)[b];
					vga[a*2+1] = 0x07;
					a++;
				}
				if(a == 80 * 25) {
					memmove(vga, vga + 2 * 80, 2 * 80 * 24);
					for(a = 80 * 24; a < 80 * 25; a++) {
						vga[a*2] = ' ';
						vga[a*2+1] = 0x07;
					}
					a = 80 * 24;
				}
			}
			r.u.rw.ret = b;
			if(r.fn == MSGWRITE) {
				off = a;
				outb(0x03d4, 0x0F);
				outb(0x03d5, (uint8_t) ((a % (80 * 25)) & 0xFF));
				outb(0x03d4, 0x0E);
				outb(0x03d5, (uint8_t) ((a % (80 * 25)) >> 8));
			}
			break;
		}
		case MSGREAD:
		case MSGPREAD: {
			int i, j = 0, err;
			while(!j) {
				if((err = read(irq1, &i, sizeof(i))) < 0) {
					r.u.rw.ret = err;
					goto outloop;
				}
				for(err = 0, j = 0; err < i; err++) {
					char c = inb(0x60);
					if(c > sizeof(map)/sizeof(map[0]) || map[c] == 0)
						continue;
					((char *)r.u.rw.buf)[j++] = map[c];
				}
			}
			r.u.rw.ret = j;
outloop:		break;
		}
		case MSGSEEK:
			switch(r.u.seek.whence) {
			case 0:
				off = r.u.seek.off;
				break;
			case 1:
				off += r.u.seek.off;
				break;
			case 2:
				off = 80 * 25 + r.u.seek.off;
				break;
			}
			off %= 80 * 25;
			r.u.seek.ret = off;
			break;
		case MSGWALK:
			if(strcmp(r.u.walk.path, "cons") != 0) {
				r.u.walk.ret.type = 0xFF;
				r.u.walk.ret.path = -ENOENT;
				r.u.walk.ret.vers = 0;
			} else {
				r.u.walk.ret.type = 0x40;
				r.u.walk.ret.path = 1;
				r.u.walk.ret.vers = 0;
			}
			break;
		default:
			exits("unknown");
		}
		answer(r, mnt);
	}

	return 0;
}

