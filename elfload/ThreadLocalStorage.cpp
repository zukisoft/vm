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
#include "Exception.h"					// Include Exception declarations
#include "Instruction.h"				// Include Instruction declarations
#include "ModRM.h"						// Include ModRM declarations
#include "Win32Exception.h"				// Include Win32Exception delcarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// Function Prototypes

static LONG CALLBACK	GSSegmentExceptionHandler(PEXCEPTION_POINTERS exception);
static uint8_t			ReadTls8(uint32_t offset);
static uint16_t			ReadTls16(uint32_t offset);
static uint32_t			ReadTls32(uint32_t offset);

static uint16_t		g_test;

//-----------------------------------------------------------------------------
// Global Variables

// g_gs - TLS slot used to store virtual GS register
static DWORD g_gs = TLS_OUT_OF_INDEXES;

//-----------------------------------------------------------------------------
// Instructions

// 8E/r : MOV Sreg,r/m16
Instruction MOV_SREG_RM16(0x8E, [](ContextRecord& context) -> bool {

	// Grab the ModR/M byte and verify that the target register is GS
	ModRM modrm(context.PopInstruction<uint8_t>());
	if(modrm.reg != 0x05) return false;

	// The segment register value is 16-bit, but is stored as a 32-bit in TLS
	uint32_t value = *modrm.GetEffectiveAddress<uint16_t>(context);
	TlsSetValue(g_gs, reinterpret_cast<void*>(value));
	return true;
});

// 66 8E/r : MOV Sreg,r/m16 (operand size override)
Instruction MOV16_SREG_RM16(0x66, 0x8E, [](ContextRecord& context) -> bool {

	// Grab the ModR/M byte and verify that the target register is GS
	ModRM modrm(context.PopInstruction<uint8_t>());
	if(modrm.reg != 0x05) return false;

	// The segment register value is 16-bit, but is stored as a 32-bit in TLS
	uint32_t value = *modrm.GetEffectiveAddress<uint16_t>(context);
	TlsSetValue(g_gs, reinterpret_cast<void*>(value));
	return true;
});

//-----------------------------------------------------------------------------
// InitializeThreadLocalStorage
//
// Initializes the emulated thread local storage system
//
// Arguments:
//
//	NONE

void InitializeThreadLocalStorage(void)
{
	// This should only be done once from main()
	if(g_gs != TLS_OUT_OF_INDEXES) throw Win32Exception(ERROR_ALREADY_INITIALIZED);

	// Allocate the process-wide TLS slot for the virtual registers
	g_gs = TlsAlloc();
	if(g_gs == TLS_OUT_OF_INDEXES) throw Win32Exception();

	// Initialize the virtual GS register to zero
	TlsSetValue(g_gs, reinterpret_cast<void*>(0));

	// Add the exception handler to intercept access to the GS register
	AddVectoredExceptionHandler(0, GSSegmentExceptionHandler);
}

//-----------------------------------------------------------------------------
// ReadTls8
//
// Reads an 8-bit value from thread local storage
//
// Arguments:
//
//	offset		- Offset from TLS base address to read the value from

static inline uint8_t ReadTls8(uint32_t offset)
{
	// gs contains a segment code generated from the slot number, convert back
	uintptr_t slot = (uintptr_t(TlsGetValue(g_gs)) - 3) >> 3;
	return *reinterpret_cast<uint8_t*>(uintptr_t(TlsGetValue(slot)) + offset);
}

//-----------------------------------------------------------------------------
// ReadTls16
//
// Reads a 16-bit value from thread local storage
//
// Arguments:
//
//	offset		- Offset from TLS base address to read the value from

static inline uint16_t ReadTls16(uint32_t offset)
{
	// gs contains a segment code generated from the slot number, convert back
	uintptr_t slot = (uintptr_t(TlsGetValue(g_gs)) - 3) >> 3;
	return *reinterpret_cast<uint16_t*>(uintptr_t(TlsGetValue(slot)) + offset);
}

//-----------------------------------------------------------------------------
// ReadTls32
//
// Reads a 32-bit value from thread local storage
//
// Arguments:
//
//	offset		- Offset from TLS base address to read the value from

static inline uint32_t ReadTls32(uint32_t offset)
{
	// gs contains a segment code generated from the slot number, convert back
	uintptr_t slot = (uintptr_t(TlsGetValue(g_gs)) - 3) >> 3;
	return *reinterpret_cast<uint32_t*>(uintptr_t(TlsGetValue(slot)) + offset);
}

//-----------------------------------------------------------------------------
// GSSegmentExceptionHandler
//
// Intercepts and processes low-level exceptions caused by access to the GS
// segment register by the hosted application.  32-bit systems use this segment
// to define thread local storage.
//
// This technique is based on a sample presented by proog128 available at:
// http://0xef.wordpress.com/2012/11/17/emulate-linux-system-calls-on-windows/
//
// Arguments:
//
//	exception		- Exception information

LONG CALLBACK GSSegmentExceptionHandler(PEXCEPTION_POINTERS exception)
{
	// Accessing the GS segment should cause an access violation
	if(exception->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION)
		return EXCEPTION_CONTINUE_SEARCH;

	ContextRecord	context(exception->ContextRecord);		// Context helper

	// Cast the instruction pointer as a byte pointer to make life easier
	uint8_t* eip = reinterpret_cast<uint8_t*>(exception->ContextRecord->Eip);


	if(
		
		// MOV Sreg, r/m16
		MOV_SREG_RM16.Execute(context) || MOV16_SREG_RM16.Execute(context)
		
		) return EXCEPTION_CONTINUE_EXECUTION;

	// EIP --> 0x65				; GS Segment Override Prefix
	if(eip[0] == 0x65) {

		// 0x65, 0xA0			; MOV AL, BYTE PTR GS:[xxxxxxxx]
		if(eip[1] == 0xA0) {
			
			exception->ContextRecord->Eax &= 0xFFFFFF00;
			exception->ContextRecord->Eax |= ReadTls8(*(reinterpret_cast<uint32_t*>(&eip[2])));
			exception->ContextRecord->Eip += 6;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0xA1			; MOV EAX, DWORD PTR GS:[xxxxxxxx]
		if(eip[1] == 0xA1) {

			exception->ContextRecord->Eax = ReadTls32(*(reinterpret_cast<uint32_t*>(&eip[2])));
			exception->ContextRecord->Eip += 6;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0x8A, 0x05		; MOV AL, BYTE PTR GS:[xxxxxxxx]
		if((eip[1] == 0x8A) && (eip[2] == 0x05)) {

			exception->ContextRecord->Eax &= 0xFFFFFF00;
			exception->ContextRecord->Eax |= ReadTls8(*(reinterpret_cast<uint32_t*>(&eip[3])));
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0x8A, 0x0D		; MOV CL, BYTE PTR GS:[xxxxxxxx]
		if((eip[1] == 0x8A) && (eip[2] == 0x0D)) {

			exception->ContextRecord->Ecx &= 0xFFFFFF00;
			exception->ContextRecord->Ecx |= ReadTls8(*(reinterpret_cast<uint32_t*>(&eip[3])));
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0x8A, 0x15		; MOV DL, BYTE PTR GS:[xxxxxxxx]
		if((eip[1] == 0x8A) && (eip[2] == 0x15)) {

			exception->ContextRecord->Edx &= 0xFFFFFF00;
			exception->ContextRecord->Edx |= ReadTls8(*(reinterpret_cast<uint32_t*>(&eip[3])));
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0x8A, 0x1D		; MOV BL, BYTE PTR GS:[xxxxxxxx]
		if((eip[1] == 0x8A) && (eip[2] == 0x1D)) {

			exception->ContextRecord->Ebx &= 0xFFFFFF00;
			exception->ContextRecord->Ebx |= ReadTls8(*(reinterpret_cast<uint32_t*>(&eip[3])));
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0x8A, 0x25		; MOV AH, BYTE PTR GS:[xxxxxxxx]
		if((eip[1] == 0x8A) && (eip[2] == 0x25)) {

			exception->ContextRecord->Eax &= 0xFFFF00FF;
			exception->ContextRecord->Eax |= (ReadTls8(*(reinterpret_cast<uint32_t*>(&eip[3]))) << 8);
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0x8A, 0x2D		; MOV CH, BYTE PTR GS:[xxxxxxxx]
		if((eip[1] == 0x8A) && (eip[2] == 0x2D)) {

			exception->ContextRecord->Ecx &= 0xFFFF00FF;
			exception->ContextRecord->Ecx |= (ReadTls8(*(reinterpret_cast<uint32_t*>(&eip[3]))) << 8);
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0x8A, 0x35		; MOV DH, BYTE PTR GS:[xxxxxxxx]
		if((eip[1] == 0x8A) && (eip[2] == 0x35)) {

			exception->ContextRecord->Edx &= 0xFFFF00FF;
			exception->ContextRecord->Edx |= (ReadTls8(*(reinterpret_cast<uint32_t*>(&eip[3]))) << 8);
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0x8A, 0x3D		; MOV BH, BYTE PTR GS:[xxxxxxxx]
		if((eip[1] == 0x8A) && (eip[2] == 0x3D)) {

			exception->ContextRecord->Ebx &= 0xFFFF00FF;
			exception->ContextRecord->Ebx |= (ReadTls8(*(reinterpret_cast<uint32_t*>(&eip[3]))) << 8);
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0x8B, 0x05		; MOV EAX, DWORD PTR GS:[xxxxxxxx]
		if((eip[1] == 0x8B) && (eip[2] == 0x05)) {

			exception->ContextRecord->Eax = ReadTls32(*(reinterpret_cast<uint32_t*>(&eip[3])));
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0x8B, 0x0D		; MOV ECX, DWORD PTR GS:[xxxxxxxx]
		if((eip[1] == 0x8B) && (eip[2] == 0x0D)) {

			exception->ContextRecord->Ecx = ReadTls32(*(reinterpret_cast<uint32_t*>(&eip[3])));
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0x8B, 0x15		; MOV EDX, DWORD PTR GS:[xxxxxxxx]
		if((eip[1] == 0x8B) && (eip[2] == 0x15)) {

			exception->ContextRecord->Edx = ReadTls32(*(reinterpret_cast<uint32_t*>(&eip[3])));
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0x8B, 0x1D		; MOV EBX, DWORD PTR GS:[xxxxxxxx]
		if((eip[1] == 0x8B) && (eip[2] == 0x1D)) {

			exception->ContextRecord->Ebx = ReadTls32(*(reinterpret_cast<uint32_t*>(&eip[3])));
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0x8B, 0x25		; MOV ESP, DWORD PTR GS:[xxxxxxxx]
		if((eip[1] == 0x8B) && (eip[2] == 0x25)) {

			exception->ContextRecord->Esp = ReadTls32(*(reinterpret_cast<uint32_t*>(&eip[3])));
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0x8B, 0x2D		; MOV EBP, DWORD PTR GS:[xxxxxxxx]
		if((eip[1] == 0x8B) && (eip[2] == 0x2D)) {

			exception->ContextRecord->Ebp = ReadTls32(*(reinterpret_cast<uint32_t*>(&eip[3])));
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0x8B, 0x35		; MOV ESI, DWORD PTR GS:[xxxxxxxx]
		if((eip[1] == 0x8B) && (eip[2] == 0x35)) {

			exception->ContextRecord->Esi = ReadTls32(*(reinterpret_cast<uint32_t*>(&eip[3])));
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// 0x65, 0x8B, 0x3D		; MOV EDI, DWORD PTR GS:[xxxxxxxx]
		if((eip[1] == 0x8B) && (eip[2] == 0x3D)) {

			exception->ContextRecord->Edi = ReadTls32(*(reinterpret_cast<uint32_t*>(&eip[3])));
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}

	// EIP --> 0x66, 0x65		; Operand Size + GS Segment Override Prefixes
	if((eip[0] == 0x66) && (eip[1] == 0x65)) {

		// 0x66, 0x65, 0xA1		; MOV AX, WORD PTR GS:[xxxxxxxx]
		if(eip[2] == 0xA1) {

			exception->ContextRecord->Eax &= 0xFFFF0000;
			exception->ContextRecord->Eax |= ReadTls16(*(reinterpret_cast<uint32_t*>(&eip[3])));
			exception->ContextRecord->Eip += 7;
			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}

	// EIP --> 0x8E, 0xE8		; MOV GS, AX
	if((eip[0] == 0x8E) && (eip[1] == 0xE8)) {

		// Store the desired GS register value in thread local storage
		TlsSetValue(g_gs, reinterpret_cast<void*>(exception->ContextRecord->Eax & 0xFFFF)); 
		exception->ContextRecord->Eip += 2;
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	// 0x8E
	// 0x66, 0x8E

	// Not an instruction this handler understands
	return EXCEPTION_CONTINUE_SEARCH;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)