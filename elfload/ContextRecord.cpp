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

#include "stdafx.h"						// Include project pre-compiled headers
#include "ContextRecord.h"				// Include ContextRecord declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//---------------------------------------------------------------------------
// ContextRecord::AtInstruction
//
// Determines if the context record instruction pointer indicates the 
// specified instruction
//
// Arguments:
//
//	opcode		- opcode_t instance describing the instruction

bool ContextRecord::AtInstruction(const opcode_t& opcode) const
{
	uint8_t		instructionsize = opcode.numprefixes + opcode.numopcodes;
	uint8_t*	ip = reinterpret_cast<uint8_t*>(m_context->Eip);

	// This should work better than a loop; each case falls through to check
	// the previous byte in the chain ...
	switch(instructionsize) {

		case 7: if(opcode.b6 != ip[6]) return false;	// fall through
		case 6: if(opcode.b5 != ip[5]) return false;	// fall through
		case 5: if(opcode.b4 != ip[4]) return false;	// fall through
		case 4: if(opcode.b3 != ip[3]) return false;	// fall through
		case 3: if(opcode.b2 != ip[2]) return false;	// fall through
		case 2: if(opcode.b1 != ip[1]) return false;	// fall through
		case 1: if(opcode.b0 != ip[0]) return false;	break;
		default: return false;
	}

	// Check the ModR/M byte against the provided bitmask to filter this
	if(opcode.modrm && ((ip[instructionsize] & opcode.modrmmask) != opcode.modrmmask)) return false;

	return true;
}

//-----------------------------------------------------------------------------
// ContextRecord::EatInstruction
//
// "Eats" an instruction by advancing the EIP register beyond it.  Automatically
// deals with ModR/M and SIB bytes, but displacement/immediate bytes must be
// provided with the extra argument
//
// Arguments:
//
//	opcode		- Instruction to eat
//	extra		- Data after the instruction that has also been consumed

void ContextRecord::EatInstruction(const opcode_t& opcode, size_t extra)
{
	// Start with the prefixes and the opcodes
	uint8_t munched = opcode.numprefixes + opcode.numopcodes;

	// Add the appropriate number of eaten bytes for any extra data specified
	// by the ModR/M byte of the instruction.  See the documentation for this
	// byte in the Intel IA-32 Architecture Software Developer's Manual ...
	if(opcode.modrm) {

		modrm_t modrm(*reinterpret_cast<uint8_t*>(m_context->Eip + munched));
		switch(modrm.mod) {

			// MOD 00 --> + 0 bytes
			// MOD 00 + R/M 100 --> + 1 SIB byte
			// MOD 00 + R/M 101 --> + 4 displacement bytes
			case 0x00: 

				if(modrm.rm == 0x04) munched += 1;
				else if(modrm.rm == 0x05) munched += 4;				
				break;

			// MOD 01 --> + 1 displacement byte
			// MOD 01 + R/M 100 --> + 1 SIB byte + 1 displacement byte
			case 0x01:
				munched += (modrm.rm == 0x04) ? 2 : 1;
				break;

			// MOD 10 --> + 4 displacement bytes
			// MOD 10 + R/M 100 --> + 1 SIB byte + 4 displacement bytes
			case 0x02: 
				munched += (modrm.rm == 0x04) ? 5 : 4;
				break;
			
			// MOD 11 --> + 0 bytes
			case 0x03:
				break;
		}
	}

	// Consume the instruction bytes by advancing the instruction pointer
	m_context->Eip += (munched + extra);
}

uint32_t ContextRecord::GetOperand32(const opcode_t& opcode)
{
	uint8_t				disp8;					// 8-bit displacement value
	uint32_t			disp32;					// 32-bit displacement value

	if(!opcode.modrm) return 0;
	
	// Get the ModR/M and SIB bytes for the instruction (SIB byte doesn't always apply)
	uint8_t modrmoffset = opcode.numprefixes + opcode.numopcodes;
	modrm_t modrm(*reinterpret_cast<uint8_t*>(m_context->Eip + modrmoffset));

	// The ModR/M byte dictates where the value comes from
	switch(modrm.mod) {

		// MOD 00 EFFECTIVE ADDRESS TYPES (REGISTERS AS POINTERS)
		case 0x00:
			if(modrm.rm == 0x00) return *reinterpret_cast<uint32_t*>(m_context->Eax);
			else if(modrm.rm == 0x01) return *reinterpret_cast<uint32_t*>(m_context->Ecx);
			else if(modrm.rm == 0x02) return *reinterpret_cast<uint32_t*>(m_context->Edx);
			else if(modrm.rm == 0x03) return *reinterpret_cast<uint32_t*>(m_context->Ebx);
			// 4 --> SIB
			// 5 --> disp32
			else if(modrm.rm == 0x06) return *reinterpret_cast<uint32_t*>(m_context->Esi);
			else if(modrm.rm == 0x07) return *reinterpret_cast<uint32_t*>(m_context->Edi);
			break;

		// MOD 01 EFFECTIVE ADDRESS TYPES (REGISTERS AS POINTERS + 8BIT DISPLACEMENT)
		case 0x01:
			disp8 = *reinterpret_cast<uint8_t*>(m_context->Eip + opcode.numprefixes + opcode.numopcodes + 1);
			if(modrm.rm == 0x00) return (*reinterpret_cast<uint32_t*>(m_context->Eax)) + disp8;
			else if(modrm.rm == 0x01) return (*reinterpret_cast<uint32_t*>(m_context->Ecx)) + disp8;
			else if(modrm.rm == 0x02) return (*reinterpret_cast<uint32_t*>(m_context->Edx)) + disp8;
			else if(modrm.rm == 0x03) return (*reinterpret_cast<uint32_t*>(m_context->Ebx)) + disp8;
			// 4 --> SIB
			else if(modrm.rm == 0x05) return (*reinterpret_cast<uint32_t*>(m_context->Ebp)) + disp8;
			else if(modrm.rm == 0x06) return (*reinterpret_cast<uint32_t*>(m_context->Esi)) + disp8;
			else if(modrm.rm == 0x07) return (*reinterpret_cast<uint32_t*>(m_context->Edi)) + disp8;
			break;

		// MOD 10 EFFECTIVE ADDRESS TYPES (REGISTERS AS POINTERS + 32BIT DISPLACEMENT)
		case 0x02:
			disp32 = *reinterpret_cast<uint32_t*>(m_context->Eip + opcode.numprefixes + opcode.numopcodes + 1);
			if(modrm.rm == 0x00) return (*reinterpret_cast<uint32_t*>(m_context->Eax)) + disp32;
			else if(modrm.rm == 0x01) return (*reinterpret_cast<uint32_t*>(m_context->Ecx)) + disp32;
			else if(modrm.rm == 0x02) return (*reinterpret_cast<uint32_t*>(m_context->Edx)) + disp32;
			else if(modrm.rm == 0x03) return (*reinterpret_cast<uint32_t*>(m_context->Ebx)) + disp32;
			// 4 --> SIB
			else if(modrm.rm == 0x05) return (*reinterpret_cast<uint32_t*>(m_context->Ebp)) + disp32;
			else if(modrm.rm == 0x06) return (*reinterpret_cast<uint32_t*>(m_context->Esi)) + disp32;
			else if(modrm.rm == 0x07) return (*reinterpret_cast<uint32_t*>(m_context->Edi)) + disp32;
			break;

		// MOD 11 EFFECTIVE ADDRESS TYPES (REGISTERS AS VALUES)
		case 0x03:
			if(modrm.rm == 0x00) return m_context->Eax;
			else if(modrm.rm == 0x01) return m_context->Ecx;
			else if(modrm.rm == 0x02) return m_context->Edx;
			else if(modrm.rm == 0x03) return m_context->Ebx;
			else if(modrm.rm == 0x04) return m_context->Esp;
			else if(modrm.rm == 0x05) return m_context->Ebp;
			else if(modrm.rm == 0x06) return m_context->Esi;
			else if(modrm.rm == 0x07) return m_context->Edi;
			break;
	}
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
