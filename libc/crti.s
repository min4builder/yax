global _init
global _fini

section .init
_init:
	push ebp
	mov ebp, esp
	; ... rest of function added by linker ...

section .fini
_fini:
	push ebp
	mov ebp, esp
	; idem

