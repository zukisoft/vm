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
#include "ModRM.h"
#include "syscalls.h"

// t_gs (TLS)
//
// Emulated GS register
__declspec(thread) static uint16_t t_gs = 0;

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

	// Grab the ModR/M byte and verify that the target register is GS
	ModRM modrm(context.PopValue<uint8_t>());
	if(modrm.reg != 0x05) return false;

	// Replace the value stored in the virtual GS register on this thread
	t_gs = *modrm.GetEffectiveAddress<uint16_t>(context);

	return true;
});

// 66 8E/r : MOV Sreg,r/m16 (operand size override)
Instruction MOV16_GS_RM16(0x66, 0x8E, [](ContextRecord& context) -> bool {

	// Grab the ModR/M byte and verify that the target register is GS
	ModRM modrm(context.PopValue<uint8_t>());
	if(modrm.reg != 0x05) return false;

	// Replace the value stored in the virtual GS register on this thread
	t_gs = *modrm.GetEffectiveAddress<uint16_t>(context);

	return true;
});

// 65 8B : MOV r32,GS:[r/m32]
Instruction MOV_R32_GS_RM32(0x65, 0x8B, [](ContextRecord& context) -> bool {

	ModRM modrm(context.PopValue<uint8_t>());
	uint32_t value = ReadGS<uint32_t>(*modrm.GetEffectiveAddress<uint32_t>(context));

	switch(modrm.reg) {

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

	ModRM modrm(context.PopValue<uint8_t>());
	uint16_t value = ReadGS<uint16_t>(*modrm.GetEffectiveAddress<uint32_t>(context));

	switch(modrm.reg) {

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

	context.Registers.EAX = ReadGS<uint32_t>(context.PopValue<uint32_t>());
	return true;
});

// 66 65 A1 : MOV AX,GS:moffs32
Instruction MOV_AX_GS_MOFFS32(0x66, 0x65, 0xA1, [](ContextRecord& context) -> bool {

	context.Registers.AX = ReadGS<uint16_t>(context.PopValue<uint32_t>());
	return true;
});

// 65 83 : CMP GS:[xxxxxx],imm8
Instruction CMP_GS_RM32_IMM8(0x65, 0x83, [](ContextRecord& context) -> bool {

	ModRM modrm(context.PopValue<uint8_t>());

	// RHS is sign-extended to match the bit length of LHS
	int32_t lhs = ReadGS<int32_t>(*modrm.GetEffectiveAddress<uint32_t>(context));
	int32_t rhs = context.PopValue<uint8_t>();

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

		// MOV r32,GS:[r/m32]
		// MOV r16,GS:[r/m32]
		if(MOV_R32_GS_RM32.Execute(context)) return EXCEPTION_CONTINUE_EXECUTION;
		if(MOV_R16_GS_RM32.Execute(context)) return EXCEPTION_CONTINUE_EXECUTION;

		// CMP GS:[xxxxxx],imm8
		if(CMP_GS_RM32_IMM8.Execute(context)) return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

//-----------------------------------------------------------------------------
