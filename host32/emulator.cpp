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
#include <linux/errno.h>
#include "ContextRecord.h"
#include "Instruction.h"
#include "syscalls.h"

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

// CD/b 80 : INT 80
Instruction INT_80(0xCD, 0x80, [](ContextRecord& context) -> bool {

#ifdef _DEBUG
	// Get the system call number to show up when the breakpoint below triggers
	uint32_t syscall = context.Registers.EAX;
#endif

	// There are slots implemented for only 512 system call ordinals
	if(syscall > 511) { context.Registers.EAX = -LINUX_ENOSYS; return true; }

	// The system call number is stored in the EAX register on entry and
	// the return value from the function is stored in EAX on exit
	syscall_t func = g_syscalls[context.Registers.EAX];
	context.Registers.EAX = static_cast<DWORD>((func) ? func(context) : -LINUX_ENOSYS);

#ifdef _DEBUG

	// Keep track of things that fail in the debug output (ENOSYS = -38)
	if(static_cast<int32_t>(context.Registers.EAX) < 0)
		_RPTF2(_CRT_WARN, "System Call %d Failed: EAX = %d\n", syscall, context.Registers.EAX);

#endif

	return true;						// Always considered successful
});

// 8E/r : MOV Sreg,r/m16
Instruction MOV_GS_RM16(0x8E, [](ContextRecord& context) -> bool {

	auto modrm = context.PopModRM<uint16_t>();
	if(modrm.Reg != 0x05) return false;

	// Replace the value stored in the virtual GS register on this thread
	t_gs = *reinterpret_cast<uint16_t*>(modrm.Pointer);

	return true;
});

// 66 8E/r : MOV Sreg,r/m16 (operand size override)
Instruction MOV16_GS_RM16(0x66, 0x8E, [](ContextRecord& context) -> bool {

	auto modrm = context.PopModRM<uint16_t>();
	if(modrm.Reg != 0x05) return false;

	// Replace the value stored in the virtual GS register on this thread
	t_gs = *reinterpret_cast<uint16_t*>(modrm.Pointer);

	return true;
});

// 65 89 : MOV GS:[r/m32],r32
Instruction MOV_GS_RM32_R32(0x65, 0x89, [](ContextRecord& context) -> bool {

	auto modrm = context.PopModRM<uint32_t>();
	uint32_t value = 0;

	switch(modrm.Reg) {

		case 0x00: value = context.Registers.EAX; break;
		case 0x01: value = context.Registers.ECX; break;
		case 0x02: value = context.Registers.EDX; break;
		case 0x03: value = context.Registers.EBX; break;
		case 0x04: value = context.Registers.ESP; break;
		case 0x05: value = context.Registers.EBP; break;
		case 0x06: value = context.Registers.ESI; break;
		case 0x07: value = context.Registers.EDI; break;
	}

	WriteGS<uint32_t>(value, modrm.Displacement);
	return true;
});

// 65 8B : MOV r32,GS:[r/m32]
Instruction MOV_R32_GS_RM32(0x65, 0x8B, [](ContextRecord& context) -> bool {

	auto modrm = context.PopModRM<uint32_t>();
	uint32_t value = ReadGS<uint32_t>(modrm.Displacement);

	switch(modrm.Reg) {

		case 0x00: context.Registers.EAX = value; break;
		case 0x01: context.Registers.ECX = value; break;
		case 0x02: context.Registers.EDX = value; break;
		case 0x03: context.Registers.EBX = value; break;
		case 0x04: context.Registers.ESP = value; break;
		case 0x05: context.Registers.EBP = value; break;
		case 0x06: context.Registers.ESI = value; break;
		case 0x07: context.Registers.EDI = value; break;
	}

	return true;
});

// 66 65 8B : MOV r16,GS:[r/m32]
Instruction MOV_R16_GS_RM32(0x66, 0x65, 0x8B, [](ContextRecord& context) -> bool {

	auto modrm = context.PopModRM<uint32_t>();
	uint32_t value = ReadGS<uint16_t>(modrm.Displacement);

	switch(modrm.Reg) {

		case 0x00: context.Registers.AX = value; break;
		case 0x01: context.Registers.CX = value; break;
		case 0x02: context.Registers.DX = value; break;
		case 0x03: context.Registers.BX = value; break;
		case 0x04: context.Registers.SP = value; break;
		case 0x05: context.Registers.BP = value; break;
		case 0x06: context.Registers.SI = value; break;
		case 0x07: context.Registers.DI = value; break;
	}

	return true;
});

// 65 A1 : MOV EAX,GS:moffs32
Instruction MOV_EAX_GS_MOFFS32(0x65, 0xA1, [](ContextRecord& context) -> bool {

	// moffs32
	context.Registers.EAX = ReadGS<uint32_t>(context.PopOffset<uint32_t>());
	return true;
});

// 66 65 A1 : MOV AX,GS:moffs32
Instruction MOV_AX_GS_MOFFS32(0x66, 0x65, 0xA1, [](ContextRecord& context) -> bool {

	context.Registers.AX = ReadGS<uint16_t>(context.PopOffset<uint32_t>());
	return true;
});

// 65 A3 : MOV GS:moffs32,EAX
Instruction MOV_GS_MOFFS32_EAX(0x65, 0xA3, [](ContextRecord& context) -> bool {
	
	WriteGS<uint32_t>(context.Registers.EAX, context.PopOffset<uint32_t>());
	return true;
});

// 65 C7 : MOV GS:[r/m32],imm32
Instruction MOV_GS_RM32_IMM32(0x65, 0xC7, [](ContextRecord& context) -> bool {
	
	auto modrm = context.PopModRM<uint32_t>();
	WriteGS<int32_t>(context.PopImmediate<int32_t>(), modrm.Displacement);
	return true;
});

// 65 83 : CMP GS:[r/m32],imm8
Instruction CMP_GS_RM32_IMM8(0x65, 0x83, [](ContextRecord& context) -> bool {

	auto modrm = context.PopModRM<uint32_t>();

	int32_t lhs = ReadGS<int32_t>(modrm.Displacement);
	int32_t rhs = context.PopImmediate<int8_t>();		// <-- sign-extend

	// Due to the flags, this operation is easier not to simulate
	__asm mov eax, lhs
	__asm cmp eax, rhs
	__asm pushfd
	__asm pop eax
	__asm mov rhs, eax

	// CMP instruction affects OF, SF, ZF, AF, PF and CF
	context.Flags.OF = ((rhs & 0x00000800) != 0);
	context.Flags.SF = ((rhs & 0x00000080) != 0);
	context.Flags.ZF = ((rhs & 0x00000040) != 0);
	context.Flags.AF = ((rhs & 0x00000010) != 0);
	context.Flags.PF = ((rhs & 0x00000004) != 0);
	context.Flags.CF = ((rhs & 0x00000001) != 0);

	return true;
});

// 65 33 : XOR r32,GS:[r/m32]
Instruction XOR_R32_GS_RM32(0x65, 0x33, [](ContextRecord& context) -> bool {

	auto modrm = context.PopModRM<uint32_t>();
	uint32_t value = ReadGS<uint32_t>(modrm.Displacement);

	switch(modrm.Reg) {

		case 0x00: context.Registers.EAX ^= value; break;
		case 0x01: context.Registers.ECX ^= value; break;
		case 0x02: context.Registers.EDX ^= value; break;
		case 0x03: context.Registers.EBX ^= value; break;
		case 0x04: context.Registers.ESP ^= value; break;
		case 0x05: context.Registers.EBP ^= value; break;
		case 0x06: context.Registers.ESI ^= value; break;
		case 0x07: context.Registers.EDI ^= value; break;
	}

	// TODO: FLAGS!!

	return true;
});

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

		// Wrap the low level context record in the ContextRecord class
		ContextRecord context(exception->ContextRecord);

		// INT 80
		if(INT_80.Execute(context)) return EXCEPTION_CONTINUE_EXECUTION;

		// MOV GS, r/m16
		if(MOV_GS_RM16.Execute(context)) return EXCEPTION_CONTINUE_EXECUTION;
		if(MOV16_GS_RM16.Execute(context)) return EXCEPTION_CONTINUE_EXECUTION;

		// MOV EAX, GS:moffs32
		// MOV AX, GS:moffs32
		if(MOV_EAX_GS_MOFFS32.Execute(context)) return EXCEPTION_CONTINUE_EXECUTION;
		if(MOV_AX_GS_MOFFS32.Execute(context)) return EXCEPTION_CONTINUE_EXECUTION;

		// MOV GS:moffs32, EAX
		if(MOV_GS_MOFFS32_EAX.Execute(context)) return EXCEPTION_CONTINUE_EXECUTION;

		if(MOV_GS_RM32_IMM32.Execute(context)) return EXCEPTION_CONTINUE_EXECUTION;

		// MOV GS:[r/m32], r32
		if(MOV_GS_RM32_R32.Execute(context)) return EXCEPTION_CONTINUE_EXECUTION;

		// MOV r32, GS:[r/m32]
		// MOV r16, GS:[r/m32]
		if(MOV_R32_GS_RM32.Execute(context)) return EXCEPTION_CONTINUE_EXECUTION;
		if(MOV_R16_GS_RM32.Execute(context)) return EXCEPTION_CONTINUE_EXECUTION;

		// CMP GS:[xxxxxx],imm8
		if(CMP_GS_RM32_IMM8.Execute(context)) return EXCEPTION_CONTINUE_EXECUTION;

		if(XOR_R32_GS_RM32.Execute(context)) return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

//-----------------------------------------------------------------------------
