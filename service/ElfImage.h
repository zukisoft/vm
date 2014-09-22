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
#include "Exception.h"
#include "HeapBuffer.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// ElfImage
//
// Base interface exposed by various ELF image format classes

class ElfImage
{
public:

	// Destructor
	//
	~ElfImage()=default;

	// Type
	//
	// Defines the class of the ELF binary image, used to determine if a 32 or 64-bit
	// loader needs to be used
	enum class Type
	{
		None		= LINUX_ELFCLASSNONE,
		i386		= LINUX_ELFCLASS32,
		x86_64		= LINUX_ELFCLASS64,
	};

	//-------------------------------------------------------------------------
	// Member Functions

	// todo
	static Type GetType(void) { return Type::None; }

	// this guy takes multiple things in and then calls loader<T> to get
	static std::unique_ptr<ElfImage> Load(void);

	//-------------------------------------------------------------------------
	// Properties

	// BaseAddress
	//
	// Gets the virtual memory base address of the loaded image
	__declspec(property(get=getBaseAddress)) const void* BaseAddress;
	virtual const void* getBaseAddress(void) const { return m_metadata.BaseAddress; }

	// EntryPoint
	//
	// Gets the entry point for the image
	__declspec(property(get=getEntryPoint)) const void* EntryPoint;
	virtual const void* getEntryPoint(void) const { return m_metadata.EntryPoint; }

	// Interpreter
	//
	// Indicates the path to the program interpreter, if one is present
	__declspec(property(get=getInterpreter)) const tchar_t* Interpreter;
	virtual const tchar_t* getInterpreter(void) const { return (m_metadata.Interpreter.size() == 0) ? nullptr : m_metadata.Interpreter.c_str(); }

	// NumProgramHeaders
	//
	// Number of program headers defines as part of the loaded image
	__declspec(property(get=getNumProgramHeaders)) size_t NumProgramHeaders;
	virtual size_t getNumProgramHeaders(void) const { return m_metadata.NumProgramHeaders; }

	// ProgramHeaders
	//
	// Pointer to program headers that were defined as part of the loaded image
	__declspec(property(get=getProgramHeaders)) const void* ProgramHeaders;
	virtual const void* getProgramHeaders(void) const { return m_metadata.ProgramHeaders; }

private:

	ElfImage(const ElfImage&)=delete;
	ElfImage& operator=(const ElfImage&)=delete;

	// Forward Declarations
	//
	struct Metadata;

	// Instance Constructor
	//
	ElfImage(Metadata&& metadata) : m_metadata(metadata) {}
	friend std::unique_ptr<ElfImage> std::make_unique<ElfImage, Metadata>(Metadata&&);

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// ElfCommon_Ehdr
	//
	// Common portion of the ELF header structure, can be used to determine
	// what the size and type of the full header structure is
	typedef struct {

	  uint8_t		e_ident[LINUX_EI_NIDENT];
	  uint16_t		e_type;
	  uint16_t		e_machine;
	  uint32_t		e_version;

	} ElfCommon_Ehdr;

	// image_reader_func
	//
	// Invoked by LoadImage to read data from the binary image file
	using image_read_func = std::function<size_t(void* buffer, size_t offset, size_t count)>;

	using image_copy_func = std::function<size_t(size_t offset, size_t count, void* buffer)>;

	// Metadata
	//
	// Provides information about an image that has been loaded by ElfLoader<>
	struct Metadata
	{
		void*					BaseAddress = nullptr;
		void*					ProgramHeaders = nullptr;
		size_t					NumProgramHeaders = 0;
		void*					EntryPoint = nullptr;
		std::tstring			Interpreter;
	};

	//-------------------------------------------------------------------------
	// Priavte Member Functions

	// FlagsToProtection
	//
	// Converts ELF p_flags into VirtualAlloc(Ex) protection flags
	static DWORD FlagsToProtection(uint32_t flags);

	// LoadBinary
	//
	// Loads an ELF binary image into virtual memory
	template <int elfclass, class ehdr_t, class phdr_t, class shdr_t>
	static Metadata LoadBinary(HANDLE process, image_read_func reader);

	// ValidateHeader
	//
	// Validates the contents of an ELF binary header
	template <int elfclass, class ehdr_t, class phdr_t, class shdr_t>
	static void ValidateHeader(const ehdr_t* elfheader);

	//-------------------------------------------------------------------------
	// Member Variables

	const Metadata				m_metadata;		// Loaded image metadata
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFIMAGE_H_
