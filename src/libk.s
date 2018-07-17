global memset:function (memset.end - memset)
global memcpy:function (memcpy.end - memcpy)
;global memmove:function (memmove.end - memmove)
global memcmp:function (memcmp.end - memcmp)
global strcmp:function (strcmp.end - strcmp)
global strncmp:function (strncmp.end - strncmp)
global strlen:function (strlen.end - strlen)
global strlcpy:function (strlcpy.end - strlcpy)

section .text
memset:
	push edi
	mov ecx, [esp+16] ; count
	mov eax, [esp+12] ; byte
	mov edi, [esp+8] ; dest
	rep stosb
	mov eax, [esp+8]
	pop edi
	ret
.end:
memcpy:
	push esi
	push edi
	mov ecx, [esp+20] ; count
	mov esi, [esp+16] ; source
	mov edi, [esp+12] ; dest
	rep movsb
	mov eax, [esp+12]
	pop edi
	pop esi
	ret
.end:
memmove: ; not implemented correctly
	push esi
	push edi
	mov ecx, [esp+20] ; count
	mov esi, [esp+16] ; source
	mov edi, [esp+12] ; dest
	rep movsb
	mov eax, [esp+12]
	pop edi
	pop esi
	ret
.end:
memcmp:
	push esi
	push edi
	mov ecx, [esp+20] ; count
	mov esi, [esp+16] ; a
	mov edi, [esp+12] ; b
.again:	cmpsb
	jb .gt
	jl .st
	loop .again
	mov eax, 0
.ret:	pop edi
	pop esi
	ret
.gt:	mov eax, 1
	jmp .ret
.st:	mov eax, -1
	jmp .ret
.end:
strcmp:
	push esi
	push edi
	mov esi, [esp+16] ; a
	mov edi, [esp+12] ; b
.again:	cmpsb
	ja .gt
	jb .st
	cmp byte [esi], 0
	jne .again
	mov eax, 0
.ret:	pop edi
	pop esi
	ret
.gt:	mov eax, 1
	jmp .ret
.st:	mov eax, -1
	jmp .ret
.end:
strncmp:
	push esi
	push edi
	push ecx
	mov ecx, [esp+24] ; len
	mov esi, [esp+20] ; a
	mov edi, [esp+16] ; b
.again:	cmpsb
	ja .gt
	jb .st
	cmp byte [esi], 0
	jz .eq
	loop .again
.eq:	mov eax, 0
.ret:	pop edi
	pop esi
	pop ecx
	ret
.gt:	mov eax, 1
	jmp .ret
.st:	mov eax, -1
	jmp .ret
.end:
strlen:
	push edi
	mov edi, [esp + 8]
	mov al, 0
	mov ecx, 0
.again:	scasb
	jz .done
	inc ecx
	jmp .again
.done:	mov eax, ecx
	pop edi
	ret
.end:
strlcpy:
	push edi
	push esi
	mov edi, [esp + 12]
	mov esi, [esp + 16]
	mov ecx, [esp + 20]
.loop:	test ecx, ecx
	jz .done
	lodsb
	stosb
	test al, al
	jz .done
	dec ecx
	jmp .loop
.done:	mov eax, [esp + 20]
	sub eax, ecx ; XXX TODO FIX THIS
	pop esi
	pop edi
	ret
.end:

