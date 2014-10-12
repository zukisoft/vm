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

#ifndef __EMULATOR_H_
#define __EMULATOR_H_
#pragma once

#include <functional>
#include <stdint.h>

#pragma warning(push, 4)
#pragma warning(disable:4127)		// conditional expression is constant
#pragma warning(disable:4201)		// nameless struct / union

namespace emulator {

	// context_t
	//
	// Typedef for the Win32 CONTEXT structure
	using context_t = CONTEXT;

	// handler_t
	//
	// Defines the lambda function that executes an instruction
	typedef std::function<bool(context_t*)> handler_t;

	// instruction_bytes_t
	//
	// Base class for encoded instruction byte wrappers
	template<typename size_type> struct instruction_bytes_t
	{
		// Instance Constructor
		//
		explicit instruction_bytes_t(context_t* context)
		{
			// Extract the value from the current instruction pointer and
			// increment it to the next encoded instruction byte
			Value = *reinterpret_cast<size_type*>(context->Eip);
			context->Eip += sizeof(size_type);
		}

		// size_type conversion operator
		//
		operator size_type() { return Value; }

		// value
		//
		// Value extracted from the encoded instruction bytes
		size_type Value;
	};

	// modrm_byte_t
	//
	// Union that defines the individual values within a ModR/M byte
	union modrm_byte_t {

		explicit modrm_byte_t(uint8_t val) : value(val) {}
		struct	{ uint8_t rm:3; uint8_t reg:3; uint8_t mod:2; };
		uint8_t	value;

	};

	// sib_byte_t
	//
	// Union that defines the individual values within a SIB byte
	union sib_byte_t {

		explicit sib_byte_t(uint8_t val) : value(val) {}
		struct { uint8_t base:3; uint8_t index:3; uint8_t scale:2; };
		uint8_t	value;
	};

	// byte
	//
	// 8-bit unsigned instruction
	using byte			= instruction_bytes_t<uint8_t>;

	// word
	//
	// 16-bit unsigned instruction
	using word			= instruction_bytes_t<uint16_t>;

	// doubleword
	//
	// 32-bit unsigned instruction
	using doubleword	= instruction_bytes_t<uint32_t>;

	// dispxx
	//
	// Displacement instructions
	using disp8			= instruction_bytes_t<uint8_t>;
	using disp16		= instruction_bytes_t<uint16_t>;
	using disp32		= instruction_bytes_t<uint32_t>;

	// immxx
	//
	// Immediate instructions
	using imm8			= instruction_bytes_t<int8_t>;
	using imm16			= instruction_bytes_t<int16_t>;
	using imm32			= instruction_bytes_t<int32_t>;

	// moffsxx
	//
	// Memory offset instructions
	using moffs8		= instruction_bytes_t<uint8_t>;
	using moffs16		= instruction_bytes_t<uint16_t>;
	using moffs32		= instruction_bytes_t<uint32_t>;

	// rmxx
	//
	// ModR/M instructions
	template<typename size_type> class modrm_t;

	using rm8		= modrm_t<uint8_t>;
	using rm16		= modrm_t<uint16_t>;
	using rm32		= modrm_t<uint32_t>;

	// instruction_t
	//
	// Defines an emulated instruction instance
	class instruction_t 
	{
	public:

		// Instance Constructors
		//
		instruction_t(uint8_t opcode0, handler_t handler) : m_opcount(1), m_opcode0(opcode0), m_handler(handler) {}
		instruction_t(uint8_t opcode0, uint8_t opcode1, handler_t handler) : m_opcount(2), m_opcode0(opcode0), m_opcode1(opcode1), m_handler(handler) {}
		instruction_t(uint8_t opcode0, uint8_t opcode1, uint8_t opcode2, handler_t handler) : m_opcount(3), m_opcode0(opcode0), m_opcode1(opcode1), m_opcode2(opcode2), m_handler(handler) {}
		instruction_t(uint8_t opcode0, uint8_t opcode1, uint8_t opcode2, uint8_t opcode3, handler_t handler) : m_opcount(4), m_opcode0(opcode0), m_opcode1(opcode1), m_opcode2(opcode2), m_opcode3(opcode3), m_handler(handler) {}
		instruction_t(uint8_t opcode0, uint8_t opcode1, uint8_t opcode2, uint8_t opcode3, uint8_t opcode4, handler_t handler) : m_opcount(5), m_opcode0(opcode0), m_opcode1(opcode1), m_opcode2(opcode2), m_opcode3(opcode3), m_opcode4(opcode4), m_handler(handler) {}
		instruction_t(uint8_t opcode0, uint8_t opcode1, uint8_t opcode2, uint8_t opcode3, uint8_t opcode4, uint8_t opcode5, handler_t handler) : m_opcount(6), m_opcode0(opcode0), m_opcode1(opcode1), m_opcode2(opcode2), m_opcode3(opcode3), m_opcode4(opcode4), m_opcode5(opcode5), m_handler(handler) {}
		instruction_t(uint8_t opcode0, uint8_t opcode1, uint8_t opcode2, uint8_t opcode3, uint8_t opcode4, uint8_t opcode5, uint8_t opcode6, handler_t handler) : m_opcount(7), m_opcode0(opcode0), m_opcode1(opcode1), m_opcode2(opcode2), m_opcode3(opcode3), m_opcode4(opcode4), m_opcode5(opcode5), m_opcode6(opcode6), m_handler(handler) {}

		// Execute
		//
		// Attempts to execute the defined instruction handler
		bool Execute(context_t* context);

	private:

		instruction_t(const instruction_t&)=delete;
		instruction_t& operator=(const instruction_t&)=delete;

		uint8_t				m_opcode0 = 0;			// Opcode byte 0
		uint8_t				m_opcode1 = 0;			// Opcode byte 1
		uint8_t				m_opcode2 = 0;			// Opcode byte 2
		uint8_t				m_opcode3 = 0;			// Opcode byte 3
		uint8_t				m_opcode4 = 0;			// Opcode byte 4
		uint8_t				m_opcode5 = 0;			// Opcode byte 5
		uint8_t				m_opcode6 = 0;			// Opcode byte 6
		const uint8_t		m_opcount;				// Number of opcode bytes
		handler_t			m_handler;				// Function to execute instruction
	};

	// modrm_t
	//
	// Value type that processes the special ModR/M[+SIB] bytes of an instruction.
	// The interpretation of the fields depends on the context in which the ModR/M
	// value is being used.
	template<typename size_type> class modrm_t
	{
	public:

		// Instance Constructor
		//
		explicit modrm_t(context_t* context)
		{
			modrm_byte_t modrm(byte(context).Value);

			Opcode = modrm.reg;
			Displacement = PopDisplacement(context, modrm);
			EffectiveAddress = reinterpret_cast<size_type*>(Displacement);
			Register = GetRegister(context, modrm);
		}
	
		// Displacement
		//
		// Indicates the displacement specified by the ModR/M [R/M] field,
		// subject to interpretation by the caller
		uintptr_t Displacement;

		// EffectiveAddress
		//
		// Indicates the effective address specified by the ModR/M [R/M] field,
		// subject to interpretation by the caller
		size_type* EffectiveAddress;

		// Opcode
		//
		// Indicates the opcode specified by the ModR/M [REG] field, subject to
		// interpretation by the caller
		uint8_t Opcode;

		// Register
		//
		// Indicates the register specified by the ModR/M [REG] field, subject to
		// interpretation by the caller
		size_type* Register;

	private:

		// GetRegister (static)
		//
		// Gets a pointer to the register referened by the ModR/M byte
		static size_type* GetRegister(context_t* context, modrm_byte_t modrm)
		{
			// Get a pointer to the register referenced by the modrm.reg value
			switch(modrm.reg) {

				case 0x00: return reinterpret_cast<size_type*>(&context->Eax);		// EAX/AX/AL
				case 0x01: return reinterpret_cast<size_type*>(&context->Ecx);		// ECX/CX/CL
				case 0x02: return reinterpret_cast<size_type*>(&context->Edx);		// EDX/DX/DL
				case 0x03: return reinterpret_cast<size_type*>(&context->Ebx);		// EBX/BX/BL

				case 0x04:										// ESP/SP/AH
					if(sizeof(size_type) > sizeof(uint8_t)) return reinterpret_cast<size_type*>(&context->Esp);
					else return reinterpret_cast<size_type*>(&context->Eax) + 1;

				case 0x05:										// EBP/BP/CH
					if(sizeof(size_type) > sizeof(uint8_t)) return reinterpret_cast<size_type*>(&context->Ebp);
					else return reinterpret_cast<size_type*>(&context->Ecx) + 1;

				case 0x06:										// ESI/SI/DH
					if(sizeof(size_type) > sizeof(uint8_t)) return reinterpret_cast<size_type*>(&context->Esi);
					else return reinterpret_cast<size_type*>(&context->Edx) + 1;

				case 0x07:										// EDI/DI/BH
					if(sizeof(size_type) > sizeof(uint8_t)) return reinterpret_cast<size_type*>(&context->Edi);
					else return reinterpret_cast<size_type*>(&context->Ebx) + 1;
			}

			return nullptr;
		}

		// PopDisplacement (static)
		//
		// Pops the effective address (displacement) referenced by the ModR/M byte
		static uintptr_t PopDisplacement(context_t* context, modrm_byte_t modrm)
		{
			// Switch on the Mod and R/M bits of the modr/m bytes (11000111 = 0xC7)
			switch (modrm.value & 0xC7) {

				// Mod 00
				case 0x00: return context->Eax;										// [EAX]
				case 0x01: return context->Ecx;										// [ECX]
				case 0x02: return context->Edx;										// [EDX]
				case 0x03: return context->Ebx;										// [EBX]
				case 0x04: return PopScaledIndex(context, modrm);					// [--][--]
				case 0x05: return disp32(context);									// disp32
				case 0x06: return context->Esi;										// [ESI]
				case 0x07: return context->Edi;										// [EDI]

				// Mod 01
				case 0x40: return context->Eax + disp8(context);					// [EAX] + disp8
				case 0x41: return context->Ecx + disp8(context);					// [ECX] + disp8
				case 0x42: return context->Edx + disp8(context);					// [EDX] + disp8
				case 0x43: return context->Ebx + disp8(context);					// [EBX] + disp8
				case 0x44: return PopScaledIndex(context, modrm) + disp8(context);	// [--][--] + disp8
				case 0x45: return context->Ebp + disp8(context);					// [EBP] + disp8
				case 0x46: return context->Esi + disp8(context);					// [ESI] + disp8
				case 0x47: return context->Edi + disp8(context);					// [EDI] + disp8

				// Mod 10
				case 0x80: return context->Eax + disp32(context);					// [EAX] + disp32
				case 0x81: return context->Ecx + disp32(context);					// [EDX] + disp32
				case 0x82: return context->Edx + disp32(context);					// [ECX] + disp32
				case 0x83: return context->Ebx + disp32(context);					// [EBX] + disp32
				case 0x84: return PopScaledIndex(context, modrm) + disp32(context);	// [--][--] + disp32
				case 0x85: return context->Ebp + disp32(context);					// [EBP] + disp32
				case 0x86: return context->Esi + disp32(context);					// [ESI] + disp32
				case 0x87: return context->Edi + disp32(context);					// [EDI] + disp32

				// Mod 11 (Register-Direct)
				//
				// Register-Direct mode may access different registers depending on the operation.
				// Returns a pointer to the context structure register field, which happens to be
				// an offset into the data segment so this should behave as expected
				//
				// r32 -> EAX / ECX / EDX / EBX / ESP / EBP / ESI / EDI
				// r16 -> AX  / CX  / DX  / BX  / SP  / BP  / SI  / DI
				// r8  -> AL  / CL  / DL  / BL  / AH  / CH  / DH  / BH
	
				case 0xC0: return uintptr_t(&context->Eax);							// EAX/AX/AL
				case 0xC1: return uintptr_t(&context->Ecx);							// ECX/CX/CL
				case 0xC2: return uintptr_t(&context->Edx);							// EDX/DX/DL
				case 0xC3: return uintptr_t(&context->Ebx);							// EBX/BX/BL

				case 0xC4:															// ESP/SP/AH
					if(sizeof(size_type) > sizeof(uint8_t)) return uintptr_t(&context->Esp);
					else return uintptr_t(&context->Eax) + 1;

				case 0xC5:															// EBP/BP/CH
					if(sizeof(size_type) > sizeof(uint8_t)) return uintptr_t(&context->Ebp);
					else return uintptr_t(&context->Ecx) + 1;

				case 0xC6:															// ESI/SI/DH
					if(sizeof(size_type) > sizeof(uint8_t)) return uintptr_t(&context->Esi);
					else return uintptr_t(&context->Edx) + 1;

				case 0xC7:															// EDI/DI/BH
					if(sizeof(size_type) > sizeof(uint8_t)) return uintptr_t(&context->Edi);
					else return uintptr_t(&context->Ebx) + 1;
			}

			return 0;
		}

		// PopScaledIndex (static)
		//
		// Pops the scaled effective address based on the value of an SIB byte
		static uintptr_t PopScaledIndex(context_t* context, modrm_byte_t modrm)
		{
			uintptr_t effectiveaddr;			// Calculated effective address

			// Pop the SIB byte from the instruction queue
			sib_byte_t sib(byte(context).Value);	
	
			// scale * index
			switch(sib.index) {

				case 0x00: effectiveaddr = (context->Eax << sib.scale); break;
				case 0x01: effectiveaddr = (context->Ecx << sib.scale); break;
				case 0x02: effectiveaddr = (context->Edx << sib.scale); break;
				case 0x03: effectiveaddr = (context->Ebx << sib.scale); break;
				case 0x04: effectiveaddr = 0; break;
				case 0x05: effectiveaddr = (context->Ebp << sib.scale); break;
				case 0x06: effectiveaddr = (context->Esi << sib.scale); break;
				case 0x07: effectiveaddr = (context->Edi << sib.scale); break;
			}

			// + base + offset
			//
			// Special Case: When base is 0x05, the interpretation depends on the
			// mod value of the ModR/M byte. When MOD=0x00, the base is a 32-bit 
			// displacement. When MOD=0x01 or 0x02, the base is the EBX register
			switch(sib.base) {

				case 0x00: effectiveaddr += context->Eax; break;
				case 0x01: effectiveaddr += context->Ecx; break;
				case 0x02: effectiveaddr += context->Edx; break;
				case 0x03: effectiveaddr += context->Ebx; break;
				case 0x04: effectiveaddr += context->Esp; break;
				case 0x05: 
					// SPECIAL CASE: if mod is 00b, the base is a disp32 value.  Otherwise
					// if mod is 01b or 10b, the base is the EBP register.
					if(modrm.mod == 0x00) effectiveaddr += disp32(context);
					else if(modrm.mod == 0x01 || modrm.mod == 0x02) effectiveaddr += context->Ebp; 
					break;
				case 0x06: effectiveaddr += context->Esi; break;
				case 0x07: effectiveaddr += context->Edi; break;
			}

			return effectiveaddr;
		}
	};

};	// namespace emulator

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __EMULATOR_H_
