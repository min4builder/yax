global procrfork:function (procrfork.end - procrfork)
global procexits:function (procexits.end - procexits)
global procswitch:function (procswitch.end - procswitch)
global prochandle:function (prochandle.end - prochandle)
global procusrcall:function (procusrcall.end - procusrcall)
global procusrret:function (procusrret.end - procusrret)
extern switchsyscallstack
extern procblock
extern procunblock
extern procrforkgut
extern procexitsgut1
extern procexitsgut2
extern procswitchgut
extern prochandlegut

section .text
procswitch:
	pop eax
	pushf
	push dword 0x08
	push eax
	push gs
	push fs
	push es
	push ds
	pushad
	mov eax, esp
	push eax
	call procswitchgut
	mov esp, eax
	popad
	pop ds
	pop es
	pop fs
	pop gs
	iret
.end:
procrfork:
	pop ecx
	pushfd
	push dword 0x08
	push ecx
	push gs
	push fs
	push es
	push ds
	mov eax, 0
	pushad
	mov eax, esp
	mov ecx, [esp+60]
	push ecx
	push eax
	call procrforkgut
	add esp, 8
	mov [esp+28], eax
	popad
	pop ds
	pop es
	pop fs
	pop gs
	iret
.end:
procexits:
	mov eax, [esp+4]
	sub esp, 4 ; for p
	push eax
	lea eax, [esp+8]
	push eax
	call procexitsgut1
	mov ecx, [esp+12] ; p
	mov esp, eax
	push ecx
	call procexitsgut2
	add esp, 4
	popad
	pop ds
	pop es
	pop fs
	pop gs
	iret
.end:
prochandle:
	add esp, 8 ; pop ss and esp from int stack frame
	call prochandlegut
	mov esp, eax
	popad
	pop ds
	pop es
	pop fs
	pop gs
	iret
.end:
procusrcall:
	push ebp
	mov ebp, esp
	sub esp, 4
	pushad
	mov eax, esp
	push eax
	call [switchsyscallstack]
	add esp, 4
	mov [ebp-4], eax
	push dword 0x23
	push dword [ebp+12]
	pushf
	push dword 0x1b
	push dword [ebp+8]
	push dword 0x23
	push dword 0x23
	push dword 0x23
	push dword 0x23
	pushad
	popad
	pop ds
	pop es
	pop fs
	pop gs
	iret
.end:
procusrret:
	push dword 0
	call [switchsyscallstack]
	mov esp, eax
	popad
	push dword [ebp-4]
	call [switchsyscallstack]
	leave
	ret
.end:

