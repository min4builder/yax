# YAX
A small, preemptively-multitasking, 32-bit x86 operating system kernel.
(The name means Yet Another uniX).
## The idea
Write a Unix-like microkernel with a Plan 9-inspired VFS as IPC.
On top of that, make a UNIXish userland that is enough source compatible with
Linux to be able to make a fully-fledged system without having to reinvent the
wheel too much.
I hope it works.
## Current status
It doesn't do much by now, I'm still starting the libc and there are many bugs
and missing features on the kernel (also the code is a mess), but it already
manages memory, multitasks and has a VFS. See src/STATUS for an overview.
## License stuff
It is under the public domain or CC0 if your country doesn't understand that.

