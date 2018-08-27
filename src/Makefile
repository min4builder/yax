include ../config.mk
LD=i686-yax-ld
CFLAGS+=-std=c89 -ffreestanding -Werror
LDFLAGS=-T linker.ld -nostdlib
LIBS=

# boot.o must be first
OBJS = boot.o conn.o exec.o fds.o int.o iofs.o libk.o lock.o main.o malloc.o \
	mnt.o multitask.o multitasks.o name.o pgdir.o physmman.o pic.o pipe.o \
	pit.o port.o printk.o printkfs.o ref.o ssp.o stat.o syscall.o \
	sysentry.o virtmman.o

all: yax.bin

headers:
	mkdir -p $(INCLUDEINSTALL)/yax; cp -R include/* $(INCLUDEINSTALL)/yax/

clean:
	-$(RM) -f *.o depend.mk yax.bin yax.img yax.sym

yax.img: yax.bin
	mkfs.msdos -C $@ 1440
	mcopy -i $@ yax.bin syslinux.cfg ../initrd/initrd ::
	syslinux $@

yax.sym: yax.bin
	i686-elf-nm -B yax.bin > $@

test: yax.bin ../initrd/initrd
	qemu-system-i386 -serial stdio -kernel yax.bin -initrd ../initrd/initrd #2>/dev/null

debug: yax.bin ../initrd/initrd
	qemu-system-i386 -S -s -serial stdio -kernel yax.bin -initrd ../initrd/initrd 2>/dev/null

bochs: yax.img yax.sym
	bochs -q

yax.bin: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

include depgen.mk

