global _exits:function (_exits.end - _exits)
global rfork:function (rfork.end - rfork)

global test:function (test.end - test)

global mmap:function (mmap.end - mmap)
global munmap:function (munmap.end - munmap)

global notify:function (notify.end - notify)
global noted:function (noted.end - noted)

global sleep:function (sleep.end - sleep)
global alarm:function (alarm.end - alarm)

global close:function (close.end - close)

global pread:function (pread.end - pread)
global pwrite:function (pwrite.end - pwrite)
global read:function (read.end - read)
global write:function (write.end - write)

global pipe:function (pipe.end - pipe)

global fstat:function (fstat.end - fstat)
global fwstat:function (fwstat.end - fwstat)

global __printk:function (__printk.end - __printk)
global __cprintk:function (__cprintk.end - __cprintk)


section .text
%macro sys 2
%1:
	mov eax, %2
	mov edx, .ret
	lea ecx, [esp+4]
	sysenter
;	ud2
.ret:
	sub esp, 4
	ret
.end:
%endmacro
sys _exits, 0
sys rfork, 1
sys __printk, 2 ; exec

sys test, 3

sys mmap, 4
sys munmap, 5

sys notify, 6
sys __cprintk, 7
sys noted, 8

sys sleep, 9
sys alarm, 10

sys close, 12

sys pread, 13
sys pwrite, 14
sys read, 15
sys write, 16

sys pipe, 21

sys fstat, 25
sys fwstat, 27

