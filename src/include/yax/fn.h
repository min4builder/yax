#ifndef _YAX_FN_H
#define _YAX_FN_H

enum {
	MWANTSMASK = 0xffff,
	MWANTSOFF = 1 << 16,
	MWANTSPTR = 1 << 17,
	MWANTSWR = 1 << 18,
	MWANTSPTR2 = 1 << 19,
	MWANTSWR2= 1 << 20
};
enum {
	/* auth(fid, 0, user, 0...) -> status */
	MAUTH = 0 | MWANTSPTR,
	/* open(fid, flags, 0...) -> status */
	MOPEN = 1,
	/* sread(fid, 0, buf, 0...) -> bytes */
	MSREAD = 2 | MWANTSPTR | MWANTSWR,
	/* swrite(fid, 0, buf, 0...) -> bytes */
	MSWRITE = 3 | MWANTSPTR,
	/* seek(fid, func, 0, off, 0) -> newoff */
	MSEEK = 4,
	/* pread(fid, 0, buf, off, 0) -> bytes */
	MPREAD = 5 | MWANTSPTR | MWANTSWR | MWANTSOFF,
	/* pwrite(fid, 0, buf, off, 0) -> bytes */
	MPWRITE = 6 | MWANTSPTR | MWANTSOFF,
	/* stat(fid, 0, buf, 0...) -> bytes */
	MSTAT = 7 | MWANTSPTR | MWANTSWR,
	/* wstat(fid, 0, buf, 0...) -> bytes */
	MWSTAT = 8 | MWANTSPTR,
	/* ioctl generic message, OR with needed attributes and use submsg for
	 * code */
	MIOCTL = 9,
/* for mnt only, cannot be sent directly */
	/* walk(fid, 0, path, 0, &qid) -> qid.path */
	MWALK = 29 | MWANTSPTR | MWANTSPTR2 | MWANTSWR2,
	/* dup(fid) -> newfid */
	MDUP = 30,
	/* del(fid) -> void */
	MDEL = 31
};

#define MIMPL(n) (1 << ((n) & 31))

#endif /* _YAX_FN_H */

