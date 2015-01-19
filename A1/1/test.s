.text
.globl main

main:
	inc %eax
_loop:
	inc %eax
	dec %ebx
	inc %edx
	dec %eax
	inc %ebx
	jmp _loop
