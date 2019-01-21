#include <_yalc.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_SYS_STAT_H) && defined(_YALC_NEED_POSIX)
#define _SYS_STAT_H

#include <_yalc_struct_timespec.h>

#define S_IFMT 0xf0000000
#define S_IFBLK 0x40000000
#define S_IFCHR 0x40000000
#define S_IFIFO 0x60000000
#define S_IFREG 0x00000000
#define S_IFDIR 0x80000000
#define S_IFLNK 0 /* TODO implement symlinks */
#define S_IFSOCK 0

#define S_IRWXU 0700
#define S_IRUSR 0400
#define S_IWUSR 0200
#define S_IXUSR 0100
#define S_IRWXG 0070
#define S_IRGRP 0040
#define S_IWGRP 0020
#define S_IXGRP 0010
#define S_IRWXO 0007
#define S_IROTH 0004
#define S_IWOTH 0002
#define S_IXOTH 0001
#define S_ISUID 040000
#define S_ISGID 020000
#define S_ISVTX 01000

#define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISLNK(m) 0 /* TODO implement symlinks */
#define S_ISSOCK(m) 0

_YALCDEFTYPE(dev_t);
_YALCDEFTYPE(gid_t);
_YALCDEFTYPE(ino_t);
_YALCDEFTYPE(mode_t);
_YALCDEFTYPE(off_t);
_YALCDEFTYPE(uid_t);

struct stat {
	dev_t st_dev;
	ino_t st_ino;
	mode_t st_mode;
	off_t st_size;
	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;
};

int chmod(char const *, mode_t);
int lstat(char const *__restrict, struct stat *__restrict);
int mkdir(char const *, mode_t);
int mkfifo(char const *, mode_t);
int mknod(char const *, mode_t, dev_t);
int stat(char const *__restrict, struct stat *__restrict);
mode_t umask(mode_t);
int utimensat(int, char const *, struct timespec const[2], int);

#endif /* _SYS_STAT_H */

#ifdef __cplusplus
}
#endif

