include ../config.mk
CFLAGS+=-std=c89 -ffreestanding -Wall -Wextra -Werror -Wno-error=parentheses -I$(PWD)
LDFLAGS=-T linker.ld -nostdlib
LIBS=-lcodas -lk

QEMU=-serial stdio -kernel yax.bin -initrd ../initrd/initrd -hda /dev/zero

FS = fs/conn.o fs/fds.o fs/iofs.o fs/mnt.o fs/name.o fs/pipe.o fs/printkfs.o
MEM = mem/malloc.o mem/pgdir.o mem/pgdirs.o mem/phys.o mem/str.o mem/user.o \
	mem/usrboundary.o mem/virt.o
# boot.o must be first, otherwise alphabetic order
OBJS := boot.o exec.o $(FS) int.o main.o $(MEM) multitask.o multitasks.o pic.o \
	pit.o printk.o ssp.o syscall.o sysentry.o

all: yax.bin

headers:
	mkdir -p $(INCLUDEINSTALL)/yax; cp -R include/* $(INCLUDEINSTALL)/yax/

clean:
	-$(RM) -f *.o */*.o depend.mk yax.bin yax.img yax.sym

yax.img: yax.bin
	mkfs.msdos -C $@ 1440
	mcopy -i $@ yax.bin syslinux.cfg ../initrd/initrd ::
	syslinux $@

yax.sym: yax.bin
	i686-elf-nm -B yax.bin > $@

test: yax.bin ../initrd/initrd
	qemu-system-i386 $(QEMU)

debug: yax.bin ../initrd/initrd
	qemu-system-i386 -S -s $(QEMU) 2>/dev/null

bochs: yax.img yax.sym
	bochs -q

yax.bin: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

include depgen.mk

