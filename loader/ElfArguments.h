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

#include <linux/auxvec.h>
#include <linux/elf.h>
#include <Exception.h>
#include <MemoryRegion.h>
#include <Win32Exception.h>

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// ElfArgumentsT
//
// ELF arguments on the x86/x64 platform are provided by pushing a vector of
// values/pointers onto the stack prior to jumping to the entry point.  The
// typical memory format is as follows:
//
//  STACK POINTER -->   argc          number of arguments
//                      argv[0-n]     pointers to command line arguments
//                      NULL          separator
//                      env[0-n]      pointers to environment variables
//                      NULL          separator
//                      auxv[0-n]     auxiliary vectors
//                      AT_NULL       separator
//                      NULL          terminator
//                      zero[0-15]    16-byte alignment
//  INFO BLOCK ----->   [auxv]        packed auxiliary vector data
//                      [env]         packed environment strings
//                      [argv]        packed command line argument strings
//  STACK BOTTOM ---->  NULL          terminator
//
// ElfArgumentsT does things differently since we want to use the existing stack
// space of the process/thread.  A single chunk of virtual memory (64KiB) is
// allocated, and the arguments/variables/aux vectors are appended it to it.
// When complete, the ELF arguments are allocated in the same chunk of memory.
// The arguments can then be copied into the process/thread stack for execution:
//
//  BASE ADDRESS --->   [info]         packed information block  <--+
//                      zero[0-15]     16-byte alignment            |
//  ARGUMENTS ------>   argc           number of arguments          |
//                      argv           argument pointers -----------+ 
//                      NULL           separator                    |
//                      env[0-n]       environment pointers --------+
//                      NULL           separators                   |
//                      auxv[0-n]      auxliliary vectors ----------+
//                      AT_NULL        separator                          
//                      NULL           terminator
//                      zero[0-15]     16-byte alignment

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
	void AppendArgument(const char_t* value);
	void AppendArgument(const wchar_t* value);

	// AppendAuxiliaryVector
	//
	// Appends an auxiliary vector
	void AppendAuxiliaryVector(addr_t type, addr_t value);
	void AppendAuxiliaryVector(addr_t type, const char_t* value);
	void AppendAuxiliaryVector(addr_t type, const wchar_t* value);
	void AppendAuxiliaryVector(addr_t type, const void* buffer, size_t length);
	void AppendAuxiliaryVector(addr_t type, int value) { AppendAuxiliaryVector(type, static_cast<addr_t>(value)); }
	void AppendAuxiliaryVector(addr_t type, const void* value) { AppendAuxiliaryVector(type, reinterpret_cast<addr_t>(value)); }


	// AppendEnvironmentVariable
	//
	// Appends an environment variable
	void AppendEnvironmentVariable(const char_t* keyandvalue);
	void AppendEnvironmentVariable(const wchar_t* keyandvalue);
	void AppendEnvironmentVariable(const char_t* key, const char_t* value);
	void AppendEnvironmentVariable(const wchar_t* key, const wchar_t* value);

	// CreateArgumentVector
	//
	// Creates the argument vector
	const void* CreateArgumentVector(size_t* length);

private:

	ElfArgumentsT(const ElfArgumentsT&)=delete;
	ElfArgumentsT& operator=(const ElfArgumentsT&)=delete;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// AlignUp
	//
	// Address alignment helper function
	static size_t AlignUp(size_t offset, size_t alignment);

	// AppendInfo
	//
	// Appends data to the information block
	addr_t AppendInfo(addr_t value);
	addr_t AppendInfo(auxv_t value);
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
typedef ElfArgumentsT<uapi::Elf64_Addr, uapi::Elf64_auxv_t>	ElfArguments;
#else
typedef ElfArgumentsT<uapi::Elf32_Addr, uapi::Elf32_auxv_t>	ElfArguments;
#endif

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFARGUMENTS_H_
