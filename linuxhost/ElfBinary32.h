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

#ifndef __ELFBINARY32_H_
#define __ELFBINARY32_H_
#pragma once

#include "elf.h"						// Include ELF file format decls
#include "ElfBinary.h"					// Include ElfBinary declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// ElfBinary32
//
// Specialization of the ElfBinary class for a 32-bit image

class ElfBinary32 : public ElfBinary
{
public:

	// Instance Constructors
	//
	ElfBinary32(const void* base, size_t length);
	ElfBinary32(const uint8_t* ident, size_t identlen, std::unique_ptr<StreamReader>& reader);

	// Destructor
	//
	virtual ~ElfBinary32() {}

private:

	ElfBinary32(const ElfBinary32&);
	ElfBinary32& operator=(const ElfBinary32&);

	//-------------------------------------------------------------------------
	// Member Variables

	Elf32_Ehdr				m_header;			// ELF32 header data
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFBINARY32_H_
