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

#ifndef __ELFARGUMENTS_H_
#define __ELFARGUMENTS_H_
#pragma once

#include "auxvec.h"						// Include auxiliary vector decls
#include "elf.h"						// Include ELF format declarations
#include "Exception.h"					// Include Exception declarations
#include "MemoryRegion.h"				// Include MemoryRegion declarations
#include "Win32Exception.h"				// Include Win32Exception declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// ElfArgumentsT
//
// ELF arguments on the x86/x64 platform are provided by pushing a vector of
// values/pointers onto the stack prior to jumping to the entry point
//
// !!STACK MUST BE 16-BYTE ALIGNED!!
//
//	STACK POINTER ----->	argc				number of arguments
//							argv[0]				first command line argument
//							argv[n]				last command line argument
//							NULL				separator
//							env[0]				first environment variable
//							env[n]				last environment variable
//							NULL				separator
//							auxv[0]				first auxiliary vector
//							auxv[n]				last auxiliary vector
//							AT_NULL				separator
//							zero[0-15]			padding to provide 16-byte alignment
//							[auxv]				packed auxiliary vector data
//							[env]				packed environment strings
//							[argv]				packed command line argument strings
//	BOTTOM OF STACK ---->	NULL				terminator
//
// After loading the values into the builder, the CreateVector() function
// is called to allocate and initialize the arguments vector.  The vector
// will be returned in the reverse of the stack order so that the values
// can be easily iterated and pushed onto the program stack

template <class addr_t, class auxv_t>
class ElfArgumentsT
{
public:

	// Instance constructors
	//
	ElfArgumentsT();

	//-------------------------------------------------------------------------
	// Member Functions

	// AppendArgument
	//
	// Appends a command line argument
	void AppendArgument(const char* value);
	void AppendArgument(const wchar_t* value);

	// AppendAuxiliaryVector
	//
	// Appends an auxiliary vector
	void AppendAuxiliaryVector(addr_t type, addr_t value);
	void AppendAuxiliaryVector(addr_t type, const char* value);
	void AppendAuxiliaryVector(addr_t type, const wchar_t* value);
	void AppendAuxiliaryVector(addr_t type, const void* buffer, size_t length);

	// AppendEnvironmentVariable
	//
	// Appends an environment variable
	void AppendEnvironmentVariable(const char* key, const char* value);
	void AppendEnvironmentVariable(const wchar_t* key, const wchar_t* value);

	// CreateArgumentStack
	//
	// Creates the bottom-up block of data to be pushed onto the stack
	size_t CreateArgumentStack(addr_t** stack);

	// ReleaseArgumentStack
	//
	// Releases memory allocated with CreateArgumentStack()
	static addr_t* ReleaseArgumentStack(addr_t* stack);

private:

	ElfArgumentsT(const ElfArgumentsT&);
	ElfArgumentsT& operator=(const ElfArgumentsT&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// AlignUp
	//
	// Address alignment helper function
	static size_t AlignUp(size_t offset, size_t alignment);

	// AppendInfo
	//
	// Appends data to the information block
	addr_t AppendInfo(const void* buffer, size_t length);

	//-------------------------------------------------------------------------
	// Member Variables

	std::vector<addr_t>				m_argv;		// Command line arguments
	std::vector<addr_t>				m_env;		// Environment variables
	std::vector<auxv_t>				m_auxv;		// Auxiliary vectors

	std::unique_ptr<MemoryRegion>	m_info;		// Information block
	uint32_t						m_offset;	// Offset into info block
};

//-----------------------------------------------------------------------------
// ElfArgumentBuilder
//
// Typedef of ElfArgumentBuilderT<> based on build configuration

#ifdef _M_X64
typedef ElfArgumentsT<Elf64_Addr, Elf64_auxv_t>	ElfArguments;
#else
typedef ElfArgumentsT<Elf32_Addr, Elf32_auxv_t>	ElfArguments;
#endif

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFARGUMENTS_H_
