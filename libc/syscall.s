section .text

%macro sys 2

global %1:function (%1.end - %1)

%1:
	mov eax, %2
	mov edx, .ret
	lea ecx, [esp+4]
	sysenter
.ret:
	sub esp, 4
	ret
.end:

%endmacro

sys _exits, 0
sys rfork, 1
sys notify, 2
sys noted, 3
sys alarm, 4
sys sleep, 5
sys exec, 6
sys mmap, 7
sys munmap, 8
sys mkmnt, 9
sys pipe, 10
sys dup2, 11
sys open, 12
sys func, 13
;sys poll, 14
sys close, 15
sys mount, 16
sys fd2path, 17
sys chdir, 18
sys ioperm, 19
sys __getprintk, 20

