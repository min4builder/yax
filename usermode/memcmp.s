global memcmp:function (memcmp.end - memcmp)

section .text
memcmp:
	push ebp
	mov ebp, esp
	push edi
	push esi
	push ecx
	mov edi, [ebp+8]
	mov esi, [ebp+12]
	mov ecx, [ebp+16]
.loop:	cmpsb
	ja .gt
	jb .st
	loop .loop
	mov eax, 0
.ret:	pop ecx
	pop esi
	pop edi
	pop ebp
	ret
.gt:	mov eax, 1
	jmp .ret
.st:	mov eax, -1
	jmp .ret
.end:

