global strcat:function (strcat.end - strcat)
section .text
strcat:
	push ebp
	mov ebp, esp
	push esi
	push edi
	mov edx, [ebp+8] ; destination
.l1:	mov al, [edx]
	test al, al
	jz .l1e
	inc edx
	jmp .l1
.l1e:
	mov edi, edx
	mov esi, [ebp+12] ; source
.l2:	lodsb
	stosb
	test al, al
	jne .l2
	mov eax, [ebp+8]
	pop edi
	pop esi
	pop ebp
	ret
.end:

