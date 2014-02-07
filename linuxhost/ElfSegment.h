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

#ifndef __ELFSEGMENT_H_
#define __ELFSEGMENT_H_
#pragma once

#include "elf.h"						// Include ELF format declarations
#include "Exception.h"					// Include Exception declarations
#include "MappedFileView.h"				// Include MappedFileView declarations
#include "Win32Exception.h"				// Include Win32Exception declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// ElfSegmentT
//
// Represents a single loaded segment of an ELF binary image
//
//	phdr_t		- ELF program header structure type

template <class phdr_t>
class ElfSegmentT
{
public:

	// Constructor / Destructor
	//
	ElfSegmentT(const phdr_t* header, std::unique_ptr<MappedFileView>& view);
	~ElfSegmentT();

private:

	ElfSegmentT(const ElfSegmentT&);
	ElfSegmentT& operator=(const ElfSegmentT&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// FlagsToProtection
	//
	// Converts the ELF p_flags into VirtualAlloc protection flags
	static DWORD FlagsToProtection(uint32_t flags);

	//-------------------------------------------------------------------------
	// Member Variables

	phdr_t					m_header;				// Segment header
	void*					m_base;					// Allocated base address
};

//-----------------------------------------------------------------------------
// ElfSegment
//
// Typedef of ElfSegmentT<> based on build configuration

#ifdef _M_X64
typedef ElfSegmentT<Elf64_Phdr>		ElfSegment;
#else
typedef ElfSegmentT<Elf32_Phdr>		ElfSegment;
#endif

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFSEGMENT_H_
