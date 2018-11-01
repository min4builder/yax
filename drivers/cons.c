#define __YAX__
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/port.h>
#include <sys/serve.h>
#include <unistd.h>

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
	int irq1 = open("/irq/1", O_RDONLY);

	fd = mkmnt(&mnt);
	__mountfd("/dev", fd, MAFTER);
	close(fd);

	ioperm(0x3d4, 0x3d5, 1);
	ioperm(0x60, 0x60, 1);

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
		case MDEL:
			break;
		case MDUP:
			r.ret = 1;
			break;
		case MOPEN:
			if(r.submsg & O_EXCL)
				r.ret = -1;
			else
				r.ret = 0;
			break;
		case MSWRITE:
			r.off = off;
			/* FALLTHRU */
		case MPWRITE: {
			size_t b;
			a = r.off;
			for(b = 0; b < r.len; b++) {
				char c = ((char *) r.buf)[b];
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
				} else if(c == '\t') {
					int endc = a + 8 - a % 8;
					for(; a < 80 * 25 && a < endc; a++) {
						vga[a*2] = ' ';
						vga[a*2+1] = 0x07;
					}
				} else {
					vga[a*2] = c;
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
			r.ret = b;
			if(r.fn == MSWRITE) {
				off = a;
				outb(0x03d4, 0x0F);
				outb(0x03d5, (uint8_t) ((a % (80 * 25)) & 0xFF));
				outb(0x03d4, 0x0E);
				outb(0x03d5, (uint8_t) ((a % (80 * 25)) >> 8));
			}
			break;
		}
		case MSREAD:
		case MPREAD: {
			int i, j = 0, err;
			while(!j) {
				if((err = read(irq1, &i, sizeof(i))) < 0) {
					r.ret = err;
					goto outloop;
				}
				for(err = 0, j = 0; err < i; err++) {
					unsigned char c = inb(0x60);
					if(c > sizeof(map)/sizeof(map[0]) || map[c] == 0)
						continue;
					((char *)r.buf)[j++] = map[c];
				}
			}
			r.ret = j;
outloop:		break;
		}
		case MSEEK:
			switch(r.submsg) {
			case 0:
				off = r.off;
				break;
			case 1:
				off += r.off;
				break;
			case 2:
				off = 80 * 25 + r.off;
				break;
			}
			off %= 80 * 25;
			r.ret = off;
			break;
		case MWALK:
			if(strncmp(r.buf, "cons", r.len) != 0) {
				r.ret = -ENOENT;
			} else {
				r.ret = 1;
			}
			break;
		default:
			exits("unknown");
		}
		answer(r, mnt);
	}

	(void) argc, (void) argv;
	return 1;
}

