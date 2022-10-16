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

Currently only tested on my Linux computer. It *might* work on WSL or macOS, but
that hasn't been tested.

    ninja headers
    ./bootstrap-llvm.sh # this step might take a few hours
    ninja

Then, `ninja qemu` for a qemu-based test run; `ninja qemu-dbg` to start qemu on
debug mode.

## Legal
All code I wrote is under the MIT license.
Code not written by me:

 - liballoc is under the public domain.
   It is at `src/mem/malloc.[ch]` and `libc/malloc.[ch]`
 - vfprintf was taken from the Sortix libc, under ISC.
   It is at `libc/stream/vfprintf.c`

