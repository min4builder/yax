global __stack_chk_guard
global __stack_chk_fail:function (__stack_chk_fail.end - __stack_chk_fail)
extern printk

section .text
__stack_chk_fail:
	cli
	push ebp
	mov ebp, esp
	push failtext
	call printk
.die:	hlt
	jmp .die
.end:

section .rodata
failtext: db "Stack check fail", 0

section .data
__stack_chk_guard: dd 0xDEADBEEF

