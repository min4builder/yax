global memcpy:function (memcpy.end - memcpy)
global memmove:function (memmove.end - memmove)

section .text
memcpy:
	push ebp
	mov ebp, esp
	push edi
	push esi
	push ecx
	mov edi, [ebp+8]
	mov esi, [ebp+12]
	mov ecx, [ebp+16]
	rep movsb
	pop ecx
	pop esi
	pop edi
	pop ebp
	ret
.end:
memmove:
	push ebp
	mov ebp, esp
	push edi
	push esi
	push ecx
	mov edi, [ebp+8]
	mov esi, [ebp+12]
	mov ecx, [ebp+16]
	cmp edi, esi
	jb .ok
	add edi, ecx
	add esi, ecx
	std
.ok:	rep movsb
	cld
	pop ecx
	pop esi
	pop edi
	pop ebp
	ret
.end:

