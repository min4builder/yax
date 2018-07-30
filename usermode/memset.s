global memset:function (memset.end - memset)

section .text
memset:
	push ebp
	mov ebp, esp
	push edi
	push ecx
	mov edi, [ebp+8]
	mov eax, [ebp+12]
	mov ecx, [ebp+16]
	rep stosb
	mov eax, [ebp+8]
	pop ecx
	pop edi
	pop ebp
	ret
.end:

