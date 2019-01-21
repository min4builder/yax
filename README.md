# YAX
A small, preemptively-multitasking, 32-bit x86 operating system kernel.
(The name means Yet Another uniX).

## The idea
Write a Unix-like microkernel with a Plan 9-inspired VFS as IPC.
On top of that, make a UNIXish userland that is enough source compatible with
Linux for me to be able to make a fully-fledged system without having to
reinvent the wheel too much.

## Current status
The libc is very incomplete.

There are very few drivers; not even a proper disk filesystem is in place yet.

The kernel works, and is actually quite complete, but there are a few features
missing and some of the code is rather messy.

There is documentation available as man pages in the `doc/` directory (for the
kernel only, currently). It is mostly complete.

## How to compile

    ./configure
    ./build-binutils.sh
    ./install-headers.sh
    ./build-gcc.sh

Then setup your environment with `. ./config.sh`.

    make

Then, `make test` for a qemu-based test run.

## Legal
All code I wrote is under the (Expat) MIT license (previously public domain).
Code not written by me:

 - liballoc is under the public domain.
   It is at `src/mem/malloc.[ch]` and `libc/malloc.[ch]`
 - vfprintf was taken from the Sortix libc; it is under ISC.
   It is at `libc/stream/vfprintf.c`

