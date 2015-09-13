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

#ifndef __ELFBINARY_H_
#define __ELFBINARY_H_
#pragma once

#include <memory>
#include "Architecture.h"
#include "Binary.h"
#include "BinaryFormat.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

// Forward Declarations
//
class Executable;
class Host;

//-----------------------------------------------------------------------------
// ElfBinary
//
// Specialization of Binary for ELF images

class ElfBinary : public Binary
{
public:

	// Destructor
	//
	virtual ~ElfBinary()=default;

	//-------------------------------------------------------------------------
	// Friend Functions

	// LoadElfBinary
	//
	// Architecture-specific implementation of Load
	template<Architecture architecture>
	friend std::unique_ptr<Binary> LoadElfBinary(Host* host, Executable const* executable);

	// ValidateElfHeader
	//
	// Architecture-specific ELF header validation function
	template <Architecture architecture>
	friend void ValidateElfHeader(void const* buffer, size_t cb);

	//-------------------------------------------------------------------------
	// Member Functions

	// Load (static)
	//
	// Loads an ELF binary image
	static std::unique_ptr<Binary> Load(Host* host, Executable const* executable);

	//-------------------------------------------------------------------------
	// Binary Implementation

	// BaseAddress
	//
	// Gets the virtual memory base address of the loaded image
	virtual void const* getBaseAddress(void) const;

	// EntryPoint
	//
	// Gets the entry point for the image
	virtual void const* getEntryPoint(void) const;

	//-------------------------------------------------------------------------
	// Properties

	//// Interpreter
	////
	//// Indicates the path to the program interpreter, if one is present
	//__declspec(property(get=getInterpreter)) const char_t* Interpreter;
	//const char_t* getInterpreter(void) const;

	//// ProgramBreak
	////
	//// Pointer to the initial program break address
	//__declspec(property(get=getProgramBreak)) const void* ProgramBreak;
	//const void* getProgramBreak(void) const;

	//// NumProgramHeaders
	////
	//// Number of program headers defines as part of the loaded image
	//__declspec(property(get=getNumProgramHeaders)) size_t NumProgramHeaders;
	//size_t getNumProgramHeaders(void) const;

	//// ProgramHeaders
	////
	//// Pointer to program headers that were defined as part of the loaded image
	//__declspec(property(get=getProgramHeaders)) const void* ProgramHeaders;
	//const void* getProgramHeaders(void) const;

private:

	ElfBinary(ElfBinary const&)=delete;
	ElfBinary& operator=(ElfBinary const&)=delete;

	// format_traits_t
	//
	// Architecture specific ELF format traits
	template <Architecture architecture> struct format_traits_t {};

	// metadata_t
	//
	// Provides metadata about the loaded Elf image
	struct metadata_t
	{
		void const*		baseaddress = nullptr;
		void const*		breakaddress = nullptr;
		void const*		entrypoint = nullptr;
		void const*		progheaders = nullptr;
		size_t			numprogheaders = 0;
		std::string		interpreter;
	};

	// Instance Constructor
	//
	ElfBinary(metadata_t&& metadata);
	friend std::unique_ptr<ElfBinary> std::make_unique<ElfBinary, metadata_t>(metadata_t&&);

	//-------------------------------------------------------------------------
	// Member Variables

	metadata_t const		m_metadata;			// Loaded image metadata
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFBINARY_H_
