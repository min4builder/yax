include ../config.mk

STRING := string/basename.o string/itoa.o string/memcmp.o string/memmove.o \
	string/memset.o string/strcat.o string/strcmp.o string/strchr.o \
	string/strcpy.o string/strlcpy.o string/strerror.o string/strlen.o \
	string/strncmp.o string/strrchr.o string/strstr.o

STREAM := stream/fclose.o stream/ferror.o stream/fflush.o stream/fprintf.o \
	stream/fputc.o stream/fputs.o stream/fread.o stream/fwrite.o \
	stream/perror.o stream/printf.o stream/puts.o stream/stream.o \
	stream/vfprintf.o stream/vprintf.o

LIBK_OBJS := abs.o lock.o port.o stat.o $(STRING)

OBJS := _YALCmain.o _exit.o abort.o assert.o environ.o errno.o execve.o exit.o \
	exits.o fork.o isatty.o malloc.o read.o syscall.o write.o $(STREAM) \
	$(LIBK_OBJS)

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

