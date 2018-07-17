global strcmp:function (strcmp.end - strcmp)

section .text
strcmp:
	push ebp
	mov ebp, esp
	push esi
	push edi
	mov edi, [ebp+8]
	mov esi, [ebp+12]
.loop:	cmpsb
	ja .gt
	jb .st
	jnz .loop
	mov eax, 0
.ret:	pop edi
	pop esi
	pop ebp
	ret
.gt:	mov eax, 1
	jmp .ret
.st:	mov eax, -1
	jmp .ret
.end:

