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
#include "ContextRecord.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// ContextRecord::PopEffectiveAddress (private)
//
// Gets the effective address (displacement) associated with a ModR/M byte
//
// Arguments:
//
//	modrm			- ModR/M byte value retrieved from instruction pointer
//	operandsize		- Size expected for a register-direct operation

uintptr_t ContextRecord::PopEffectiveAddress(modrm_t& modrm, uint8_t operandsize)
{
	// Switch on the Mod and R/M bits of the modr/m bytes (11000111 = 0xC7)
	switch (modrm.value & 0xC7) {

		// Mod 00
		case 0x00: return m_context->Eax;								// [EAX]
		case 0x01: return m_context->Ecx;								// [ECX]
		case 0x02: return m_context->Edx;								// [EDX]
		case 0x03: return m_context->Ebx;								// [EBX]
		case 0x04: return PopScaledIndex(modrm);						// [--][--]
		case 0x05: return PopValue<uint32_t>();							// disp32
		case 0x06: return m_context->Esi;								// [ESI]
		case 0x07: return m_context->Edi;								// [EDI]

		// Mod 01
		case 0x40: return m_context->Eax + PopValue<uint8_t>();			// [EAX]+disp8
		case 0x41: return m_context->Ecx + PopValue<uint8_t>();			// [ECX]+disp8
		case 0x42: return m_context->Edx + PopValue<uint8_t>();			// [EDX]+disp8
		case 0x43: return m_context->Ebx + PopValue<uint8_t>();			// [EBX]+disp8
		case 0x44: return PopScaledIndex(modrm) + PopValue<uint8_t>();	// [--][--]+disp8
		case 0x45: return m_context->Ebp + PopValue<uint8_t>();			// [EBP]+disp8
		case 0x46: return m_context->Esi + PopValue<uint8_t>();			// [ESI]+disp8
		case 0x47: return m_context->Edi + PopValue<uint8_t>();			// [EDI]+disp8

		// Mod 10
		case 0x80: return m_context->Eax + PopValue<uint32_t>();		// [EAX]+disp32
		case 0x81: return m_context->Ecx + PopValue<uint32_t>();		// [EDX]+disp32
		case 0x82: return m_context->Edx + PopValue<uint32_t>();		// [ECX]+disp32
		case 0x83: return m_context->Ebx + PopValue<uint32_t>();		// [EBX]+disp32
		case 0x84: return PopScaledIndex(modrm) + PopValue<uint32_t>();	// [--][--]+disp32
		case 0x85: return m_context->Ebp + PopValue<uint32_t>();		// [EBP]+disp32
		case 0x86: return m_context->Esi + PopValue<uint32_t>();		// [ESI]+disp32
		case 0x87: return m_context->Edi + PopValue<uint32_t>();		// [EDI]+disp32

		// Mod 11 (Register-Direct)
		//
		// Register-Direct mode may access different registers depending on the operation.
		// Returns a pointer to the context structure register field, which happens to be
		// an offset into the data segment so this should behave as expected
		//
		// r32 -> EAX / ECX / EDX / EBX / ESP / EBP / ESI / EDI
		// r16 -> AX  / CX  / DX  / BX  / SP  / BP  / SI  / DI
		// r8  -> AL  / CL  / DL  / BL  / AH  / CH  / DH  / BH
	
		case 0xC0: return uintptr_t(&m_context->Eax);				// EAX/AX/AL
		case 0xC1: return uintptr_t(&m_context->Ecx);				// ECX/CX/CL
		case 0xC2: return uintptr_t(&m_context->Edx);				// EDX/DX/DL
		case 0xC3: return uintptr_t(&m_context->Ebx);				// EBX/BX/BL

		case 0xC4:													// ESP/SP/AH
			if(operandsize > sizeof(uint8_t)) return uintptr_t(&m_context->Esp);
			else return uintptr_t(&m_context->Eax) + 1;

		case 0xC5:													// EBP/BP/CH
			if(operandsize > sizeof(uint8_t)) return uintptr_t(&m_context->Ebp);
			else return uintptr_t(&m_context->Ecx) + 1;

		case 0xC6:													// ESI/SI/DH
			if(operandsize > sizeof(uint8_t)) return uintptr_t(&m_context->Esi);
			else return uintptr_t(&m_context->Edx) + 1;

		case 0xC7:													// EDI/DI/BH
			if(operandsize > sizeof(uint8_t)) return uintptr_t(&m_context->Edi);
			else return uintptr_t(&m_context->Ebx) + 1;
	}

	return 0;						// <--- Can't happen
}

//-----------------------------------------------------------------------------
// ContextRecord::PopScaledIndex (private)
//
// Calculates a scaled effective address based on the value of an SIB byte
// read from the next location in the instruction queue
//
// Arguments:
//
//	modrm		- ModR/M byte for the operation

uintptr_t ContextRecord::PopScaledIndex(modrm_t& modrm)
{
	uintptr_t effectiveaddr;			// Calculated effective address

	// Pop the SIB byte from the instruction queue
	sib_t sib(PopValue<uint8_t>());	
	
	// scale * index
	switch(sib.index) {

		case 0x00: effectiveaddr = (m_context->Eax << sib.scale); break;
		case 0x01: effectiveaddr = (m_context->Ecx << sib.scale); break;
		case 0x02: effectiveaddr = (m_context->Edx << sib.scale); break;
		case 0x03: effectiveaddr = (m_context->Ebx << sib.scale); break;
		case 0x04: effectiveaddr = 0; break;
		case 0x05: effectiveaddr = (m_context->Ebp << sib.scale); break;
		case 0x06: effectiveaddr = (m_context->Esi << sib.scale); break;
		case 0x07: effectiveaddr = (m_context->Edi << sib.scale); break;
	}

	// + base + offset
	//
	// Special Case: When base is 0x05, the interpretation depends on the
	// mod value of the ModR/M byte. When MOD=0x00, the base is a 32-bit 
	// displacement. When MOD=0x01 or 0x02, the base is the EBX register
	switch(sib.base) {

		case 0x00: effectiveaddr += m_context->Eax; break;
		case 0x01: effectiveaddr += m_context->Ecx; break;
		case 0x02: effectiveaddr += m_context->Edx; break;
		case 0x03: effectiveaddr += m_context->Ebx; break;
		case 0x04: effectiveaddr += m_context->Esp; break;
		case 0x05: 
			// Special Case: if mod is 00b, the base is a disp32 value.  Otherwise
			// if mod is 01b or 10b, the base is the EBP register.
			if(modrm.mod == 0x00) effectiveaddr += PopValue<uint32_t>();
			else if(modrm.mod == 0x01 || modrm.mod == 0x02) effectiveaddr += m_context->Ebp; 
			break;
		case 0x06: effectiveaddr += m_context->Esi; break;
		case 0x07: effectiveaddr += m_context->Edi; break;
	}

	return effectiveaddr;			// Return calculated effective address
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
