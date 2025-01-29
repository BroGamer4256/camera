%macro pushaq 0
	push rax
	push rcx
	push rdx
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	sub rsp, 0x10
%endmacro

%macro popaq 0
	add rsp, 0x10
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rax
%endmacro

extern implOfSetCameraData
extern realSetCameraData

section .text
implOfSetCameraData:
	pushaq
	call realSetCameraData
	popaq
	ret
