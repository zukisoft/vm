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

#ifndef __ELFARGUMENTS_H_
#define __ELFARGUMENTS_H_
#pragma once

#include <memory>
#include <vector>
#include "ElfClass.h"
#include "Exception.h"
#include "HeapBuffer.h"
#include "MemoryRegion.h"
#include "Win32Exception.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// ElfArguments
//
// ELF arguments on the x86/x64 platform are provided by pushing a vector of
// values/pointers onto the stack prior to jumping to the entry point.  The
// typical memory format is as follows:
//
//  STACK POINTER --->  argc          number of arguments
//                      argv[0-n]     pointers to command line arguments
//                      NULL          separator
//                      env[0-n]      pointers to environment variables
//                      NULL          separator
//                      auxv[0-n]     auxiliary vectors
//                      AT_NULL       separator
//                      NULL          terminator
//                      zero[0-15]    16-byte alignment
//  INFO BLOCK ------>  [auxv]        packed auxiliary vector data
//                      [env]         packed environment strings
//                      [argv]        packed command line argument strings
//  STACK BOTTOM ---->  NULL          terminator

class ElfArguments
{
public:

	// Instance Constructors
	//
	ElfArguments() : ElfArguments(nullptr, nullptr) {}
	ElfArguments(const uapi::char_t** argv, const uapi::char_t** envp);

	//-------------------------------------------------------------------------
	// Member Functions

	// AppendArgument
	//
	// Appends a command line argument
	void AppendArgument(const uapi::char_t* value);

	// AppendAuxiliaryVector
	//
	// Appends an auxiliary vector
	void AppendAuxiliaryVector(int type, uintptr_t value);
	void AppendAuxiliaryVector(int type, const uapi::char_t* value);
	void AppendAuxiliaryVector(int type, const void* buffer, size_t length);
	void AppendAuxiliaryVector(int type, int value) { AppendAuxiliaryVector(type, uintptr_t(value)); }
	void AppendAuxiliaryVector(int type, const void* value) { AppendAuxiliaryVector(type, uintptr_t(value)); }

	// AppendEnvironmentVariable
	//
	// Appends an environment variable
	void AppendEnvironmentVariable(const uapi::char_t* keyandvalue);
	void AppendEnvironmentVariable(const uapi::char_t* key, const uapi::char_t* value);

	// GenerateProcessStack
	//
	// Generates a stack from the collected ELF arguments
	template <ElfClass _elfclass> void* GenerateProcessStack(HANDLE process, void* base, size_t length);

private:

	ElfArguments(const ElfArguments&)=delete;
	ElfArguments& operator=(const ElfArguments&)=delete;

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// auxvec_t
	//
	// Generic auxiliary vector structure, converted to ELF class specific
	// structure during generation.  Indicate that the value is an offset into
	// the information block by specifying type as a negative
	struct auxvec_t
	{
		auxvec_t(int type) : a_type(type), a_val(0) {}
		auxvec_t(int type, uintptr_t value) : a_type(type), a_val(value) {}

		int			a_type;		// Type code (negative = a_val is an offset)
		uintptr_t	a_val;		// Value or offset into the info block
	};

	// MAX_INFO_BUFFER
	//
	// Defines the maximum allowed size of the information buffer; arbitrary
	// for now, may become configurable in the future
	static const uint32_t MAX_INFO_BUFFER = (256 KiB);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// AppendInfo
	//
	// Appends data to the information block and returns the offset
	uint32_t AppendInfo(const void* buffer, size_t length);

	//-------------------------------------------------------------------------
	// Member Variables

	std::vector<uint8_t>		m_info;			// Information block
	std::vector<uint32_t>		m_argv;			// Argument string offsets
	std::vector<uint32_t>		m_envp;			// Environment var string offsets
	std::vector<auxvec_t>		m_auxv;			// Auxiliary vectors / offsets
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFARGUMENTS_H_
