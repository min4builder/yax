global pginval:function (ptinval.end - pginval)
global ptinval:function (ptinval.end - ptinval)
global getpgdir:function (getpgdir.end - getpgdir)
global switchpgdir:function (switchpgdir.end - switchpgdir)

section .text
pginval:
	mov eax, cr3
	mov cr3, eax
	ret
.end:
ptinval:
	mov eax, cr3
	mov cr3, eax
	ret
.end:
getpgdir:
	mov eax, cr3
	ret
.end:
switchpgdir:
	mov ecx, [esp+4]
	mov eax, cr3
	mov cr3, ecx
	ret
.end:
%if 0
pginval:
	mov eax, [esp+4]
	invlpg [eax]
	jmp pdinval
.end:
ptinval:
	mov eax, [esp+4]
	mov ecx, 1024
.loop:	invlpg [eax]
	mov edx, eax
	shr edx, 2
	add edx, 0xFFC00000
	invlpg [edx]
	add eax, 1 << 12
	loop .loop
	ret
.end:
pdinval:
	push eax
	shr eax, 2
	add eax, 0xFFC00000
	invlpg [eax]
	pop eax
	ret
%endif

