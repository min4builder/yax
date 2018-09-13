SUBDIRS=libc drivers initrd src

all:
	./install-headers.sh
	for i in $(SUBDIRS); do cd "$$i"; $(MAKE) $@; cd ..; done
test: all
	cd src; $(MAKE) $@
debug: all
	cd src; $(MAKE) $@
clean:
	for i in $(SUBDIRS); do cd "$$i"; $(MAKE) $@; cd ..; done

src: libc libc/libk.a
initrd: drivers libc libc/libc.a
drivers: libc libc/libc.a

