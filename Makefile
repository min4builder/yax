SUBDIRS=libc drivers initrd src

all:
	./install-headers.sh
	for i in $(SUBDIRS); do cd "$$i"; $(MAKE) $@; cd ..; done
test: all
	cd src; $(MAKE) test
clean:
	for i in $(SUBDIRS); do cd "$$i"; $(MAKE) $@; cd ..; done

src: libc
initrd: drivers libc
drivers: libc

