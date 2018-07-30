global strlen:function (strlen.end - strlen)

section .text
strlen:
	push ebp
	mov ebp, esp
	push edi
	mov edi, [ebp+8]
	mov al, 0
.loop:	repne scasb
	mov eax, edi
	sub eax, [ebp+8]
	dec eax
	pop edi
	pop ebp
	ret
.end:

