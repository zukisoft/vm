;------------------------------------------------------------------------------
; Copyright (c) 2014 Michael G. Brehm
; 
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
; 
; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.
; 
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.
;------------------------------------------------------------------------------

.code

;------------------------------------------------------------------------------
; elfmain
;
; Invokes the hosted ELF image entry point
;
;	Arguments:
;
;		address			- ELF image entry point
;		stackimage		- Initial stack image
;		stackimagelen	- Length of stack image in bytes
;
;	Function Prototype:
;
;		void elfmain(void* address, const void* stackimage, size_t stackimagelen);

elfmain proc

	; copy the entry point into r9 since we need rcx for movsq
	mov r9, rcx

	; copy the stack image into the thread stack space
	mov rsi, rdx
	sub rsp, r8
	mov rdi, rsp
	mov rcx, r8
	shr rcx, 4
	rep movsq

	; zero out the general purpose registers
	xor rax, rax
	xor rbx, rbx
	xor rcx, rcx
	xor rdx, rdx
	xor rsi, rsi
	xor rdi, rdi
	xor r8, r8
	xor r10, r10
	xor r11, r11
	xor r12, r12
	xor r13, r13
	xor r14, r14
	xor r15, r15

	; jump into the ELF image entry point
	jmp r9

	; suppress A6001 warning
	ret

elfmain endp

end
