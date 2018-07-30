CC=cparser -target i686-yax
LD=i686-yax-ld
AR=i686-elf-ar crs
ASM=nasm -felf32 -g
CFLAGS=-g -O0 -ffreestanding -I../include
LDFLAGS=-L. -nostdlib crt0.o crti.o -T linker.ld #-ffreestanding
LIBS=-lc crtn.o #-lgcc

OBJS=exit.o malloc.o memcpy.o memcmp.o memset.o port.o serve.o strcmp.o strlen.o syscall.o

initrd: testcode.o sffs.o initrd.o libc.a crt0.o crti.o crtn.o
	$(LD) $(LDFLAGS) -o $@ testcode.o sffs.o initrd.o $(LIBS)

libc.a: $(OBJS)
	$(AR) -o $@ $(OBJS)

clean:
	rm -f *.o *.a initrd

.s.o: $*.s
	$(ASM) $<

.c.o: $*.c
	$(CC) $(CFLAGS) -o $@ -c $<
	
