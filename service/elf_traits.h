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

#ifndef __ELF_TRAITS_H_
#define __ELF_TRAITS_H_
#pragma once

#include <linux/auxvec.h>
#include <linux/elf.h>
#include <linux/elf-em.h>
#include "Architecture.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// elf_traits
//
// Used to collect the various ELF types that differ between the platforms
// to ease the number of template arguments that need to be specified

// elf_traits
//
template <Architecture architecture> struct elf_traits {};

// elf_traits<x86>
//
template <> struct elf_traits<Architecture::x86>
{
	typedef uapi::Elf32_Addr		addr_t;
	typedef uapi::Elf32_auxv_t		auxv_t;
	typedef uapi::Elf32_Ehdr		elfheader_t;
	typedef uapi::Elf32_Phdr		progheader_t;
	typedef uapi::Elf32_Shdr		sectheader_t;

	// elfclass
	//
	// Defines the ELFCLASS value for this elf_traits<>
	static const int elfclass = LINUX_ELFCLASS32;

	// machinetype
	//
	// Defines the ELF_EM value for this elf_traits<>
	static const int machinetype = LINUX_EM_386;

	// platform
	//
	// Defines the platform string for this elf_traits<>
	static const uapi::char_t* platform;

	elf_traits(const elf_traits&)=delete;
	elf_traits& operator=(const elf_traits&)=delete;
};

// elf_traits<x86> static initializers
//
__declspec(selectany)
const uapi::char_t* elf_traits<Architecture::x86>::platform = "i686";

#ifdef _M_X64
// elf_traits<x86_64>
//
template <> struct elf_traits<Architecture::x86_64>
{
	typedef uapi::Elf64_Addr		addr_t;
	typedef uapi::Elf64_auxv_t		auxv_t;
	typedef uapi::Elf64_Ehdr		elfheader_t;
	typedef uapi::Elf64_Phdr		progheader_t;
	typedef uapi::Elf64_Shdr		sectheader_t;

	// elfclass
	//
	// Defines the ELFCLASS value for this elf_traits<>
	static const int elfclass = LINUX_ELFCLASS64;

	// machinetype
	//
	// Defines the ELF_EM value for this elf_traits<>
	static const int machinetype = LINUX_EM_X86_64;

	// platform
	//
	// Defines the platform string for this elf_traits<>
	static const uapi::char_t* platform;

	elf_traits(const elf_traits&)=delete;
	elf_traits& operator=(const elf_traits&)=delete;
};

// elf_traits<x86_64> static initializers
//
__declspec(selectany)
const uapi::char_t* elf_traits<Architecture::x86_64>::platform = "x86_64";
#endif

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELF_TRAITS_H_
