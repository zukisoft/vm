//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
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
#include "syscalls.h"
#include <linux/errno.h>

// g_ldt (main.cpp)
//
// Pointer to the process-wide local descriptor table
extern void* g_ldt;

// t_gs (main.cpp)
//
// Thread-local emulated GS segment register
extern __declspec(thread) uint16_t t_gs;

// trace.cpp
//
void TraceMessage(const char_t* message, size_t length);

// GS<>
//
// Accesses a value in the emulated GS segment via an offset
template<typename size_type> inline size_type& GS(uintptr_t offset)
{
	// In order to try and prevent running into a real LDT, the high bit of the
	// slot is always set, limiting this to 4096 slots.  Strip out the high bit
	// and then the lowest 3 bits to get the actual offset
	uint16_t slot = (t_gs >> 3) & ~LINUX_LDT_ENTRIES;

	// Return a reference to the specified offset as the requested data type
	uapi::user_desc32* ldt = reinterpret_cast<uapi::user_desc32*>(g_ldt);
	return *reinterpret_cast<size_type*>(uintptr_t(ldt[slot].base_addr) + offset);
}

// InvokeSystemCall
//
// Invokes a system call from the global syscall table.  Pulled out of INT_80
// emulation handler due to an internal compiler error when __try/__except are
// used inside a lambda function
//
DWORD InvokeSystemCall(int syscall, emulator::context_t* context)
{
	// There are only 512 system call slots defined
	_ASSERTE(syscall < 512);
	if(syscall > 511) return -LINUX_ENOSYS;

	// Grab the function pointer for the system call
	syscall_t func = g_syscalls[syscall];
	if(func == nullptr) return -LINUX_ENOSYS;

	// Invoke the system call in a __try/__except to catch RPC errors like null
	// ref pointers and whatnot and translate them to EFAULT for the application
	__try { return static_cast<DWORD>(func(context)); }
	__except(EXCEPTION_EXECUTE_HANDLER) { 
		
		// todo: this _RPTF2() needs to be removed
		_RPTF2(_CRT_ASSERT, "System call %d: Unhandled exception 0x%08X\r\n", 123, GetExceptionCode());
		return -LINUX_EFAULT; 
	}
}

//
// SYSTEM CALL EMULATION INSTRUCTIONS
//

// CD 80 : INT 80
//
emulator::instruction INT_80(0xCD, 0x80, [](emulator::context_t* context) -> bool {

	int syscall = static_cast<int>(context->Eax);
	// todo: this _RPT2 needs to be removed
	_RPT2(_CRT_WARN, "0x%04X: System call %d\r\n", GetCurrentProcessId(), syscall);

	// The system call number is stored in the EAX register on entry and
	// the return value from the function is stored in EAX on exit
	context->Eax = InvokeSystemCall(syscall, context);

	// TODO: remove this at some point, it's only going to be useful until
	// the applications can just report it on their own
	if(static_cast<int>(context->Eax) < 0) {
		// todo: this _RPT2 needs to be removed
		_RPT2(_CRT_WARN, "System call %d: result = %d\r\n", syscall, context->Eax);
	}

	return true;
});

//
// GS SEGMENT EMULATION INSTRUCTIONS
//

// 65 03 : ADD r32, GS:[r/m32]
//
emulator::instruction ADD_R32_GSRM32(0x65, 0x03, [](emulator::context_t* context) -> bool {

	emulator::rm32 modrm(context);

	uint32_t value = GS<uint32_t>(modrm.Displacement);

	__asm mov esi, context
	__asm push [esi]context.EFlags
	__asm popfd
	
	// ADD modrm.Register, value
	__asm mov edi, modrm.Register
	__asm mov eax, value
	__asm add [edi], eax
	
	__asm pushfd
	__asm pop [esi]context.EFlags

	return true;
});

// 65 33 : XOR r32, GS:[r/m32]
//
emulator::instruction XOR_R32_GSRM32(0x65, 0x33, [](emulator::context_t* context) -> bool {

	emulator::rm32 modrm(context);

	uint32_t value = GS<uint32_t>(modrm.Displacement);

	__asm mov esi, context
	__asm push [esi]context.EFlags
	__asm popfd
	
	// XOR modrm.Register, value
	__asm mov edi, modrm.Register
	__asm mov eax, value
	__asm xor [edi], eax
	
	__asm pushfd
	__asm pop [esi]context.EFlags

	return true;
});

// 65 83 : CMP GS:[r/m32], imm8
//
emulator::instruction CMP_GSRM32_IMM8(0x65, 0x83, [](emulator::context_t* context) -> bool {

	emulator::rm32 modrm(context);

	int32_t rhs = GS<int32_t>(modrm.Displacement);
	int32_t lhs = emulator::imm8(context);				// <--- sign-extend

	__asm mov esi, context
	__asm push [esi]context.EFlags;
	__asm popfd

	// CMP rhs, lhs
	__asm mov eax, rhs
	__asm cmp eax, lhs

	__asm pushfd
	__asm pop [esi]context.EFlags;

	return true;
});

// 65 89 : MOV GS:[r/m32], r32
//
emulator::instruction MOV_GSRM32_R32(0x65, 0x89, [](emulator::context_t* context) -> bool {

	emulator::rm32 modrm(context);

	GS<uint32_t>(modrm.Displacement) = *modrm.Register;
	return true;
});

// 65 8B : MOV r32, GS:[r/m32]
//
emulator::instruction MOV_R32_GSRM32(0x65, 0x8B, [](emulator::context_t* context) -> bool {

	emulator::rm32 modrm(context);

	*modrm.Register = GS<uint32_t>(modrm.Displacement);
	return true;
});

// 8E : MOV Sreg, r/m16
//
emulator::instruction MOV_SREG_RM16(0x8E, [](emulator::context_t* context) -> bool {

	emulator::rm16 modrm(context);
	if(modrm.Opcode != 0x05) return false;			// 0x05 --> GS

	t_gs = *modrm.EffectiveAddress;
	return true;
});

// 65 A1 : MOV EAX, GS:moffs32
//
emulator::instruction MOV_EAX_GSMOFFS32(0x65, 0xA1, [](emulator::context_t* context) -> bool {

	context->Eax = GS<uint32_t>(emulator::moffs32(context));
	return true;
});

// 65 A3 : MOV GS:moffs32, EAX
//
emulator::instruction MOV_GSMOFFS32_EAX(0x65, 0xA3, [](emulator::context_t* context) -> bool {
	
	GS<uint32_t>(emulator::moffs32(context)) = context->Eax;
	return true;
});

// 65 C7 : MOV GS:[r/m32], imm32
//
emulator::instruction MOV_GSRM32_IMM32(0x65, 0xC7, [](emulator::context_t* context) -> bool {
	
	emulator::rm32 modrm(context);

	GS<uint32_t>(modrm.Displacement) = emulator::imm32(context);
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
	// All the exceptions that are handled here in the emulator are access violations
	if(exception->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {

		// System Call Emulation
		if(INT_80(exception->ContextRecord))			return EXCEPTION_CONTINUE_EXECUTION;

		// GS Segment Register Emulations
		if(ADD_R32_GSRM32(exception->ContextRecord))	return EXCEPTION_CONTINUE_EXECUTION;
		if(CMP_GSRM32_IMM8(exception->ContextRecord))	return EXCEPTION_CONTINUE_EXECUTION;
		if(MOV_EAX_GSMOFFS32(exception->ContextRecord)) return EXCEPTION_CONTINUE_EXECUTION;
		if(MOV_GSMOFFS32_EAX(exception->ContextRecord))	return EXCEPTION_CONTINUE_EXECUTION;
		if(MOV_GSRM32_IMM32(exception->ContextRecord))	return EXCEPTION_CONTINUE_EXECUTION;
		if(MOV_GSRM32_R32(exception->ContextRecord))	return EXCEPTION_CONTINUE_EXECUTION;
		if(MOV_R32_GSRM32(exception->ContextRecord))	return EXCEPTION_CONTINUE_EXECUTION;
		if(MOV_SREG_RM16(exception->ContextRecord))		return EXCEPTION_CONTINUE_EXECUTION;
		if(XOR_R32_GSRM32(exception->ContextRecord))	return EXCEPTION_CONTINUE_EXECUTION;

#ifdef _DEBUG

		// 0x65 or 0x6566 - report an unhandled GS segment override prefix instruction
		if((*reinterpret_cast<uint8_t*>(exception->ContextRecord->Eip) == 0x65) || (*reinterpret_cast<uint16_t*>(exception->ContextRecord->Eip) == 0x6566)) {
			
			uint8_t* bytes = reinterpret_cast<uint8_t*>(exception->ContextRecord->Eip);
			// TODO: This _RPTF4() needs to be removed; it won't work right
			_RPTF4(_CRT_ASSERT, "Unhandled GS segment override prefix instruction: 0x%02X 0x%02X 0x%02X 0x%02X\r\n", bytes[0], bytes[1], bytes[2], bytes[3]);
		}
#endif
	}

	// 0x40010006 -- DBG_PRINTEXCEPTION_C
	//
	else if(exception->ExceptionRecord->ExceptionCode == DBG_PRINTEXCEPTION_C) {

		// OutputDebugString and _RPTFx(_CRT_WARN, ...) are flaky when called from within a vectored exception
		// handler and can cause a stack overflow or just plain nuke the process.  This catches the attempt to 
		// write a message and sends it to the custom trace handler instead
		TraceMessage(reinterpret_cast<char*>(exception->ExceptionRecord->ExceptionInformation[1]), exception->ExceptionRecord->ExceptionInformation[0]);
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	// 0xC00000096 -- STATUS_PRIVILEGED_INSTRUCTION
	//
	else if(exception->ExceptionRecord->ExceptionCode == STATUS_PRIVILEGED_INSTRUCTION) {

		// TODO: If this is an emulated thread, need to call exit() rather than just dying
		// to properly unregister it from the service

		// TODO: This happens when HLT is called; should likely be removed once proper exiting
		// of the hosted application has been established
		ExitThread(0xC0000096);
	}

	// TODO: This should kill a virtual machine thread as an unhandled exception,
	// which would also deal with the STATUS_PRIVILEGED_INSTRUCTION above although
	// the result code may be different - see how GLIB deals with unhandled exceptions
	return EXCEPTION_CONTINUE_SEARCH;
}

//-----------------------------------------------------------------------------
