global _start:function (_start.end - _start)
extern _init
extern exits
extern _YALCmain

section .text
_start:
	pop esi ; argv
	pop edi ; envp
	push dword 0
	push dword 0
	mov ebp, esp
	push edi
	push esi
	call _init
	call _YALCmain
	push 0
	call exits
.end:

