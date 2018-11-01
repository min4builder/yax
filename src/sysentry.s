global usermode:function (usermode.end - usermode)
global idle:function (idle.end - idle)
global _sysenter:function (_sysenter.end - _sysenter)
global _syscall:function (_syscall.end - _syscall)
extern verusrptr
extern sys_exits
extern sys_rfork
extern sys_exec
extern sys_mkmnt
extern sys_mmap
extern sys_munmap
extern sys_notify
extern sys_ioperm
extern sys_noted
extern sys_sleep
extern sys_alarm
extern sys_open
extern sys_func
extern sys_close
extern sys_dup2
extern sys_chdir
extern sys_pipe
extern sys_mountfd
extern sys_fd2path

extern sys_getprintk

callbytes equ 7*4 ; XXX update this

section .text
usermode:
	; standard fake interrupt frame stuff
	mov ax, 0x23
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov eax, [esp+4]
	mov ecx, [esp+8]
	push dword 0x23
	push ecx
	pushfd
	push dword 0x1b
	push eax
	iret
.end:
idle: ; wait till next interrupt
	sti
.loop:
	hlt
	ret
.end:
_sysenter:
	; called on sysenter; set up stuff and call _syscall
	push ecx
	push ebp
	mov ebp, esp
	cld
	sti
	push edi
	push esi
	push gs
	push fs
	push es
	push ds
	push edx
	push ecx
	push eax

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	call _syscall

	add esp, 4
	pop ecx
	pop edx
	pop ds
	pop es
	pop fs
	pop gs
	pop esi
	pop edi
	pop ebp
	add esp, 4
	sti
	sysexit
.end:

_syscall:
	; do the argument copying and jump table stuff
	mov eax, [esp+8] ; user esp
	push dword 1 ; PROT_READ
	push dword callbytes
	push eax
	call verusrptr
	add esp, 12
	mov ecx, eax
	mov eax, [esp+4] ; call number
	test ecx, ecx
	jz .inval

	mov ecx, callbytes
	mov esi, [esp+8] ; user esp
	lea edi, [esp-callbytes]
	rep movsb
	sub esp, callbytes

	cmp eax, (calltable.end - calltable) / 4
	jae .nocall
	mov eax, [calltable+eax*4]
	jz .nocall
	call eax
	add esp, callbytes
	ret
.nocall:
	add esp, callbytes
.inval:
	mov eax, -1 ; -ENOSYS
	ret
.end:

section .rodata
calltable:
	dd sys_exits
	dd sys_rfork
	dd sys_notify
	dd sys_noted
	dd sys_alarm
	dd sys_sleep
	dd sys_exec
	dd sys_mmap
	dd sys_munmap
	dd sys_mkmnt
	dd sys_pipe
	dd sys_dup2
	dd sys_open
	dd sys_func
	dd 0 ;sys_poll
	dd sys_close
	dd sys_mountfd ;sys_mount
	dd sys_fd2path
	dd sys_chdir
	dd sys_ioperm
	dd sys_getprintk ; empty
.end:

