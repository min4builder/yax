global inton:function (inton.end - inton)
global intoff:function (intoff.end - intoff)
global inttest:function (inttest.end - inttest)
global idt_desc
extern piceoi
extern timer_handler
extern page_fault
extern int_handler
extern _syscall
extern _int_start_high

%macro pushsegs 0
	push gs
	push fs
	push es
	push ds
%endmacro
%macro popsegs 0
	pop ds
	pop es
	pop fs
	pop gs
%endmacro
	
section .text
inton:
	sti
	ret
.end:
intoff:
	pushf
	cli
	pop eax
	and eax, ~0x200 ; old IF
	ret
.end:
inttest:
	int 0x30
	ret
.end:

section int_handler
%macro interrupt 1
_int%1 equ $ - $$
int%1:
	pushsegs
	pushad
	mov eax, [esp+48] ; error
	mov ecx, 48
	lea esi, [esp+44]
	lea edi, [esp+48]
	std
	rep movsd
	cld
	mov [esp], eax ; error (push)
	push dword %1 ; n
	lea eax, [esp+8] ; Regs *
	push eax
	call int_handler
	add esp, 12
	popad
	popsegs
	iret
%endmacro
%macro interrupt_noerr 1
_int%1 equ $ - $$
int%1:
	pushsegs
	pushad
	push dword 0 ; error
	push dword %1 ; n
	lea eax, [esp+8] ; Regs *
	push eax
	call int_handler
	add esp, 12
	popad
	popsegs
	iret
%endmacro

interrupt_noerr 0
interrupt_noerr 1
interrupt_noerr 2
interrupt_noerr 3
interrupt_noerr 4
interrupt_noerr 5
_int6 equ $ - $$
int6:
	pushsegs
	pushad
	mov eax, [esp+48] ; eip
	mov ax, [eax]
	cmp ax, 0x340f ; SYSENTER
	je .sysenter
	cmp ax, 0x0b0f ; UD2
	je .sysenter
	push dword 0
	push dword 6
	lea eax, [esp+8]
	push eax
	call int_handler
	add esp, 12
	popad
	popsegs
	iret
.sysenter:
	mov eax, [esp+28] ; old eax
	push ecx
	push eax
	call _syscall
	add esp, 8
	mov [esp+28], eax
	popad
	popsegs
	mov [esp], edx
	mov [esp+12], ecx
	iret

interrupt_noerr 7
interrupt 8
interrupt_noerr 9
interrupt 10
interrupt 11
interrupt 12
interrupt 13
_int14 equ $ - $$
int14:
	pushsegs
	pushad
	mov eax, [esp+48] ; error
	mov ecx, 48
	lea esi, [esp+44]
	lea edi, [esp+48]
	std
	rep movsd
	cld
	mov [esp], eax
	mov eax, cr2 ; addr
	push eax
	lea eax, [esp+8] ; Regs *
	push eax
	call page_fault
	add esp, 12
	popad
	popsegs
	iret
; 15 missing
_int15 equ $ - $$
int15:
	iret
interrupt_noerr 16
interrupt 17
interrupt_noerr 18
interrupt_noerr 19
interrupt_noerr 20

; 21-31 missing
%assign i 21
%rep 32 - 21
_int%[i] equ $ - $$
int%[i]:
	iret
%assign i i+1
%endrep

_int32 equ $ - $$
int32:
	pushsegs
	pushad
	call timer_handler
	push dword 0
	call piceoi
	pop eax
	popad
	popsegs
	iret

%assign i 33
%rep 256 - 33
interrupt_noerr i
%assign i i+1
%endrep

section .rodata
align 8
idt_desc:
	dw 256 * 8 - 1
	dd idt
align 8
idt:
%assign i 0
%rep 256
	dw _int%[i] & 0xffff
	dw 0x0008
	db 0x00
	db 0x8e
	dw (_int%[i] >> 16) + _int_start_high
%assign i i+1
%endrep

