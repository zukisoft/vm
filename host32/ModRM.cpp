//-----------------------------------------------------------------------------
// Copyright (c) 2014 Michael G. Brehm
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "ModRM.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// ModRM::GetEffectiveAddress (private, static)
//
// Gets the effective address (displacement) associated with a ModR/M byte
//
// Arguments:
//
//	directsize	- Size expected for a register-direct operation
//	modrm		- Parent ModRM instance
//	context		- Execution context record

uintptr_t ModRM::GetEffectiveAddress(uint8_t directsize, ModRM& modrm, ContextRecord& context)
{
	// Register-Direct operations need the address of actual structure member
	PCONTEXT pcontext = context;

	// Switch on the Mod and R/M bits of the modr/m bytes (11000111 = 0xC7)
	switch (modrm.value & 0xC7) {

		// Mod 00
		case 0x00: return uintptr_t(context.Registers.EAX);
		case 0x01: return uintptr_t(context.Registers.ECX);
		case 0x02: return uintptr_t(context.Registers.EDX);
		case 0x03: return uintptr_t(context.Registers.EBX);
		case 0x04: return GetScaledEffectiveAddress(modrm, context);
		case 0x05: return uintptr_t(context.PopInstruction<uint32_t>());
		case 0x06: return uintptr_t(context.Registers.ESI);
		case 0x07: return uintptr_t(context.Registers.EDI);

		// Mod 01
		case 0x40: return uintptr_t(context.Registers.EAX + context.PopInstruction<uint8_t>());
		case 0x41: return uintptr_t(context.Registers.ECX + context.PopInstruction<uint8_t>());
		case 0x42: return uintptr_t(context.Registers.EDX + context.PopInstruction<uint8_t>());
		case 0x43: return uintptr_t(context.Registers.EBX + context.PopInstruction<uint8_t>());
		case 0x44: return GetScaledEffectiveAddress(modrm, context);
		case 0x45: return uintptr_t(context.Registers.EBP + context.PopInstruction<uint8_t>());
		case 0x46: return uintptr_t(context.Registers.ESI + context.PopInstruction<uint8_t>());
		case 0x47: return uintptr_t(context.Registers.EDI + context.PopInstruction<uint8_t>());

		// Mod 10
		case 0x80: return uintptr_t(context.Registers.EAX + context.PopInstruction<uint32_t>());
		case 0x81: return uintptr_t(context.Registers.ECX + context.PopInstruction<uint32_t>());
		case 0x82: return uintptr_t(context.Registers.EDX + context.PopInstruction<uint32_t>());
		case 0x83: return uintptr_t(context.Registers.EBX + context.PopInstruction<uint32_t>());
		case 0x84: return GetScaledEffectiveAddress(modrm, context);
		case 0x85: return uintptr_t(context.Registers.EBP + context.PopInstruction<uint32_t>());
		case 0x86: return uintptr_t(context.Registers.ESI + context.PopInstruction<uint32_t>());
		case 0x87: return uintptr_t(context.Registers.EDI + context.PopInstruction<uint32_t>());

		// Mod 11 (Register-Direct)
		//
		// Register-Direct mode may access different registers depending on the operation.
		// Returns a pointer to the context structure register field, which happens to be
		// an offset into the data segment so this should behave as expected
		//
		// r32 -> EAX / ECX / EDX / EBX / ESP / EBP / ESI / EDI
		// r16 -> AX  / CX  / DX  / BX  / SP  / BP  / SI  / DI
		// r8  -> AL  / CL  / DL  / BL  / AH  / CH  / DH  / BH
	
		case 0xC0: return uintptr_t(&pcontext->Eax);	// EAX/AX/AL
		case 0xC1: return uintptr_t(&pcontext->Ecx);	// ECX/CX/CL
		case 0xC2: return uintptr_t(&pcontext->Edx);	// EDX/DX/DL
		case 0xC3: return uintptr_t(&pcontext->Ebx);	// EBX/BX/BL

		case 0xC4:										// ESP/SP/AH
			if(directsize > sizeof(uint8_t)) return uintptr_t(&pcontext->Esp);
			else return uintptr_t(&pcontext->Eax) + 1;

		case 0xC5:										// EBP/BP/CH
			if(directsize > sizeof(uint8_t)) return uintptr_t(&pcontext->Ebp);
			else return uintptr_t(&pcontext->Ecx) + 1;

		case 0xC6:										// ESI/SI/DH
			if(directsize > sizeof(uint8_t)) return uintptr_t(&pcontext->Esi);
			else return uintptr_t(&pcontext->Edx) + 1;

		case 0xC7:										// EDI/DI/BH
			if(directsize > sizeof(uint8_t)) return uintptr_t(&pcontext->Edi);
			else return uintptr_t(&pcontext->Ebx) + 1;
	}

	return 0;						// <--- Can't happen
}

//-----------------------------------------------------------------------------
// ModRM::GetScaledEffectiveAddress (private, static)
//
// Calculates a scaled effective address based on the value of an SIB byte
// read from the next location in the instruction queue
//
// Arguments:
//
//	modrm		- Parent ModRM instance
//	context		- Execution context record

uintptr_t ModRM::GetScaledEffectiveAddress(ModRM& modrm, ContextRecord& context)
{
	uintptr_t effectiveaddr;			// Calculated effective address

	// Pop the SIB byte from the instruction queue
	sib_t sib(context.PopInstruction<uint8_t>());
	
	// scale * index
	switch(sib.index) {

		case 0x00: effectiveaddr = (context.Registers.EAX << sib.scale); break;
		case 0x01: effectiveaddr = (context.Registers.ECX << sib.scale); break;
		case 0x02: effectiveaddr = (context.Registers.EDX << sib.scale); break;
		case 0x03: effectiveaddr = (context.Registers.EBX << sib.scale); break;
		case 0x04: effectiveaddr = 0; break;
		case 0x05: effectiveaddr = (context.Registers.EBP << sib.scale); break;
		case 0x06: effectiveaddr = (context.Registers.ESI << sib.scale); break;
		case 0x07: effectiveaddr = (context.Registers.EDI << sib.scale); break;
	}

	// + base + offset
	switch(sib.base) {

		case 0x00: effectiveaddr += uintptr_t(context.Registers.EAX); break;
		case 0x01: effectiveaddr += uintptr_t(context.Registers.ECX); break;
		case 0x02: effectiveaddr += uintptr_t(context.Registers.EDX); break;
		case 0x03: effectiveaddr += uintptr_t(context.Registers.EBX); break;
		case 0x04: effectiveaddr += uintptr_t(context.Registers.ESP); break;
		case 0x05:
				if(modrm.mod == 0x00) effectiveaddr += context.PopInstruction<uint32_t>();
				else if(modrm.mod == 0x01) effectiveaddr += uintptr_t(context.Registers.EBP) + context.PopInstruction<uint8_t>();
				else if(modrm.mod == 0x01) effectiveaddr += uintptr_t(context.Registers.EBP) + context.PopInstruction<uint32_t>();
				break;
		case 0x06: effectiveaddr += uintptr_t(context.Registers.ESI); break;
		case 0x07: effectiveaddr += uintptr_t(context.Registers.EDI); break;
	}

	return effectiveaddr;			// Return calculated effective address
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
