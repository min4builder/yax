global inb:function (inb.end - inb)
global outb:function (outb.end - outb)
global iowait:function (iowait.end - iowait)

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
iowait:
	xor al, al
	out 0x80, al
	ret
.end:

