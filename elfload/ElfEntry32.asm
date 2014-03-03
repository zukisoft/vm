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

.386
.model flat, c

.code

;------------------------------------------------------------------------------
; ElfEntry
;
; Invokes an ELF image entry point
;
;	Arguments:
;
;		address			- ELF image entry point
;		argvector		- ELF argument vector to copy onto stack
;		argvectorlen	- Length of ELF argument vector in bytes
;
;	Function Prototype:
;
;		void ElfEntry(void* address, const void* argvector, size_t argvectorlen);

ElfEntry proc stdcall address:ptr dword, argvector:ptr dword, argvectorlen:dword

	; copy the argument vector into the thread stack space
	mov esi, argvector
	sub esp, argvectorlen
	mov edi, esp
	mov ecx, argvectorlen
	shr ecx, 2
	rep movsd

	; zero out the general purpose registers
	xor eax, eax
	xor ebx, ebx
	xor ecx, ecx
	xor edx, edx
	xor esi, esi
	xor edi, edi

	; jump into the ELF image entry point
	jmp address

	; suppress A6001 warning
	ret

ElfEntry endp

end
