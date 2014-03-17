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
#include "uapi.h"						// Include Linux UAPI declarations
#include "ContextRecord.h"				// Include ContextRecord declarations
#include "Instruction.h"				// Include Instruction declarations
#include "ModRM.h"						// Include ModRM declarations

//-----------------------------------------------------------------------------
// Type Declarations

// SYSCALL - Prototype for a system call handler
//
typedef int (*SYSCALL)(PCONTEXT context);

//-----------------------------------------------------------------------------
// Global Variables

// g_syscalls
//
// Table of all available system calls by ordinal
static SYSCALL		g_syscalls[512];

// g_handler
//
// Pointer returned from adding the vectored exception handler
static void*		g_handler = NULL;

// g_gsslot
//
// Thread local storage slot for the emulated GS register
static uint32_t		g_gsslot = TLS_OUT_OF_INDEXES;

// g_tlsbase / g_tlslength
//
// Pointer to and length of the default TLS information for this process
static uint32_t		g_tlsslot = TLS_OUT_OF_INDEXES;
static const void*	g_tlsbase = NULL;
static size_t		g_tlslength = 0;

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
	uintptr_t slot = (uintptr_t(TlsGetValue(g_gsslot)) - 3) >> 3;
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
	uintptr_t slot = (uintptr_t(TlsGetValue(g_gsslot)) - 3) >> 3;
	uintptr_t address = uintptr_t(TlsGetValue(slot)) + offset;
	*reinterpret_cast<T*>(address) = value;
}

//-----------------------------------------------------------------------------
// Exception Handler Instructions

// CD/b 80 : INT 80
Instruction INT_80(0xCD, 0x80, [](ContextRecord& context) -> bool {

	// The system call number is stored in the EAX register on entry and
	// the return value from the function is stored in EAX on exit
	SYSCALL func = g_syscalls[context.Registers.EAX];
	context.Registers.EAX = static_cast<DWORD>((func) ? func(context) : -LINUX_ENOSYS);

	return true;						// Always considered successful
});

// 8E/r : MOV Sreg,r/m16
Instruction MOV_GS_RM16(0x8E, [](ContextRecord& context) -> bool {

	// Grab the ModR/M byte and verify that the target register is GS
	ModRM modrm(context.PopValue<uint8_t>());
	if(modrm.reg != 0x05) return false;

	// The segment register value is 16-bit, but is stored as a 32-bit in TLS
	uint32_t value = *modrm.GetEffectiveAddress<uint16_t>(context);
	TlsSetValue(g_gsslot, reinterpret_cast<void*>(value));
	return true;
});

// 66 8E/r : MOV Sreg,r/m16 (operand size override)
Instruction MOV16_GS_RM16(0x66, 0x8E, [](ContextRecord& context) -> bool {

	// Grab the ModR/M byte and verify that the target register is GS
	ModRM modrm(context.PopValue<uint8_t>());
	if(modrm.reg != 0x05) return false;

	// The segment register value is 16-bit, but is stored as a 32-bit in TLS
	uint32_t value = *modrm.GetEffectiveAddress<uint16_t>(context);
	TlsSetValue(g_gsslot, reinterpret_cast<void*>(value));
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

	// gs contains a segment code generated from the slot number, convert back
	uintptr_t slot = (uintptr_t(TlsGetValue(g_gsslot)) - 3) >> 3;
	uintptr_t address = uintptr_t(TlsGetValue(slot)) + context.PopValue<uint32_t>();
	context.Registers.EAX = *reinterpret_cast<uint32_t*>(address);
	return true;
});

// 66 65 A1 : MOV AX,GS:moffs32
Instruction MOV_AX_GS_MOFFS32(0x66, 0x65, 0xA1, [](ContextRecord& context) -> bool {

	// gs contains a segment code generated from the slot number, convert back
	uintptr_t slot = (uintptr_t(TlsGetValue(g_gsslot)) - 3) >> 3;
	uintptr_t address = uintptr_t(TlsGetValue(slot)) + context.PopValue<uint32_t>();
	context.Registers.AX = *reinterpret_cast<uint16_t*>(address);
	return true;
});

//-----------------------------------------------------------------------------
// ExceptionHandler
//
// Intercepts and processes a 32-bit Linux system call using a vectored exception
// handler.  Technique is based on a sample presented by proog128 available at:
// http://0xef.wordpress.com/2012/11/17/emulate-linux-system-calls-on-windows/
//
// Arguments:
//
//	exception		- Exception information

LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS exception)
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
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

//-----------------------------------------------------------------------------
// InitializeTls
//
// Initializes a default thread local storage block for this process.  Should 
// be called before the hosted binary image is executed
//
// Arguments:
//
//	tlsbase		- Base address of the default TLS data in memory
//	tlslength	- Length of the default TLS data

DWORD InitializeTls(const void* tlsbase, size_t tlslength)
{
	// This can only be done one time per process
	if(g_tlsslot != TLS_OUT_OF_INDEXES) return ERROR_ALREADY_INITIALIZED;

	// This operation is pointless without data
	if((!tlsbase) || (tlslength == 0)) return ERROR_INVALID_PARAMETER;

	// Allocate a TLS slot for the default process data
	g_tlsslot = TlsAlloc();
	if(g_tlsslot == TLS_OUT_OF_INDEXES) return GetLastError();

	// Save the pointer and length for DllMain() thread attach notifications
	g_tlsbase = tlsbase;
	g_tlslength = tlslength;

	// Set up the TLS for the calling thread by cloning the data; additional
	// threads created afterwards are handled in DllMain()
	void* tlsdata = HeapAlloc(GetProcessHeap(), 0, g_tlslength);
	TlsSetValue(g_tlsslot, tlsdata);

	// If the allocation was successful, copy the tls memory image in
	if(tlsdata) CopyMemory(tlsdata, g_tlsbase, g_tlslength);

	// Change the emulated GS segment register to point at this slot
	TlsSetValue(g_gsslot, reinterpret_cast<void*>((g_tlsslot << 3) + 3));
	
	// The caller likely doesn't want to continue if the allocation failed
	return (tlsdata) ? ERROR_SUCCESS : ERROR_OUTOFMEMORY;
}

//-----------------------------------------------------------------------------
// DllMain
//
// Library entry point
//
// Arguments:
//
//	module			- DLL module handle
//	reason			- Reason DLLMain() is being invoked
//	reserved		- Reserved in Win32

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID)
{
	switch(reason) {

		case DLL_PROCESS_ATTACH:

			// The system calls are exported by ordinal from this DLL, which makes it
			// rather trivial to create a table of them ...
			for(int index = 0; index < 512; index++)
				g_syscalls[index] = reinterpret_cast<SYSCALL>(GetProcAddress(module, reinterpret_cast<LPCSTR>(index)));

			// Allocate the per-thread emulated GS register
			g_gsslot = TlsAlloc();
			if(g_gsslot == TLS_OUT_OF_INDEXES) return FALSE;

			// Emulation of instructions that fail are done with a vectored exception handler
			g_handler = AddVectoredExceptionHandler(1, ExceptionHandler);

			// fall through ...

		case DLL_THREAD_ATTACH:

			// If default TLS data has been provided, set that up and point
			// the emulated GS register to that TLS slot
			if(g_tlsslot != TLS_OUT_OF_INDEXES) {

				// Allocate the default TLS for this thread off the process heap
				void* tlsdata = HeapAlloc(GetProcessHeap(), 0, g_tlslength);
				if(!tlsdata) return FALSE;

				// Set the pointer to the default data and copy the template
				TlsSetValue(g_tlsslot, tlsdata);
				CopyMemory(tlsdata, g_tlsbase, g_tlslength);

				// Set the emulated GS register to point at this slot by default
				TlsSetValue(g_gsslot, reinterpret_cast<void*>((g_tlsslot << 3) + 3));
			}

			// No default TLS data; emulated GS register gets set to zero
			else TlsSetValue(g_gsslot, reinterpret_cast<void*>(0));

			break;

		case DLL_PROCESS_DETACH:

			RemoveVectoredExceptionHandler(g_handler);
			g_handler = NULL;
			break;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
