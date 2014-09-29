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

#ifndef __ELFTYPE_H_
#define __ELFTYPE_H_
#pragma once

#include <linux/auxvec.h>
#include <linux/elf.h>
#include <linux/elf-em.h>

#pragma warning(push, 4)	

//-----------------------------------------------------------------------------
// elf_traits

class elf_traits
{
};



template <class addr_t, class auxv_t>
class ElfArgumentsT
{
public:

	// Instance constructors
	//
	ElfArgumentsT() : ElfArgumentsT(nullptr, nullptr) {}
	ElfArgumentsT(const uapi::char_t** argv, const uapi::char_t** envp);

	// TODO: use static Create() methodology

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

#endif	// __ELFTYPE_H_
