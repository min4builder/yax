section .text
%macro sys 2
global %1:function (%1.end - %1)
%1:
	mov eax, %2
	mov edx, .ret
	lea ecx, [esp+4]
	sysenter
;	ud2
.ret:
	sub esp, 4
	ret
.end:
%endmacro
sys _exits, 0
sys rfork, 1
sys __getprintk, 2;sys exec, 2

sys mkmnt, 3

sys mmap, 4
sys munmap, 5

sys notify, 6
sys __iopl, 7
sys noted, 8

sys sleep, 9
sys alarm, 10

sys open, 11
sys close, 12

sys pread, 13
sys pwrite, 14
sys read, 15
sys write, 16

sys seek, 17
sys dup2, 18
;sys poll, 19
;sys chdir, 20

sys pipe, 21
sys __mountfd, 22;sys mount, 22
sys fd2path, 23

;sys stat, 24
sys fstat, 25
;sys wstat, 26
sys fwstat, 27

