global inb:function (inb.end - inb)
global outb:function (outb.end - outb)

section .text
inb:
	mov dx, [esp + 4]
	in al, dx
	ret
.end:
outb:
	mov dx, [esp + 4]
	mov al, [esp + 8]
	out dx, al
	ret
.end:

