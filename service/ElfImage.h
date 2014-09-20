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
#include <MemoryRegion.h>

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// ElfImage
//
// Base interface exposed by various ELF image format classes

struct __declspec(novtable) ElfImage
{
	// read_image_func
	//
	// Invoked by ElfImageT to read data from the binary image file
	using read_image_func = std::function<size_t(void* buffer, size_t offset, size_t count)>;

	// BaseAddress
	//
	// Gets the virtual memory base address of the loaded image
	__declspec(property(get=getBaseAddress)) const void* BaseAddress;
	virtual const void* getBaseAddress(void) const = 0;

	// EntryPoint
	//
	// Gets the entry point for the image
	__declspec(property(get=getEntryPoint)) const void* EntryPoint;
	virtual const void* getEntryPoint(void) const = 0;

	// Interpreter
	//
	// Indicates the path to the program interpreter, if one is present
	__declspec(property(get=getInterpreter)) const tchar_t* Interpreter;
	virtual const tchar_t* getInterpreter(void) const = 0;

	// NumProgramHeaders
	//
	// Number of program headers defines as part of the loaded image
	__declspec(property(get=getNumProgramHeaders)) size_t NumProgramHeaders;
	virtual size_t getNumProgramHeaders(void) const = 0;

	// ProgramHeaders
	//
	// Pointer to program headers that were defined as part of the loaded image
	__declspec(property(get=getProgramHeaders)) const void* ProgramHeaders;
	virtual const void* getProgramHeaders(void) const = 0;
};

//-----------------------------------------------------------------------------
// ElfImageT
//
// Templatized implementation of ElfImage interface
//
//	elfclass	- Expected ELF binary class value
//	ehdr_t		- ELF header structure type
//	phdr_t		- ELF program header structure type
//	shdr_t		- ELF section header structure type

template<int elfclass, class ehdr_t, class phdr_t, class shdr_t>
class ElfImageT
{
public:

	//-------------------------------------------------------------------------
	// Member Functions

	// Load (static)
	//
	// Loads the image into memory using the provided callback functions
	static std::unique_ptr<ElfImage> Load(ElfImage::read_image_func reader);

	//-------------------------------------------------------------------------
	// ElfImage Implementation

	// getBaseAddress
	//
	// Gets the virtual memory base address of the loaded image
	virtual const void* getBaseAddress(void) const { return m_base; }

	// getEntryPoint
	//
	// Gets the entry point for the image
	virtual const void* getEntryPoint(void) const { return m_entry; }

	// getInterpreter
	//
	// Indicates the path to the program interpreter, if one is present
	virtual const tchar_t* getInterpreter(void) const { return (m_interpreter.size() == 0) ? nullptr : m_interpreter.c_str(); }

	// getNumProgramHeaders
	//
	// Number of program headers defines as part of the loaded image
	virtual size_t getNumProgramHeaders(void) const { return m_phdrents; }

	// getProgramHeaders
	//
	// Pointer to program headers that were defined as part of the loaded image
	virtual const void* getProgramHeaders(void) const { return m_phdrs; }

private:

	ElfImageT(const ElfImageT&)=delete;
	ElfImageT& operator=(const ElfImageT&)=delete;

	// Instance Constructor
	//
	ElfImageT()=default;
	friend std::unique_ptr<ElfImageT> std::make_unique<ElfImageT>();

	//-------------------------------------------------------------------------
	// Private Member Functions

	// FlagsToProtection
	//
	// Converts the ELF p_flags into VirtualAlloc(Ex) protection flags
	static DWORD FlagsToProtection(uint32_t flags);

	//-------------------------------------------------------------------------
	// Member Variables

	std::unique_ptr<MemoryRegion>	m_region;			// Allocated virtual memory
	std::tstring					m_interpreter;		// Program interpreter
	void*							m_base;				// Loaded image base address
	void*							m_entry;			// Calculated image entry point
	const phdr_t*					m_phdrs;			// Program headers (in image)
	size_t							m_phdrents;			// Program header entries
};

// ElfImage32
//
// 32-bit x86 ELF binary image type
using ElfImage32 = ElfImageT<LINUX_ELFCLASS32, uapi::Elf32_Ehdr, uapi::Elf32_Phdr, uapi::Elf32_Shdr>;

// ElfLoader64
//
// 64-bit x86_64 ELF binary image type
#ifdef _M_X64
using ElfImage64 = ElfImageT<LINUX_ELFCLASS64, uapi::Elf64_Ehdr, uapi::Elf64_Phdr, uapi::Elf64_Shdr>;
#endif

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFIMAGE_H_
