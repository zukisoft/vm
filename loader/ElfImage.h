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

#include <linux/elf.h>
#include <linux/elf-em.h>
#include <Exception.h>
#include <MappedFile.h>
#include <MappedFileView.h>
#include <MemoryRegion.h>
#include "ElfArguments.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// ElfImageT
//
// Loads an ELF image into virtual memory
//
//	ehdr_t		- ELF header structure type
//	phdr_t		- ELF program header structure type
//	shdr_t		- ELF section header structure type
//	symb_t		- ELF symbol structure type

template <class ehdr_t, class phdr_t, class shdr_t, class symb_t>
class ElfImageT
{
public:

	//-------------------------------------------------------------------------
	// Member Functions

	// Execute
	//
	// Executes the ELF image by jumping to the entry point
	void Execute(ElfArguments& args);

	// FromFile
	//
	// Parses and loads the specified ELF image into virtual memory from a file
	static ElfImageT<ehdr_t, phdr_t, shdr_t, symb_t>* FromFile(const tchar_t* path);

	// FromResource
	//
	// Parses and loads the specified ELF image into virtual memory from a resource
	static ElfImageT<ehdr_t, phdr_t, shdr_t, symb_t>* FromResource(const tchar_t* name, const tchar_t* type) { return FromResource(NULL, name, type); }
	static ElfImageT<ehdr_t, phdr_t, shdr_t, symb_t>* FromResource(HMODULE module, const tchar_t* name, const tchar_t* type);

	// ValidateHeader
	//
	// Validates an ELF image header and checks for platform compatibility
	static const ehdr_t* ValidateHeader(const void* base, size_t length);

	//-------------------------------------------------------------------------
	// Properties

	// BaseAddress
	//
	// Gets the virtual memory base address of the loaded image
	__declspec(property(get=getBaseAddress)) const void* BaseAddress;
	const void* getBaseAddress(void) const { return m_base; }

	// EntryPoint
	//
	// Gets the entry point for the image
	__declspec(property(get=getEntryPoint)) const void* EntryPoint;
	const void* getEntryPoint(void) const { return m_entry; }

	// Interpreter
	//
	// Indicates the path to the program interpreter, if one is present
	__declspec(property(get=getInterpreter)) const char_t* Interpreter;
	const char_t* getInterpreter(void) const { return (m_interpreter.size() == 0) ? nullptr : m_interpreter.c_str(); }

	// NumProgramHeaders
	//
	// Number of program headers defines as part of the loaded image
	__declspec(property(get=getNumProgramHeaders)) size_t NumProgramHeaders;
	size_t getNumProgramHeaders(void) const { return m_phdrents; }

	// ProgramHeaders
	//
	// Pointer to program headers that were defined as part of the loaded image
	__declspec(property(get=getProgramHeaders)) const phdr_t* ProgramHeaders;
	const phdr_t* getProgramHeaders(void) const { return m_phdrs; }

private:

	ElfImageT(const ElfImageT&)=delete;
	ElfImageT& operator=(const ElfImageT&)=delete;

	// Instance Constructor
	//
	ElfImageT(const void* base, size_t length);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// FlagsToProtection
	//
	// Converts the ELF p_flags into VirtualAlloc protection flags
	static DWORD FlagsToProtection(uint32_t flags);

	//-------------------------------------------------------------------------
	// Member Variables

	std::unique_ptr<MemoryRegion>	m_region;		// Allocated virtual memory
	std::string						m_interpreter;	// Program interpreter
	void*							m_base;			// Loaded image base address
	void*							m_entry;		// Calculated image entry point
	const phdr_t*					m_phdrs;		// Program header (in image)
	size_t							m_phdrents;		// Program header entries
};

//-----------------------------------------------------------------------------
// ElfImage
//
// Typedef of ElfImageT<> based on build configuration

#ifdef _M_X64
typedef ElfImageT<uapi::Elf64_Ehdr, uapi::Elf64_Phdr, uapi::Elf64_Shdr, uapi::Elf64_Sym>	ElfImage;
#else
typedef ElfImageT<uapi::Elf32_Ehdr, uapi::Elf32_Phdr, uapi::Elf32_Shdr, uapi::Elf32_Sym>	ElfImage;
#endif

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFIMAGE_H_
