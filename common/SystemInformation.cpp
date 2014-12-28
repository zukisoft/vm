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
#include "SystemInformation.h"

#pragma warning(push, 4)				

// g_sysinfo (local)
//
// Global copy of the SYSTEM_INFO structure
static SYSTEM_INFO g_sysinfo;

// g_initonce (local)
//
// Global one-time initialization context
static INIT_ONCE g_initonce = INIT_ONCE_STATIC_INIT;

// InitOnceSystemInformation (local)
//
// One-time initialization handler to retrieve system information
static BOOL CALLBACK InitOnceSystemInformation(PINIT_ONCE, PVOID, PVOID*)
{
	GetNativeSystemInfo(&g_sysinfo);
	return TRUE;
}

// SystemInformation::ActiveProcessorMask
//
unsigned __int3264 const SystemInformation::ActiveProcessorMask = []() -> unsigned __int3264 {

	InitOnceExecuteOnce(&g_initonce, InitOnceSystemInformation, nullptr, nullptr);
	return g_sysinfo.dwActiveProcessorMask;
}();

// SystemInformation::AllocationGranularity
//
size_t const SystemInformation::AllocationGranularity = []() -> size_t {

	InitOnceExecuteOnce(&g_initonce, InitOnceSystemInformation, nullptr, nullptr);
	return g_sysinfo.dwAllocationGranularity;
}();

// SystemInformation::MaximumApplicationAddress
//
void* const SystemInformation::MaximumApplicationAddress = []() -> void* {

	InitOnceExecuteOnce(&g_initonce, InitOnceSystemInformation, nullptr, nullptr);
	return g_sysinfo.lpMaximumApplicationAddress;
}();

// SystemInformation::MinimumApplicationAddress
//
void* const SystemInformation::MinimumApplicationAddress = []() -> void* {

	InitOnceExecuteOnce(&g_initonce, InitOnceSystemInformation, nullptr, nullptr);
	return g_sysinfo.lpMinimumApplicationAddress;
}();

// SystemInformation::NumberOfProcessors
//
size_t const SystemInformation::NumberOfProcessors = []() -> size_t {

	InitOnceExecuteOnce(&g_initonce, InitOnceSystemInformation, nullptr, nullptr);
	return g_sysinfo.dwNumberOfProcessors;
}();

// SystemInformation::PageSize
//
size_t const SystemInformation::PageSize = []() -> size_t {

	InitOnceExecuteOnce(&g_initonce, InitOnceSystemInformation, nullptr, nullptr);
	return g_sysinfo.dwPageSize;
}();

// SystemInformation::ProcessorArchitecture
//
SystemInformation::Architecture const SystemInformation::ProcessorArchitecture = []() -> SystemInformation::Architecture {

	InitOnceExecuteOnce(&g_initonce, InitOnceSystemInformation, nullptr, nullptr);
	return static_cast<SystemInformation::Architecture>(g_sysinfo.wProcessorArchitecture);
}();

//-----------------------------------------------------------------------------

#pragma warning(pop)
