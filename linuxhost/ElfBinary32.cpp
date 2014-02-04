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
#include "ElfBinary32.h"				// Include ELFBinary32 declarations

#include "BufferStreamReader.h"			// Include BufferStreamReader declarations
#include "Exception.h"					// Include Exception declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// ElfBinary32 Constructor
//
// Arguments:
//
//	base		- Base address of the binary ELF image
//	length		- Length of the binary ELF image

ElfBinary32::ElfBinary32(const void* base, size_t length)
{
	// Verify that the buffer contains enough data to at least process the header
	if((!base) || (length < sizeof(Elf32_Ehdr))) throw Exception(E_POINTER);

	// Check the ELF header magic number
	if(memcmp(base, ELFMAG, SELFMAG) != 0) throw Exception(E_INVALIDELFMAGIC);

	// Copy the header data into the ELF header structure
	memcpy(&m_header, base, sizeof(Elf32_Ehdr));

	// Verify that a 32-bit ELF binary is being processed by the class
	if(m_header.e_ident[EI_CLASS] != ELFCLASS32) 
		throw Exception(E_UNEXPECTEDELFCLASS, m_header.e_ident[EI_CLASS]);

	// Verify the endianness of the ELF binary -- must be little endian
	if(m_header.e_ident[EI_DATA] != ELFDATA2LSB) throw Exception(E_UNEXPECTEDELFENCODING);

	// TODO: verify i386 machine
}

//-----------------------------------------------------------------------------
// ElfBinary32 Constructor
//
// Arguments:
//
//	ident		- Pointer to ELF header e_ident buffer
//	identlen	- Length of the e_ident buffer specified by ident
//	reader		- Binary image reader; expected to be pointing immediately after e_ident data

ElfBinary32::ElfBinary32(const uint8_t* ident, size_t identlen, std::unique_ptr<StreamReader>& reader)
{
	uint32_t			out;					// Bytes read from the input stream

	// Verify that the ident buffer contains the required amount of data
	if((!ident) || (identlen != EI_NIDENT)) throw Exception(E_POINTER);

	// Check the ELF header magic number and copy the e_ident data
	if(memcmp(ident, ELFMAG, SELFMAG) != 0) throw Exception(E_INVALIDELFMAGIC);
	memcpy(&m_header.e_ident, ident, EI_NIDENT);

	// Read the remainder of the ELF header from the input stream
	out = reader->Read(&m_header.e_type, sizeof(Elf32_Ehdr) - EI_NIDENT);
	if(out != (sizeof(Elf32_Ehdr) - EI_NIDENT)) throw Exception(E_TRUNCATEDELFHEADER);

	// Verify that a 32-bit ELF binary is being processed by the class
	if(m_header.e_ident[EI_CLASS] != ELFCLASS32) 
		throw Exception(E_UNEXPECTEDELFCLASS, m_header.e_ident[EI_CLASS]);

	// Verify the endianness of the ELF binary -- must be little endian
	if(m_header.e_ident[EI_DATA] != ELFDATA2LSB) throw Exception(E_UNEXPECTEDELFENCODING);

	// TODO: verify i386 machine
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
