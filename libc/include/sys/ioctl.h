#include <_yalc.h>

#if !defined(_SYS_IOCTL_H) && defined(_YALC_NEED_BSD)
#define _SYS_IOCTL_H

int ioctl(int, unsigned long, ...);

#endif /* _SYS_IOCTL_H */

