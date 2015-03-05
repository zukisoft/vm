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

#ifndef __SYSTEMINFORMATION_H_
#define __SYSTEMINFORMATION_H_
#pragma once

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// SystemInformation
//
// Exposes SYSTEM_INFO data as static constants

class SystemInformation
{
public:

	//-------------------------------------------------------------------------
	// Data Types

	// Architecture
	//
	// Strongly typed enumeration equating to the PROCESSOR_ARCHITECTURE constants
	enum class Architecture
	{
		Intel			= PROCESSOR_ARCHITECTURE_INTEL,
		MIPS			= PROCESSOR_ARCHITECTURE_MIPS,
		Alpha			= PROCESSOR_ARCHITECTURE_ALPHA,
		PowerPC			= PROCESSOR_ARCHITECTURE_PPC,
		SHx				= PROCESSOR_ARCHITECTURE_SHX,
		ARM				= PROCESSOR_ARCHITECTURE_ARM,
		IA64			= PROCESSOR_ARCHITECTURE_IA64,
		Alpha64			= PROCESSOR_ARCHITECTURE_ALPHA64,
		MSIL			= PROCESSOR_ARCHITECTURE_MSIL,
		AMD64			= PROCESSOR_ARCHITECTURE_AMD64,
		IA32OnWin64		= PROCESSOR_ARCHITECTURE_IA32_ON_WIN64,
		Neutral			= PROCESSOR_ARCHITECTURE_NEUTRAL,
		Unknown			= PROCESSOR_ARCHITECTURE_UNKNOWN
	};

	//-------------------------------------------------------------------------
	// Fields

	// ActiveProcessorMask
	//
	// Mask representing the set of processors configured into the system
	static unsigned __int3264 const ActiveProcessorMask;

	// AllocationGranularity
	//
	// The granularity at which virtual memory can be allocated
	static size_t const AllocationGranularity;

	// MaximumApplicationAddress
	//
	// The highest memory address accessible to applications and DLLs
	static void* const MaximumApplicationAddress;

	// MinimumApplicationAddress
	//
	// The lowest memory address accessible to applications and DLLs
	static void* const MinimumApplicationAddress;

	// NumberOfProcessors
	//
	// The number of logical processors in the current group
	static size_t const NumberOfProcessors;

	// PageSize
	//
	// The page size and granularity of page protection and committment
	static size_t const PageSize;

	// ProcessorArchitecture
	//
	// The processor architecture of the installed operating system
	static Architecture const ProcessorArchitecture;

	// ProcessorFeatureMask
	//
	// Acquires the CPUID processor feature bitmask
	static uint32_t const ProcessorFeatureMask;

private:

	SystemInformation()=delete;
	~SystemInformation()=delete;
	SystemInformation(const SystemInformation&)=delete;
	SystemInformation& operator=(const SystemInformation&)=delete;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SYSTEMINFORMATION_H_
