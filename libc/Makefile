include ../config.mk

LIBK_OBJS=itoa.o lock.o memcmp.o memmove.o memset.o port.o strcat.o strcmp.o \
	strcpy.o strlcpy.o strlen.o strncmp.o

OBJS:=_libc_main.o _exit.o abort.o assert.o environ.o errno.o execve.o exit.o \
	exits.o fork.o malloc.o read.o serve.o syscall.o write.o $(LIBK_OBJS)

all: headers $(LIBINSTALL)/libc.a $(LIBINSTALL)/libk.a $(LIBINSTALL)/crt0.o $(LIBINSTALL)/crti.o $(LIBINSTALL)/crtn.o

headers:
	cp -r include/* $(INCLUDEINSTALL)/

$(LIBINSTALL)/crt0.o: crt0.o
	cp crt0.o $(LIBINSTALL)/

$(LIBINSTALL)/crti.o: crti.o
	cp crti.o $(LIBINSTALL)/

$(LIBINSTALL)/crtn.o: crtn.o
	cp crtn.o $(LIBINSTALL)/

$(LIBINSTALL)/libc.a: libc.a
	cp libc.a $(LIBINSTALL)/libc.a

$(LIBINSTALL)/libk.a: libk.a
	cp libk.a $(LIBINSTALL)/libk.a

libc.a: $(OBJS)
	$(AR) $@ $(OBJS)

libk.a: $(LIBK_OBJS)
	$(AR) $@ $(LIBK_OBJS)

clean:
	rm -f *.o *.a

