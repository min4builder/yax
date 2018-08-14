global _start:function (__start.end - __start)
global halt:function (halt.end - halt)
global clearkidmap:function (clearkidmap.end - clearkidmap)
global switchsyscallstack
global kernel_stack_bottom
global kernel_stack_low
global kernel_pd
global kernel_spte
global _kernel_pt1
global _kernel_pt2
global _kernel_lastpt
global VIRT
global _tss
extern _init
extern kernel_main
extern _sysenter
extern idt_desc
extern _kernel_pt1_lonibble
extern _kernel_pt1_hiword
extern _kernel_pt2_lonibble
extern _kernel_pt2_hiword
extern _kernel_pd_lonibble
extern _kernel_pd_hiword
extern _tss_loword
extern _tss_midbyte
extern _tss_hibyte

VIRT equ 0xc0000000

%define VIRT(x) (VIRT + (x))
%define PHYS(x) ((x) - VIRT)

section multiboot
multiboot_header:
align 4
	dd 0x1badb002
	dd 3
	dd -(0x1badb002 + 3)
.end:

section .text
_start equ PHYS(__start)
__start:
	mov ecx, PHYS(kernel_pd)
	mov cr3, ecx
	mov ecx, cr4
	or ecx, 0x90 ; 4mb pages & global pages
	mov cr4, ecx
	mov ecx, cr0
	or ecx, 0x80008000 ; enable paging and WP
	mov cr0, ecx
	mov ecx, .higherhalf
	jmp ecx
.higherhalf:
	cmp eax, 0x2badb002 ; basic sanity check
	jne halt
	lidt [idt_desc]
	lgdt [gdt]
	mov ax, gdt.tss
	ltr ax
	mov ax, gdt.data
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp gdt.code:.seg
.seg:
	mov ebp, 0 ; setup bottom of ebp chain
	mov esp, kernel_stack_bottom
	push ebx ; multiboot info struct
	pushfd
	pushfd
	xor dword [esp], 0x200000 ; flip ID bit
	popfd
	pushfd
	pop eax
	xor eax, [esp]
	popfd
	and eax, 0x200000
	test eax, eax
	jz .nosysenter ; no cpuid; no sysenter
	mov eax, 0x01
	cpuid
	mov eax, edx
	xor eax, 1 << 11
	xor eax, edx
	test eax, eax
	jz .nosysenter
	mov ecx, 0x174 ; setup sysenter
	rdmsr
	mov ax, 0x08
	wrmsr
	mov ecx, 0x175
	rdmsr
	mov eax, kernel_stack_bottom
	wrmsr
	mov ecx, 0x176
	rdmsr
	mov eax, _sysenter
	wrmsr
	mov dword [switchsyscallstack], switchsysenter
.nosysenter:
	call kernel_main
.end:
halt:	; kernel shouldn't return
	cli
.die:	hlt
	jmp .die
.end:
clearkidmap:
	mov dword [kernel_pd], 0
	mov dword [kernel_pd+4], 0
	ret
.end:
switchsysenter:
	mov ecx, 0x175
	rdmsr
	mov eax, [esp+4]
	wrmsr
	; fallthrough
switchsysentry:
	mov eax, [esp+4]
	xchg [sysstack], eax
	ret

section .bss
align 16
kernel_stack_low:
resb 16 * 1024
kernel_stack_bottom:
align 0x1000

section .rodata
%macro seg_desc 2
	dq 0x00cf92000000ffff | (%1 << 43) | (%2 << 45)
%endmacro
gdt:
	dw gdt.end - gdt - 1
	dd gdt
	dw 0
.code equ $ - gdt
	seg_desc 1, 0
.data equ $ - gdt
	seg_desc 0, 0
.usercode equ $ - gdt
	seg_desc 1, 3
.userdata equ $ - gdt
	seg_desc 0, 3
.tss equ $ - gdt
	dw 0x68
	dw _tss_loword
	db _tss_midbyte
	db 0x89
	db 0x40
	db _tss_hibyte
.end:

section .data
align 4096
_kernel_pt1:
%assign i 0
%rep 1024
	dd 0x00000083 | (i << 12)
%assign i i+1
%endrep
_kernel_pt2:
%rep 1024
	dd 0x00000083 | (i << 12)
%assign i i+1
%endrep
kernel_pd:
	db 0x03
	db _kernel_pt1_lonibble
	dw _kernel_pt1_hiword
	db 0x03
	db _kernel_pt2_lonibble
	dw _kernel_pt2_hiword
	times ((VIRT(0) >> 22) - 2) dd 0
	db 0x03
	db _kernel_pt1_lonibble
	dw _kernel_pt1_hiword
	db 0x03
	db _kernel_pt2_lonibble
	dw _kernel_pt2_hiword
	times (1024 - (VIRT(0) >> 22) - 3) dd 0
	db 0x03
	db _kernel_pd_lonibble
	dw _kernel_pd_hiword
switchsyscallstack:
	dd switchsysentry
_tss:
	dd 0
sysstack:
	dd kernel_stack_bottom
	dd gdt.data
times 23 dd 0

