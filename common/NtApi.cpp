//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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
#include "NtApi.h"

#pragma warning(push, 4)

// g_initonce (local)
//
// Global one-time initialization context
static INIT_ONCE g_initonce = INIT_ONCE_STATIC_INIT;

// InitOnceLoadModule (local)
//
// One-time initialization handler to load the NTDLL.DLL module
static BOOL CALLBACK InitOnceLoadModule(PINIT_ONCE, PVOID, PVOID* context)
{
	*context = LoadLibrary(_T("ntdll.dll"));
	return TRUE;
}

// GetFunctionPointer (local)
//
// Retrieves a function pointer from the NTDLL.DLL module
template<typename _funcptr> 
static _funcptr GetFunctionPointer(const char* name)
{
	HMODULE module;
	InitOnceExecuteOnce(&g_initonce, InitOnceLoadModule, nullptr, reinterpret_cast<PVOID*>(&module));
	return reinterpret_cast<_funcptr>(GetProcAddress(module, name));
}

// NTAPI_FUNCTION
//
// Macro to simplify declaration of an NtApi function pointer field
#define NTAPI_FUNCTION(_name) const NtApi::_name##Func NtApi::##_name = GetFunctionPointer<NtApi::_name##Func>(#_name);

// NtApi::NtCurrentProcess (static)
//
// Pseudo-handle representing the current process
const HANDLE NtApi::NtCurrentProcess = reinterpret_cast<HANDLE>(static_cast<LONG_PTR>(-1));

// NtApi::<FunctionPointer> (static)
//
// Initializers for the various NTDLL.DLL function pointers
NTAPI_FUNCTION(NtAllocateVirtualMemory)
NTAPI_FUNCTION(NtClose)
NTAPI_FUNCTION(NtCreateSection)
NTAPI_FUNCTION(NtDuplicateObject)
NTAPI_FUNCTION(NtFlushVirtualMemory)
NTAPI_FUNCTION(NtFreeVirtualMemory)
NTAPI_FUNCTION(NtLockVirtualMemory)
NTAPI_FUNCTION(NtMapViewOfSection)
NTAPI_FUNCTION(NtProtectVirtualMemory)
NTAPI_FUNCTION(NtReadVirtualMemory)
NTAPI_FUNCTION(NtResumeProcess)
NTAPI_FUNCTION(NtSuspendProcess)
NTAPI_FUNCTION(NtUnlockVirtualMemory)
NTAPI_FUNCTION(NtUnmapViewOfSection)
NTAPI_FUNCTION(NtWriteVirtualMemory)
NTAPI_FUNCTION(RtlAreBitsClear)
NTAPI_FUNCTION(RtlAreBitsSet)
NTAPI_FUNCTION(RtlClearAllBits)
NTAPI_FUNCTION(RtlClearBit)
NTAPI_FUNCTION(RtlClearBits)
NTAPI_FUNCTION(RtlFindClearBits)
NTAPI_FUNCTION(RtlFindClearBitsAndSet)
NTAPI_FUNCTION(RtlFindClearRuns)
NTAPI_FUNCTION(RtlFindLastBackwardRunClear)
NTAPI_FUNCTION(RtlFindLongestRunClear)
NTAPI_FUNCTION(RtlFindNextForwardRunClear)
NTAPI_FUNCTION(RtlFindSetBits)
NTAPI_FUNCTION(RtlFindSetBitsAndClear)
NTAPI_FUNCTION(RtlInitializeBitMap)
NTAPI_FUNCTION(RtlNtStatusToDosError)
NTAPI_FUNCTION(RtlNumberOfClearBits)
NTAPI_FUNCTION(RtlNumberOfClearBitsInRange)
NTAPI_FUNCTION(RtlNumberOfSetBits)
NTAPI_FUNCTION(RtlNumberOfSetBitsInRange)
NTAPI_FUNCTION(RtlSetAllBits)
NTAPI_FUNCTION(RtlSetBit)
NTAPI_FUNCTION(RtlSetBits)
NTAPI_FUNCTION(RtlTestBit)

//-----------------------------------------------------------------------------

#pragma warning(pop)
