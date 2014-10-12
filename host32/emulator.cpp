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
#include "emulator.h"
#include <linux/errno.h>
#include "syscalls.h"

namespace emulator {

	//-------------------------------------------------------------------------
	// instruction_t::Execute
	//
	// Executes the instruction by invoking the defined lambda function
	//
	// Arguments:
	//
	//	context			- Execution context record

	bool instruction_t::Execute(context_t* context)
	{
		uint8_t* eip = reinterpret_cast<uint8_t*>(context->Eip);

		// This should work better than a loop; each case falls through
		switch(m_opcount) {

			case 7: if(m_opcode6 != eip[6]) return false;	// fall through
			case 6: if(m_opcode5 != eip[5]) return false;	// fall through
			case 5: if(m_opcode4 != eip[4]) return false;	// fall through
			case 4: if(m_opcode3 != eip[3]) return false;	// fall through
			case 3: if(m_opcode2 != eip[2]) return false;	// fall through
			case 2: if(m_opcode1 != eip[1]) return false;	// fall through
			case 1: if(m_opcode0 != eip[0]) return false;	break;
			default: return false;
		}

		// Move the instruction pointer to the first byte after the opcodes
		context->Eip += m_opcount;
	
		// If execution of the instruction fails, restore the instruction pointer
		if(m_handler(context)) return true;
		else { context->Eip = reinterpret_cast<uint32_t>(eip); return false; }
	}

};	// namespace emulator


///////////////////
// GS STUFF
//
// MOVE THESE OUT INTO THEIR OWN FILE(S)

// t_gs (TLS)
//
// Emulated GS register
__declspec(thread) static uint16_t t_gs = 0;

// TODO: ADD _DEBUG_TLS macro (or use _DEBUG) to dump everything that's 
// happening with thread local storage; it's hard to diagnose


//-----------------------------------------------------------------------------
// ReadGS<T>
//
// Reads a value from the emulated GS segment
//
// Arguments:
//
//	offset		- Offset into the emulated segment

template <typename T> T ReadGS(uint32_t offset)
{
	_ASSERTE(t_gs != 0);		// Should be set by set_thread_area()

	uintptr_t slot = ((uintptr_t(t_gs) - 3) >> 3) >> 8;
	uintptr_t address = uintptr_t(TlsGetValue(slot)) + offset;
	return *reinterpret_cast<T*>(address);
}

//-----------------------------------------------------------------------------
// WriteGS<T>
//
// Writes a value into the emulated GS segment
//
// Arguments:
//
//	offset		- Offset into the emulated segment

template <typename T> void WriteGS(T value, uint32_t offset)
{
	_ASSERTE(t_gs != 0);		// Should be set by set_thread_area()

	uintptr_t slot = ((uintptr_t(t_gs) - 3) >> 3) >> 8;
	uintptr_t address = uintptr_t(TlsGetValue(slot)) + offset;
	*reinterpret_cast<T*>(address) = value;
}

//-----------------------------------------------------------------------------
// Exception Handler Instructions
//
// MOVE THESE OUT INTO THEIR OWN FILE(S)
//-----------------------------------------------------------------------------

// CD 80 : INT 80
//
emulator::instruction_t INT_80(0xCD, 0x80, [](emulator::context_t* context) -> bool {

	// There are only 512 system call slots defined	
	if(context->Eax > 511) { context->Eax = -LINUX_ENOSYS; return true; }

	// The system call number is stored in the EAX register on entry and
	// the return value from the function is stored in EAX on exit
	syscall_t func = g_syscalls[context->Eax];
	context->Eax = static_cast<DWORD>((func) ? func(context) : -LINUX_ENOSYS);

	return true;
});

// REMAINING OPERATIONS ARE LISTED IN NUMERICAL ORDER BY PRIMARY OPCODE(S) (excludes prefixes)

// 65 33 : XOR r32, GS:[r/m32]
//
emulator::instruction_t XOR_R32_GSRM32(0x65, 0x33, [](emulator::context_t* context) -> bool {

	emulator::rm32 modrm(context);

	uint32_t value = ReadGS<uint32_t>(modrm.Displacement);

	// todo: Decide on a standard format for eax, ebx, ecx, edx ... docs say to avoid ebx
	__asm mov eax, value
	__asm mov ebx, modrm.Register
	__asm mov edx, context

	__asm push [edx]context.EFlags
	__asm popfd
	
	__asm xor [ebx], eax
	
	__asm pushfd
	__asm pop [edx]context.EFlags

	return true;
});

// 65 83 : CMP GS:[r/m32], imm8
//
emulator::instruction_t CMP_GSRM32_IMM8(0x65, 0x83, [](emulator::context_t* context) -> bool {

	emulator::rm32 modrm(context);

	int32_t rhs = ReadGS<int32_t>(modrm.Displacement);
	int32_t lhs = emulator::imm8(context);					// <--- sign-extend

	__asm mov edx, context
	__asm push [edx]context.EFlags;
	__asm popfd

	// Execute the operation
	__asm mov eax, rhs
	__asm cmp eax, lhs

	__asm pushfd
	__asm pop [edx]context.EFlags;

	return true;
});

// 65 89 : MOV GS:[r/m32], r32
//
emulator::instruction_t MOV_GSRM32_R32(0x65, 0x89, [](emulator::context_t* context) -> bool {

	emulator::rm32 modrm(context);

	WriteGS<uint32_t>(*modrm.Register, modrm.Displacement);
	return true;
});

// 65 8B : MOV r32, GS:[r/m32]
//
emulator::instruction_t MOV_R32_GSRM32(0x65, 0x8B, [](emulator::context_t* context) -> bool {

	emulator::rm32 modrm(context);

	*modrm.Register = ReadGS<uint32_t>(modrm.Displacement);
	return true;
});

// 8E : MOV Sreg, r/m16
//
emulator::instruction_t MOV_SREG_RM16(0x8E, [](emulator::context_t* context) -> bool {

	emulator::rm16 modrm(context);
	if(modrm.Opcode != 0x05) return false;			// 0x05 --> GS

	t_gs = *modrm.EffectiveAddress;
	return true;
});

// 65 A1 : MOV EAX, GS:moffs32
//
emulator::instruction_t MOV_EAX_GSMOFFS32(0x65, 0xA1, [](emulator::context_t* context) -> bool {

	context->Eax = ReadGS<uint32_t>(emulator::moffs32(context));
	return true;
});

// 65 A3 : MOV GS:moffs32, EAX
//
emulator::instruction_t MOV_GSMOFFS32_EAX(0x65, 0xA3, [](emulator::context_t* context) -> bool {
	
	WriteGS<uint32_t>(context->Eax, emulator::moffs32(context));
	return true;
});

// 65 C7 : MOV GS:[r/m32], imm32
//
emulator::instruction_t MOV_GSRM32_IMM32(0x65, 0xC7, [](emulator::context_t* context) -> bool {
	
	emulator::rm32 modrm(context);

	WriteGS<int32_t>(emulator::imm32(context), modrm.Displacement);
	return true;
});


//// 66 65 8B : MOV r16,GS:[r/m32]
//Instruction MOV_R16_GS_RM32(0x66, 0x65, 0x8B, [](ContextRecord& context) -> bool {
//
//	auto modrm = context.PopModRM<uint32_t>();
//	uint32_t value = ReadGS<uint16_t>(modrm.Displacement);
//
//	switch(modrm.Reg) {
//
//		case 0x00: context.Registers.AX = value; break;
//		case 0x01: context.Registers.CX = value; break;
//		case 0x02: context.Registers.DX = value; break;
//		case 0x03: context.Registers.BX = value; break;
//		case 0x04: context.Registers.SP = value; break;
//		case 0x05: context.Registers.BP = value; break;
//		case 0x06: context.Registers.SI = value; break;
//		case 0x07: context.Registers.DI = value; break;
//	}
//
//	return true;
//});
//
//
//// 66 65 A1 : MOV AX,GS:moffs32
//Instruction MOV_AX_GS_MOFFS32(0x66, 0x65, 0xA1, [](ContextRecord& context) -> bool {
//
//	context.Registers.AX = ReadGS<uint16_t>(context.PopOffset<uint32_t>());
//	return true;
//});


//-----------------------------------------------------------------------------
// EmulationExceptionHandler
//
// Intercepts and processes a 32-bit Linux system call using a vectored exception
// handler.  Technique is based on a sample presented by proog128 available at:
// http://0xef.wordpress.com/2012/11/17/emulate-linux-system-calls-on-windows/
//
// Arguments:
//
//	exception		- Exception information

LONG CALLBACK EmulationExceptionHandler(PEXCEPTION_POINTERS exception)
{
	// All the exceptions that are handled here start as access violations
	if(exception->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {

		// INT 80
		if(INT_80.Execute(exception->ContextRecord)) return EXCEPTION_CONTINUE_EXECUTION;

		// MOV GS, r/m16
		if(MOV_SREG_RM16.Execute(exception->ContextRecord)) return EXCEPTION_CONTINUE_EXECUTION;
		//if(MOV16_GSRM16.Execute(exception->ContextRecord)) return EXCEPTION_CONTINUE_EXECUTION;

		// MOV EAX, GS:moffs32
		// MOV AX, GS:moffs32
		if(MOV_EAX_GSMOFFS32.Execute(exception->ContextRecord)) return EXCEPTION_CONTINUE_EXECUTION;
		//if(MOV_AX_GSMOFFS32.Execute(exception->ContextRecord)) return EXCEPTION_CONTINUE_EXECUTION;

		// MOV GS:moffs32, EAX
		if(MOV_GSMOFFS32_EAX.Execute(exception->ContextRecord)) return EXCEPTION_CONTINUE_EXECUTION;

		if(MOV_GSRM32_IMM32.Execute(exception->ContextRecord)) return EXCEPTION_CONTINUE_EXECUTION;

		// MOV GS:[r/m32], r32
		if(MOV_GSRM32_R32.Execute(exception->ContextRecord)) return EXCEPTION_CONTINUE_EXECUTION;

		// MOV r32, GS:[r/m32]
		// MOV r16, GS:[r/m32]
		if(MOV_R32_GSRM32.Execute(exception->ContextRecord)) return EXCEPTION_CONTINUE_EXECUTION;
		//if(MOV_R16_GSRM32.Execute(exception->ContextRecord)) return EXCEPTION_CONTINUE_EXECUTION;

		// CMP GS:[xxxxxx],imm8
		if(CMP_GSRM32_IMM8.Execute(exception->ContextRecord)) return EXCEPTION_CONTINUE_EXECUTION;

		if(XOR_R32_GSRM32.Execute(exception->ContextRecord)) return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

//-----------------------------------------------------------------------------
