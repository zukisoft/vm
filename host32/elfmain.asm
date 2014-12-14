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
; elfmain
;
; Invokes the hosted ELF image entry point
;
;	Arguments:
;
;		entrypoint			- ELF image entry point
;		stackpointer		- Stack pointer
;
;	Function Prototype:
;
;		void __stdcall elfmain(void* entrypoint, void* stackpointer);

elfmain proc stdcall entrypoint:ptr dword, stackpointer:ptr dword

	; zero out the general purpose registers
	xor eax, eax
	xor ebx, ebx
	xor ecx, ecx
	xor edx, edx	; TODO -- EDX SHOULD BE SET TO SOMETHING
	xor esi, esi
	xor edi, edi
	xor ebp, ebp

	; set the stack pointer
	mov esp, stackpointer

	; jump into the binary image entry point
	jmp entrypoint

	; suppress A6001 warning
	ret

elfmain endp

end
