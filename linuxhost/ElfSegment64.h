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

#ifndef __ELFSEGMENT64_H_
#define __ELFSEGMENT64_H_
#pragma once

#include "ElfSegment.h"					// Include ElfSegment declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// ElfSegment64
//
// Specialization of the ElfSegment class for a 64-bit image

class ElfSegment64 : public ElfSegment
{
public:

	// Constructor / Destructor
	//
	ElfSegment64(const Elf64_Phdr* header, std::unique_ptr<StreamReader>& reader);
	virtual ~ElfSegment64();

private:

	ElfSegment64(const ElfSegment64&);
	ElfSegment64& operator=(const ElfSegment64&);

	//-------------------------------------------------------------------------
	// Member Variables

	Elf64_Phdr				m_header;				// Segment header
	void*					m_base;					// Allocated base address
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFSEGMENT64_H_
