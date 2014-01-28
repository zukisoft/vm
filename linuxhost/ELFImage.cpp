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

#include "stdafx.h"						// Include project pre-compiled headers
#include "ELFImage.h"					// Include ELFImage declarations

#include "elf.h"						// Include ELF format declarations
#include "Exception.h"					// Include Exception declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// ELFImage Constructor
//
// Arguments:
//
//	base		- Base address of the loaded ELF image
//	length		- Length of the loaded ELF image

ELFImage::ELFImage(const void* base, size_t length)
{
	if(!base) throw Exception(E_POINTER);
	if(length == 0) throw Exception(E_INVALIDARG);
	if(!IsValid(base, length)) throw Exception(E_INVALIDARG, _T("ELF header validation failed"));

	m_base = base;
	m_length = length;
}

//-----------------------------------------------------------------------------
// ELFImage::IsValid (static)
//
// Validates that the specified address points to an ELF header
//
// Arguments:
//
//	base		- ELF image base address
//	length		- Length of the provided image

bool ELFImage::IsValid(const void* base, size_t length)
{
	if(!base) return false;

	// Verify that the length is at least large enough for the smaller 32-bit header
	if(length < sizeof(elf32_hdr)) return false;

	// Verify the ELF header magic number
	if(memcmp(base, ELFMAG, SELFMAG) != 0) return false;

	//
	// TODO: Should perform more than just magic number validation
	//

	return true;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
