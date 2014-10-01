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

#ifndef __ELF_TRAITS_H_
#define __ELF_TRAITS_H_
#pragma once

#include <linux/auxvec.h>
#include <linux/elf.h>
#include <linux/elf-em.h>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// ElfClass Enumeration
//
// Used to select which instantiation of elf_traits a class will use

enum class ElfClass
{
	x86			= 0,				// 32-bit ELF data types
	x86_64		= 1,				// 64-bit ELF data types
};

//-----------------------------------------------------------------------------
// elf_traits
//
// Used to collect the various ELF types that differ between the platforms
// to ease the number of arguments that need to be specified with templates

template <ElfClass _class>
struct elf_traits {};

// elf_traits<x86>
//
template <>
struct elf_traits<ElfClass::x86>
{
	typedef uapi::Elf32_Addr		addr_t;
	typedef uapi::Elf32_auxv_t		auxv_t;
	typedef uapi::Elf32_Ehdr		elfheader_t;
	typedef uapi::Elf32_Phdr		progheader_t;
	typedef uapi::Elf32_Shdr		sectheader_t;

	// max_addr_t - Maximum size of addr_t
	//
	static const addr_t max_addr_t = 0xFFFFFFFF;
};

// elf_traits<x86_64>
//
template <>
struct elf_traits<ElfClass::x86_64>
{
	typedef uapi::Elf64_Addr		addr_t;
	typedef uapi::Elf64_auxv_t		auxv_t;
	typedef uapi::Elf64_Ehdr		elfheader_t;
	typedef uapi::Elf64_Phdr		progheader_t;
	typedef uapi::Elf64_Shdr		sectheader_t;

	// max_addr_t - Maximum size of addr_t
	//
	static const addr_t max_addr_t = 0xFFFFFFFFFFFFFFFF;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELF_TRAITS_H_
