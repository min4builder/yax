global strncat:function (strncat.end - strncat)
section .text
strncat:
	push ebp
	mov ebp, esp
	push esi
	push edi
	push ecx
	mov edx, [ebp+8] ; destination
.l1:	mov al, [edx]
	test al, al
	jz .l1e
	inc edx
	jmp .l1
.l1e:
	mov edi, edx
	mov esi, [ebp+12] ; source
	mov ecx, [ebp+16] ; count
.l2:	lodsb
	stosb
	test al, al
	je .l2e
	loop .l2
.l2e:
	mov byte [edi-1], 0
	mov eax, [ebp+8]
	pop ecx
	pop edi
	pop esi
	pop ebp
	ret
.end:

