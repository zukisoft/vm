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
//#include <linux/auxvec.h>
//#include <linux/elf.h>
//#include "elf_traits.h"
#include "Exception.h"

#pragma warning(push, 4)				
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

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
//
// ElfArguments differs mainly in the location and format of the INFO BLOCK
// data.  This data is collected into a single buffer as arguments/variables
// are appended.  When the memory image is generated, this buffer will be 
// copied into the base of the allocation and then followed by the standard
// pointers and auxiliary vectors (STACK POINTER in above table).  This provides
// the ability for the host process to merely copy the STACK IMAGE data directly
// from the allocation into the stack prior to execution.  The pointers to the
// strings and variable length auxiliary vectors remain valid, but will not
// consume any stack space in the process:
//
//  ALLOCATION BASE ->  [info]         packed information block  <--+
//                      zero[0-15]     16-byte alignment            |
//  STACK IMAGE ----->  argc           number of arguments          |
//                      argv           argument pointers -----------+ 
//                      NULL           separator                    |
//                      env[0-n]       environment pointers --------+
//                      NULL           separators                   |
//                      auxv[0-n]      auxliliary vectors ----------+
//                      AT_NULL        separator                          
//                      NULL           terminator
//                      zero[0-15]     16-byte alignment

class ElfArguments
{
public:

	//// MemoryImage Structure
	////
	//// Defines the addresses and lengths of an allocated memory image
	//struct MemoryImage
	//{
	//	void*		AllocationBase;			// Base allocation pointer
	//	size_t		AllocationLength;		// Base allocation length
	//	void*		StackImage;				// Stack image pointer
	//	size_t		StackImageLength;		// Stack image length
	//};

	//// allocator_func
	////
	//// Function passed into Generate() to allocate the output buffer space
	//using allocator_func = std::function<void*(size_t length)>;

	//// writer_func
	////
	//// Function passed into Generate() to write data into the allocated output buffer
	//using writer_func = std::function<void(const void* source, void* destination, size_t length)>;

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

	// Create (static)
	//
	// Creates an ElfArguments instance
	static std::unique_ptr<ElfArguments> Create(void) { return Create(nullptr, nullptr); }
	static std::unique_ptr<ElfArguments> Create(const uapi::char_t** argv, const uapi::char_t** envp) { return std::make_unique<ElfArguments>(argv, envp); }

	// GenerateMemoryImage
	//
	// Creates the argument vector in the specified format using the allocator and
	// writer functions provided (this allows for using WriteProcessMemory)
	//auto GenerateMemoryImage(allocator_func allocator, writer_func writer) -> MemoryImage;

private:

	ElfArguments(const ElfArguments&)=delete;
	ElfArguments& operator=(const ElfArguments&)=delete;

	// Instance Constructor
	//
	ElfArguments(const uapi::char_t** argv, const uapi::char_t** envp);
	friend std::unique_ptr<ElfArguments> std::make_unique<ElfArguments, const uapi::char_t**&, const uapi::char_t**&>(const uapi::char_t**&, const uapi::char_t**&);

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

	//-------------------------------------------------------------------------
	// Private Member Functions

	// AlignUp
	//
	// Address alignment helper function
	static size_t AlignUp(size_t offset, size_t alignment);

	// AppendInfo
	//
	// Appends data to the information block and returns the offset
	uintptr_t AppendInfo(const void* buffer, size_t length);

	//-------------------------------------------------------------------------
	// Member Variables

	std::vector<uint8_t>		m_info;			// Information block
	std::vector<uintptr_t>		m_argv;			// Argument string offsets
	std::vector<uintptr_t>		m_env;			// Environment var string offsets
	std::vector<auxvec_t>		m_auxv;			// Auxiliary vectors / offsets
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFARGUMENTS_H_
