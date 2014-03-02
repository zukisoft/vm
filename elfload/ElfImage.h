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
#include "ElfArguments.h"				// Include ElfArguments declarations
#include "Exception.h"					// Include Exception declarations
#include "MappedFile.h"					// Include MappedFile declarations
#include "MappedFileView.h"				// Include MappedFileView declarations
#include "MemoryRegion.h"				// Include MemoryRegion declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// ElfImageT
//
// Loads an ELF image into virtual memory
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

	// Execute
	//
	// Executes the ELF image by jumping to the entry point
	uint32_t Execute(ElfArguments& args);

	// Load
	//
	// Parses and loads the specified ELF image into virtual memory
	static ElfImageT<ehdr_t, phdr_t, shdr_t>* Load(const tchar_t* path);

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
	__declspec(property(get=getInterpreter)) const tchar_t* Interpreter;
	const tchar_t* getInterpreter(void) const { return (m_interpreter.size() == 0) ? nullptr : m_interpreter.c_str(); }

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

	ElfImageT(const ElfImageT&);
	ElfImageT& operator=(const ElfImageT&);

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
	std::tstring					m_interpreter;	// Program interpreter
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
typedef ElfImageT<Elf64_Ehdr, Elf64_Phdr, Elf64_Shdr>	ElfImage;
#else
typedef ElfImageT<Elf32_Ehdr, Elf32_Phdr, Elf32_Shdr>	ElfImage;
#endif

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFIMAGE_H_
