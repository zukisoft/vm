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

#ifndef __ELFIMAGE_H_
#define __ELFIMAGE_H_
#pragma once

#include "elf.h"						// Include ELF file format decls
#include "Exception.h"					// Include Exception declarations
#include "MappedFile.h"					// Include MappedFile declarations
#include "MappedFileView.h"				// Include MappedFileView declarations
#include "MemoryRegion.h"				// Include MemoryRegion declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

extern "C" void elf_entry(void);

//-----------------------------------------------------------------------------
// ElfImageT
//
// Represents a loaded ELF image
//
//	ehdr_t		- ELF header structure type
//	phdr_t		- ELF program header structure type
//	shdr_t		- ELF section header structure type

template <class ehdr_t, class phdr_t, class shdr_t>
class ElfImageT
{
public:

	//-------------------------------------------------------------------------
	// Member Functions

	// Load
	//
	// Parses and loads the specified ELF image into virtual memory
	static ElfImageT<ehdr_t, phdr_t, shdr_t>* Load(LPCTSTR path);
	static ElfImageT<ehdr_t, phdr_t, shdr_t>* Load(std::shared_ptr<MappedFile>& mapping) { return Load(mapping, 0); }
	static ElfImageT<ehdr_t, phdr_t, shdr_t>* Load(std::shared_ptr<MappedFile>& mapping, size_t length);
	static ElfImageT<ehdr_t, phdr_t, shdr_t>* Load(std::unique_ptr<MappedFileView>& view) { return Load(view, view->Length); }
	static ElfImageT<ehdr_t, phdr_t, shdr_t>* Load(std::unique_ptr<MappedFileView>& view, size_t length);

	// TryValidateHeader
	//
	// Validates an ELF binary header; does not throw an exception
	static bool TryValidateHeader(const void* base, size_t length);

	// ValidateHeader
	//
	// Validates an ELF binary header; throws an exception
	static void ValidateHeader(const void* base, size_t length);

private:

	ElfImageT(const ElfImageT&);
	ElfImageT& operator=(const ElfImageT&);

	// Instance Constructor
	//
	ElfImageT(std::shared_ptr<MappedFile>& mapping, size_t length);

	// EntryPoint
	//
	// ELF entry function pointer
	typedef void(*EntryPoint)(void);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// AlignDown/AlignUp
	//
	// Address alignment helper functions
	static uintptr_t AlignDown(uintptr_t address, size_t alignment);
	static uintptr_t AlignUp(uintptr_t address, size_t alignment);
	
	// FlagsToProtection
	//
	// Converts the ELF p_flags into VirtualAlloc protection flags
	static DWORD FlagsToProtection(uint32_t flags);

	//-------------------------------------------------------------------------
	// Member Variables

	std::unique_ptr<MemoryRegion>	m_region;		// Allocated virtual memory
	EntryPoint						m_entry;		// Entry point
};

//-----------------------------------------------------------------------------
// ElfImage
//
// Typedef of ElfImageT<> based on build configuration

#ifdef _M_X64
typedef ElfImageT<Elf64_Ehdr, Elf64_Phdr, Elf64_Shdr>	ElfImage;
#else
typedef ElfImageT<Elf32_Ehdr, Elf32_Phdr, Elf32_Shdr>	ElfImage;
#endif

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFIMAGE_H_
