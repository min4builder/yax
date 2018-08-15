CC=cparser -target i686-yax
LD=$(CC)
AR=i686-yax-ar crs
ASM=nasm -felf32 -g
CFLAGS=-g -O0 #-Wall -Wextra -Werror
LDFLAGS=
LIBS=

LIBINSTALL=../sysroot/lib
INCLUDEINSTALL=../sysroot/include

.s.o: $*.s
	$(ASM) -o $@ $<

