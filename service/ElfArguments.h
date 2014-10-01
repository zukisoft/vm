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
#include "elf_traits.h"
#include "Exception.h"
#include "HeapBuffer.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// ElfArguments
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
// ElfArguments uses a single buffer to collect all of the arguments, envrionment
// variables and pointer-based auxiliary vector data as the data is collected.
// TODO: recomment this
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

template <ElfClass _class>
class ElfArguments
{
public:

	// elf
	//
	// Alias for elf_traits<_class>
	using elf = elf_traits<_class>;

	// Instance constructors
	//
	ElfArguments() : ElfArguments(nullptr, nullptr) {}
	ElfArguments(const uapi::char_t** argv, const uapi::char_t** envp);
	// TODO: use static Create() methodology

	// MemoryImage Structure
	//
	// Defines the addresses and lengths of an allocated memory image
	struct MemoryImage
	{
		void*		AllocationBase;			// Base allocation pointer
		size_t		AllocationLength;		// Base allocation length
		void*		StackImage;				// Stack image pointer
		size_t		StackImageLength;		// Stack image length
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
	void AppendAuxiliaryVector(typename elf::addr_t type, typename elf::addr_t value);
	void AppendAuxiliaryVector(typename elf::addr_t type, const uapi::char_t* value);
	void AppendAuxiliaryVector(typename elf::addr_t type, const void* buffer, size_t length);
	void AppendAuxiliaryVector(typename elf::addr_t type, int value) { AppendAuxiliaryVector(type, static_cast<typename elf::addr_t>(value)); }
	void AppendAuxiliaryVector(typename elf::addr_t type, const void* value) { AppendAuxiliaryVector(type, reinterpret_cast<typename elf::addr_t>(value)); }

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

	ElfArguments(const ElfArguments&)=delete;
	ElfArguments& operator=(const ElfArguments&)=delete;

	// AT_ISOFFSET
	//
	// Flag combined with the AT_ value of an auxiliary vector to indicate
	// that the specified value is an offset into the info block; this will be
	// stripped out during generation of the stack image and replaced with
	// a pointer to that data
	const typename elf::addr_t AT_ISOFFSET = 0xF0000000;

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

	std::vector<uint8_t>				m_info;			// Information block
	std::vector<size_t>					m_argv;			// Argument string offsets
	std::vector<size_t>					m_env;			// Environment var string offsets
	std::vector<typename elf::auxv_t>	m_auxv;			// Auxiliary vectors
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFARGUMENTS_H_
