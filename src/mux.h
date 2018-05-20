#ifndef _MUX_H
#define _MUX_H

typedef struct Mux Mux;
typedef struct Muxmaster Muxmaster;

struct Mux {
	Conn c;
	uint16_t tag;
	uint32_t fid;
	void *msg;
	Condition recv;
	off_t off;
	Muxmaster *mm;
	Mux *next;
};

struct Muxmaster {
	Lock l;
	Conn *c;
	Mux *first;
};

Muxmaster *muxmasternew(Conn *);
void muxmasterdel(Muxmaster *);

Mux *muxnew(Muxmaster *, const char *);
void muxdel(Mux *);

int muxwalk(Mux *, const char *);

ssize_t muxpread(Mux *, void *, size_t, off_t);
ssize_t muxpwrite(Mux *, const void *, size_t, off_t);
ssize_t muxread(Mux *, void *, size_t);
ssize_t muxwrite(Mux *, const void *, size_t);
off_t muxseek(Mux *, off_t, int);

#endif /* _MUX_H */

