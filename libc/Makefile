include ../config.mk

OBJS=_libc_main.o _exit.o abort.o environ.o errno.o execve.o exit.o exits.o \
	fork.o liballoc.o memcpy.o memcmp.o memset.o port.o serve.o strcat.o \
	strcmp.o strcpy.o strlen.o strlcpy.o strncat.o syscall.o

all: headers $(LIBINSTALL)/libc.a $(LIBINSTALL)/crt0.o $(LIBINSTALL)/crti.o $(LIBINSTALL)/crtn.o

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

libc.a: $(OBJS)
	$(AR) -o $@ $(OBJS)

clean:
	rm -f *.o *.a

