#define __YAX__
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <yax/func.h>
#include <yax/mount.h>
#include <yax/port.h>
#include <codas/bit.h>
#include <yaxfs/dofunc.h>
#include <yaxfs/fid.h>
#include <yaxfs/file.h>
#include <yaxfs/serve.h>

static char map[] = {
	0x00, 0x1b, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '[', ']', '\b',
	'\t', '\'', ',', '.', 'p', 'y', 'f', 'g', 'c', 'r', 'l', '/', '=', '\n',
	0x00, 'a', 'o', 'e', 'u', 'i', 'd', 'h', 't', 'n', 's', '-', '\\',
	0x00, '<', ';', 'q', 'j', 'k', 'x', 'b', 'm', 'w', 'v', 'z', 0x00, 0x00,
	0x00, ' '
};

int main(int argc, char **argv)
{
	Fidpool fidp = FIDPOOL;
	int fd, mnt;
	int a;
	char *vga = mmap(0, 80 * 25 * 2, PROT_READ | PROT_WRITE, MAP_PHYS, 0, 0xB8000);
	int irq1 = open("/irq/1", O_RDONLY);
	File *root;

	fd = mkmnt(&mnt);
	mount("/dev", fd, MAFTER);
	close(fd);

	root = dirnew((struct stat) { .st_ino = 0, .st_mode = S_IFDIR | 0555, .st_size = 0 }, "/");
	diraddfile(root, filenew((struct stat) { .st_ino = 1, .st_mode = S_IFIFO | 0660, .st_size = 80 * 24 }, "cons", 0, 0));

	fidadd(&fidp, fidnew(root, 0));

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
		case MPWRITE:
		case MSWRITE: {
			Fid *fid = fidlookup(&fidp, r.fid);
			size_t b;
			if(!(fid && fid->omode & O_WRONLY)) {
				r.ret = -EBADF;
				break;
			}
			if(fid->f == root) {
				r.ret = -EACCES;
				break;
			}
			if(r.fn == MSWRITE)
				r.off = fid->off;
			a = r.off % (80 * 24);
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
				fid->off = a;
				outb(0x03d4, 0x0F);
				outb(0x03d5, (uint8_t) ((a % (80 * 25)) & 0xFF));
				outb(0x03d4, 0x0E);
				outb(0x03d5, (uint8_t) ((a % (80 * 25)) >> 8));
			}
			break;
		}
		case MPREAD:
		case MSREAD: {
			Fid *fid = fidlookup(&fidp, r.fid);
			int i, j = 0, err;
			if(fid->f == root) {
				r.ret = dirreadents(fid->f, &fid->dirread, r.buf, r.len);
				break;
			}
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
		case MSEEK: {
			Fid *fid = fidlookup(&fidp, r.fid);
			if(!(fid && fid->omode)) {
				r.ret = -EBADF;
				break;
			}
			switch(r.submsg) {
			case 0:
				fid->off = r.off;
				break;
			case 1:
				fid->off += r.off;
				break;
			case 2:
				fid->off = 80 * 25 + r.off;
				break;
			}
			fid->off %= 80 * 25;
			r.ret = fid->off;
			break;
		}
		default:
			dofunc(&r, &fidp, 0);
			break;
		}
		answer(r, mnt);
	}

	(void) argc, (void) argv;
	return 1;
}

