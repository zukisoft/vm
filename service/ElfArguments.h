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

#include <memory>
#include <vector>
#include <linux/auxvec.h>
#include <linux/elf.h>
#include "Exception.h"
#include "HeapBuffer.h"

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
// ElfArguments can also do things differently since we want to use the existing 
// stack space of an existing process/thread.  A single chunk of virtual memory 
// is allocated, and the arguments/variables/aux vectors are appended it to it.
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
	ElfArgumentsT() : ElfArgumentsT(nullptr, nullptr) {}
	ElfArgumentsT(const uapi::char_t** argv, const uapi::char_t** envp);

	// TODO: use static Create() methodology

	// MemoryImage Structure
	//
	// Defines the addresses and lengths of an allocated memory image
	struct MemoryImage
	{
		addr_t		AllocationBase;			// Overall allocation pointer
		size_t		AllocationLength;		// Overall allocation length
		addr_t		StackImage;				// Stack image pointer
		size_t		StackImageLength;		// Stack image length

		// argc
		// arguments
		// environment
		// aux vectors
		// INFO
		// aux vectors
		// environment
		// arguments
		// argc
	};

	// allocator_func
	//
	// Function passed into Generate() to allocate the output buffer space
	using allocator_func = std::function<void*(size_t length)>;

	// writer_func
	//
	// Function passed into Generate() to write data into the allocated output buffer
	using writer_func = std::function<void(const void* source, void* destination, size_t length)>;

	//-------------------------------------------------------------------------
	// Member Functions

	// AppendArgument
	//
	// Appends a command line argument
	void AppendArgument(const uapi::char_t* value);

	// AppendAuxiliaryVector
	//
	// Appends an auxiliary vector
	void AppendAuxiliaryVector(addr_t type, addr_t value);
	void AppendAuxiliaryVector(addr_t type, const uapi::char_t* value);
	void AppendAuxiliaryVector(addr_t type, const void* buffer, size_t length);
	void AppendAuxiliaryVector(addr_t type, int value) { AppendAuxiliaryVector(type, static_cast<addr_t>(value)); }
	void AppendAuxiliaryVector(addr_t type, const void* value) { AppendAuxiliaryVector(type, reinterpret_cast<addr_t>(value)); }

	// AppendEnvironmentVariable
	//
	// Appends an environment variable
	void AppendEnvironmentVariable(const uapi::char_t* keyandvalue);
	void AppendEnvironmentVariable(const uapi::char_t* key, const uapi::char_t* value);

	// GenerateMemoryImage
	//
	// Creates the argument vector in the specified format using the allocator and
	// writer functions provided (this allows for using WriteProcessMemory)
	auto GenerateMemoryImage(allocator_func allocator, writer_func writer) -> MemoryImage;

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
	size_t AppendInfo(const void* buffer, size_t length, bool align = true);

	//-------------------------------------------------------------------------
	// Member Variables

	std::vector<uint8_t>		m_info;			// Information block
	std::vector<size_t>			m_argv;			// Argument string offsets
	std::vector<size_t>			m_env;			// Environment var string offsets
	std::vector<auxv_t>			m_auxv;			// Auxiliary vectors
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
