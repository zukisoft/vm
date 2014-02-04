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
#include "ElfBinary.h"					// Include ELFBinary declarations

#include "BufferStreamReader.h"			// Include BufferStreamReader declarations
#include "ElfBinary32.h"				// Include ElfBinary32 declarations
#include "ElfBinary64.h"				// Include ElfBinary64 declarations
#include "Exception.h"					// Include Exception declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// ElfBinary::IsElfBinary (static)
//
// Validates that the specified address points to an ELF header
//
// Arguments:
//
//	base		- ELF image base address
//	length		- Length of the provided image

bool ElfBinary::IsElfBinary(const void* base, size_t length)
{
	if(!base) return false;

	// Verify that the length is large enough for the e_ident data
	if(length < EI_NIDENT) return false;
	const uint8_t* e_ident = reinterpret_cast<const uint8_t*>(base);

	// Verify the ELF header magic number
	if(memcmp(&e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0) return false;

	// Verify the length against the ELFCLASS of the binary
	if(e_ident[EI_CLASS] == ELFCLASS32) return (length >= sizeof(Elf32_Ehdr));
	else if(e_ident[EI_CLASS] == ELFCLASS64) return (length >= sizeof(Elf64_Ehdr));

	return false;
}

//-----------------------------------------------------------------------------
// ElfBinary::Load (static)
//
// Parses and loads the specified ELF image into virtual memory
//
// Arguments:
//
//	reader		- Pointer to a StreamReader used to access the raw image

ElfBinary* ElfBinary::Load(std::unique_ptr<StreamReader>& reader)
{
	uint32_t		out;					// Bytes read from the input stream
	uint8_t			ident[EI_NIDENT];		// ELF binary identification data

	// Read the identification information from the beginning of the input stream
	out = reader->Read(&ident, EI_NIDENT);
	if(out != EI_NIDENT) throw Exception(E_FAIL, _T("bad juju - message here"));

	reader->Reset();						// Reset the stream back to the start

	// TODO: .IsValid(&ident, E_NIDENT);

	if(ident[EI_CLASS] == ELFCLASS32) return new ElfBinary32(ident, EI_NIDENT, reader);
	else if(ident[EI_CLASS] == ELFCLASS64) return ElfBinary64::Load(reader);
	else throw Exception(E_FAIL, _T("bad juju - invalid ELF binary class"));
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
