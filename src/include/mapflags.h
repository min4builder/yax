#ifndef _YAX_MAPFLAGS_H
#define _YAX_MAPFLAGS_H

enum mapprot {
	PROT_NONE = 0,
	PROT_READ = 1,
	PROT_WRITE = 2,
	PROT_EXEC = 4
};

#define PROT_NONE PROT_NONE
#define PROT_READ PROT_READ
#define PROT_WRITE PROT_WRITE
#define PROT_EXEC PROT_EXEC

enum mapflags {
	MAP_SHARED = 0,
	MAP_PRIVATE = 1,
	MAP_ANONYMOUS = 2,
	MAP_FIXED = 4,
	MAP_PHYS = 8,
	MAP_NOSHARE = 16
};

#define MAP_SHARED MAP_SHARED
#define MAP_PRIVATE MAP_PRIVATE
#define MAP_ANONYMOUS MAP_ANONYMOUS
#define MAP_FIXED MAP_FIXED
#define MAP_PHYS MAP_PHYS
#define MAP_NOSHARE MAP_NOSHARE

#endif /* _YAX_MAPFLAGS_H */

