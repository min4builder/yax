global locklock:function (locklock.end - locklock)
global lockunlock:function (lockunlock.end - lockunlock)

section .text
locklock:
	mov eax, [esp+4]
	mov ecx, 1
.again:
	xchg ecx, [eax]
	pause
	test ecx, ecx
	jne .again
	ret
.end:
lockunlock:
	mov eax, [esp+4]
	mov dword [eax], 0 ; guaranteed to be atomic if [eax] is aligned
	ret
.end:

