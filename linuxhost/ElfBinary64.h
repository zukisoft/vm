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

#ifndef __ELFBINARY64_H_
#define __ELFBINARY64_H_
#pragma once

#include "elf.h"						// Include ELF file format decls
#include "ElfBinary.h"					// Include ElfBinary declarations
//#include "MappedFileView.h"				// Include MappedFileView declarations

class MappedFileView;

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// ElfBinary64
//
// Specialization of the ElfBinary class for a 64-bit image

class ElfBinary64 : public ElfBinary
{
public:

	// Destructor
	//
	virtual ~ElfBinary64() {}

	//-------------------------------------------------------------------------
	// Member Functions

	// Load
	//
	// Parses and loads the specified ELF image into virtual memory
	static ElfBinary64* Load(std::unique_ptr<StreamReader>& reader);

private:

	ElfBinary64(std::unique_ptr<MappedFileView>& view);
	ElfBinary64(const ElfBinary64&);
	ElfBinary64& operator=(const ElfBinary64&);

	//-------------------------------------------------------------------------
	// Member Variables

	HANDLE					m_mapping;			// Memory mapped binary image
	Elf64_Ehdr				m_header;			// ELF64 header data
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFBINARY64_H_
