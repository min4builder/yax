/* PATA PIO driver */
#define __YAX__
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <yax/func.h>
#include <yax/mount.h>
#include <yax/port.h>
#include <codas/bit.h>
#include <yaxfs/dofunc.h>
#include <yaxfs/file.h>
#include <yaxfs/serve.h>

#define BPSEC 512

enum {
	RDATA = 0,
	RERR = 1,
	RFEAT = 1,
	RSECC = 2,
	RLO = 3,
	RMID = 4,
	RHI = 5,
	RDRV = 6,
	RSTAT = 7,
	RCOMM = 7,
	RCSTAT = 0,
	RCDCR = 0,
	RCDADDR = 1
};
enum {
	D0 = 0xA0,
	D1 = 0xB0,
	DLBA = 0xD0
};
enum {
	SERR = 0x01,
	SDRQ = 0x08,
	SBSY = 0x80
};

static void print(int fd, const char *s)
{
	write(fd, s, strlen(s));
}

static void selectdrv(int iobase, int d)
{
	static int oldd = -1;
	if(oldd != d) {
		outb(iobase+RDRV, d == 0 ? D0 : D1);
		oldd = d;
	}
}

static uint16_t drvinfo[256];

static ssize_t dread(Fid *fid, void *buf, size_t len, off_t off)
{
	int err;
	off_t lba = off / BPSEC;
	int d = fid->f->dir.qid.path - 1;
	int iobase = 0x1f0;
	int cbase = 0x3f6;
	outb(iobase+RDRV, (d == 0 ? D0 : D1) | DLBA | ((lba >> 24) & 0xf));
	outb(iobase+RSECC, len / BPSEC);
	outb(iobase+RLO, lba & 0xff);
	outb(iobase+RMID, (lba >> 8) & 0xff);
	outb(iobase+RHI, (lba >> 16) & 0xff);
	outb(iobase+RCOMM, 0x20); /* READ SECTORS */
	while(len > 0) {
		while((err = inb(cbase+RCSTAT)) & SBSY) {}
		if(err & SERR)
			return -EIO;
		insw(iobase+RDATA, BPSEC / 2, buf);
		buf = (char *)buf + BPSEC;
		len -= BPSEC;
	}
	return len;
}

static ssize_t dwrite(Fid *fid, void const *buf, size_t len, off_t off)
{
	int err;
	off_t lba = off / BPSEC;
	int d = fid->f->dir.qid.path - 1;
	int iobase = 0x1f0;
	int cbase = 0x3f6;
	outb(iobase+RDRV, (d == 0 ? D0 : D1) | DLBA | ((lba >> 24) & 0xf));
	outb(iobase+RSECC, len / BPSEC);
	outb(iobase+RLO, lba & 0xff);
	outb(iobase+RMID, (lba >> 8) & 0xff);
	outb(iobase+RHI, (lba >> 16) & 0xff);
	outb(iobase+RCOMM, 0x30); /* WRITE SECTORS */
	while(len > 0) {
		while((err = inb(cbase+RCSTAT)) & SBSY) {}
		if(err & SERR)
			return -EIO;
		outsw(iobase+RDATA, BPSEC / 2, buf);
		buf = (char *)buf + BPSEC;
		len -= BPSEC;
	}
	return len;
}

int main()
{
	Fidpool fds = FIDPOOL;
	File *root;
	Func func = { .pread = dread, .pwrite = dwrite };
	char lread;
	int iobase = 0x1F0;
	int cbase = 0x3F6;
	int srv;
	Qid qid = { 0, 0, 0 };
	int fd = mkmnt(&srv, qid);
	mount("/dev", fd, MBEFORE);
	close(fd);
	fd = __getprintk();

	ioperm(iobase, iobase+8, 1);
	ioperm(cbase, cbase+1, 1);
	if(inb(iobase+RSTAT) == 0xFF)
		goto nonexistent; /* floating bus */
	selectdrv(iobase, 0);
	outb(iobase+RSECC, 0);
	outb(iobase+RLO, 0);
	outb(iobase+RMID, 0);
	outb(iobase+RHI, 0);
	outb(iobase+RCOMM, 0xEC); /* IDENTIFY command */
	if((lread = inb(cbase+RCSTAT)) == 0)
		goto nonexistent;
	else if(lread & SERR)
		goto nonexistent; /* not PATA */
	while(inb(cbase+RCSTAT) & SBSY) {}
	if(inb(iobase+RMID) != 0 || inb(iobase+RHI) != 0)
		goto nonexistent; /* not PATA */
	while(!((lread = inb(cbase+RCSTAT)) & (SDRQ | SERR))) {}
	if(lread & SERR)
		goto nonexistent;
	insw(iobase+RDATA, 256, drvinfo);
	outb(cbase+RCDCR, 0);
	write(fd, drvinfo, sizeof(drvinfo));
	print(fd, "HD 0 found\n");

	root = dirnew((Dir) { { QTDIR, 0, 0 }, DMDIR | 0555, 0, 0, 0, "/", "", "", "" });
	diraddfile(root, filenew((Dir) { { QTTMP, 1, 0 }, DMTMP | 0660, 0, 0, drvinfo[60] + (uint32_t) drvinfo[61] << 16, "hd0", "", "", "" }, 0, 0));

	fidadd(&fds, fidnew(root, 0));

	for(;;) {
		Req r = recv(srv);
		dofunc(&r, &fds, &func);
		answer(r, srv);
	}

	return 0;

nonexistent:
	print(fd, "HD 0 nonexistent\n");
	return 1;
}

