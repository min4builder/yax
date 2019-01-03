# YAX
A small, preemptively-multitasking, 32-bit x86 operating system kernel.
(The name means Yet Another uniX).

## The idea
Write a Unix-like microkernel with a Plan 9-inspired VFS as IPC.
On top of that, make a UNIXish userland that is enough source compatible with
Linux for me to be able to make a fully-fledged system without having to
reinvent the wheel too much.

## Current status
It doesn't do much by now, I'm still working on the libc and there are likely
too many bugs and missing features on the kernel (also the code is a mess), but
it already manages memory, multitasks and has a VFS. See src/STATUS for an
overview.

There is documentation available as man pages in the `doc/` directory.

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

 - liballoc was under the public domain; this version relicensed to MIT

