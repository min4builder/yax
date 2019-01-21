global memset:function (memset.end - memset)

section .text
memset:
	push edi
	push ecx
	mov edi, [esp+12]
	mov eax, [esp+16]
	mov ecx, [esp+20]
	rep stosb
	pop ecx
	pop edi
	mov eax, [esp+4]
	ret
.end:

