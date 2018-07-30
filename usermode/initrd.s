global initrd
global initrdlen

section .rodata
initrd:
	incbin "sf"
initrdlen dd $ - initrd

