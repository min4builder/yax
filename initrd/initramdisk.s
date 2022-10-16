global initrd
global initrdlen

section .rodata
initrd:
	incbin "initrd/initrd"
initrdlen dd $ - initrd

