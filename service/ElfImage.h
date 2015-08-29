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

#ifndef __ELFIMAGE_H_
#define __ELFIMAGE_H_
#pragma once

#include "elf_traits.h"
#include "Architecture.h"
#include "Exception.h"
#include "FileSystem.h"
#include "HeapBuffer.h"
#include "ProcessMemory.h"
#include "SystemInformation.h"
#include "Win32Exception.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// ElfImage
//
// Loads an ELF binary image into a native operating system host process

class ElfImage
{
public:

	// Destructor
	//
	virtual ~ElfImage()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Load (static)
	//
	// Loads an ELF image into a process' virtual address space
	template<Architecture architecture>
	static std::unique_ptr<ElfImage> Load(const std::shared_ptr<FileSystem::Handle>& handle, const std::unique_ptr<ProcessMemory>& memory);

	//-------------------------------------------------------------------------
	// Properties

	// BaseAddress
	//
	// Gets the virtual memory base address of the loaded image
	__declspec(property(get=getBaseAddress)) const void* BaseAddress;
	const void* getBaseAddress(void) const;

	// EntryPoint
	//
	// Gets the entry point for the image
	__declspec(property(get=getEntryPoint)) const void* EntryPoint;
	const void* getEntryPoint(void) const;

	// Interpreter
	//
	// Indicates the path to the program interpreter, if one is present
	__declspec(property(get=getInterpreter)) const char_t* Interpreter;
	const char_t* getInterpreter(void) const;

	// ProgramBreak
	//
	// Pointer to the initial program break address
	__declspec(property(get=getProgramBreak)) const void* ProgramBreak;
	const void* getProgramBreak(void) const;

	// NumProgramHeaders
	//
	// Number of program headers defines as part of the loaded image
	__declspec(property(get=getNumProgramHeaders)) size_t NumProgramHeaders;
	size_t getNumProgramHeaders(void) const;

	// ProgramHeaders
	//
	// Pointer to program headers that were defined as part of the loaded image
	__declspec(property(get=getProgramHeaders)) const void* ProgramHeaders;
	const void* getProgramHeaders(void) const;

private:

	ElfImage(const ElfImage&)=delete;
	ElfImage& operator=(const ElfImage&)=delete;

	// metadata_t
	//
	// Provides metadata about the loaded ELF image
	struct metadata_t
	{
		const void*		BaseAddress = nullptr;
		const void*		ProgramBreak = nullptr;
		const void*		ProgramHeaders = nullptr;
		size_t			NumProgramHeaders = 0;
		const void*		EntryPoint = nullptr;
		std::string		Interpreter;
	};

	// Instance Constructor
	//
	ElfImage(metadata_t&& metadata) : m_metadata(std::move(metadata)) {}
	friend std::unique_ptr<ElfImage> std::make_unique<ElfImage, metadata_t>(metadata_t&&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// ValidateHeader
	//
	// Validates the contents of an ELF binary header
	template <Architecture architecture>
	static void ValidateHeader(const typename elf_traits<architecture>::elfheader_t* elfheader);

	//-------------------------------------------------------------------------
	// Member Variables

	metadata_t				m_metadata;			// Loaded image metadata
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFIMAGE_H_
