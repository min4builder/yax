global strcpy:function (strcpy.end - strcpy)
section .text
strcpy:
	push ebp
	mov ebp, esp
	push esi
	push edi
	mov esi, [ebp+12]
	mov edi, [ebp+8]
.loop:	lodsb
	stosb
	test al, al
	jne .loop
	mov eax, [ebp+8]
	pop edi
	pop esi
	pop ebp
	ret
.end:

