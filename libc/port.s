global inb:function (inb.end - inb)
global inw:function (inw.end - inw)
global ind:function (ind.end - ind)
global insb:function (insb.end - insb)
global insw:function (insw.end - insw)
global insd:function (insd.end - insd)
global outb:function (outb.end - outb)
global outw:function (outb.end - outb)
global outd:function (outb.end - outb)
global outsb:function (outb.end - outb)
global outsw:function (outb.end - outb)
global outsd:function (outb.end - outb)

section .text
inb:
	mov dx, [esp + 4]
	in al, dx
	ret
.end:
inw:
	mov dx, [esp + 4]
	in ax, dx
	ret
.end:
ind:
	mov dx, [esp + 4]
	in eax, dx
	ret
.end:
insb:
	push edi
	mov dx, [esp + 8]
	mov ecx, [esp + 12]
	mov edi, [esp + 16]
	rep insb
	pop edi
	ret
.end:
insw:
	push edi
	mov dx, [esp + 8]
	mov ecx, [esp + 12]
	mov edi, [esp + 16]
	rep insw
	pop edi
	ret
.end:
insd:
	push edi
	mov dx, [esp + 8]
	mov ecx, [esp + 12]
	mov edi, [esp + 16]
	rep insd
	pop edi
	ret
.end:
outb:
	mov dx, [esp + 4]
	mov al, [esp + 8]
	out dx, al
	ret
.end:
outw:
	mov dx, [esp + 4]
	mov ax, [esp + 8]
	out dx, ax
	ret
.end:
outd:
	mov dx, [esp + 4]
	mov eax, [esp + 8]
	out dx, eax
	ret
.end:
outsb:
	push esi
	mov dx, [esp + 8]
	mov ecx, [esp + 12]
	mov esi, [esp + 16]
	rep outsb
	pop esi
	ret
.end:
outsw:
	push esi
	mov dx, [esp + 8]
	mov ecx, [esp + 12]
	mov esi, [esp + 16]
	rep outsw
	pop esi
	ret
.end:
outsd:
	push esi
	mov dx, [esp + 8]
	mov ecx, [esp + 12]
	mov esi, [esp + 16]
	rep outsd
	pop esi
	ret
.end:

