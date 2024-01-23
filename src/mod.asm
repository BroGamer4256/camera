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

extern implOfSetCameraPosition
extern realSetCameraPosition

extern implOfSetCameraFocus
extern realSetCameraFocus

extern implOfSetCameraRotation
extern realSetCameraRotation

extern implOfSetCameraHorizontalFov
extern realSetCameraHorizontalFov

extern implOfSetCameraVerticalFov
extern realSetCameraVerticalFov

section .text
implOfSetCameraPosition:
	pushaq
	call realSetCameraPosition
	popaq
	ret

implOfSetCameraFocus:
	pushaq
	call realSetCameraFocus
	popaq
	ret

implOfSetCameraRotation:
	pushaq
	call realSetCameraRotation
	popaq
	ret

implOfSetCameraHorizontalFov:
	pushaq
	call realSetCameraHorizontalFov
	popaq
	ret

implOfSetCameraVerticalFov:
	pushaq
	call realSetCameraVerticalFov
	popaq
	ret
