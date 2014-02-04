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
#include "ElfSegment64.h"				// Include ELFSegment64 declarations

#include "Exception.h"					// Include Exception declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// ElfSegment64 Constructor
//
// Arguments:
//
//	header		- Pointer to the segment program header
//	reader		- Image stream reader

ElfSegment64::ElfSegment64(const Elf64_Phdr* header, std::unique_ptr<StreamReader>& reader) : m_base(NULL)
{
	if(!header) throw Exception(E_POINTER);
	if(header->p_offset != reader->Position) throw Exception(E_ABORT);

	// Create a copy of the provided segment program header
	memcpy(&m_header, header, sizeof(Elf64_Phdr));

	// Only allocate virtual memory if the segment memory size is non-zero
	if(m_header.p_memsz) {

		//
		// TODO: Just allocating and expanding segment for now; needs to try
		// to be loaded at the right place
		//

		// TODO: FlushInstructionCache() when making PAGE_EXECUTE

		// Allocate the segment memory as READWRITE first to allow for manipulations
		m_base = VirtualAlloc(NULL, m_header.p_memsz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		if(!m_base) throw Exception(GetLastError());

		uint32_t out = reader->Read(m_base, m_header.p_filesz);
		if(out != m_header.p_filesz) { 
			
			VirtualFree(m_base, m_header.p_memsz, MEM_RELEASE); 
			throw Exception(E_ABORT);
		}

		// Memory that was not loaded from the ELF image must be initialized to zero
		memset(reinterpret_cast<uint8_t*>(m_base) + m_header.p_filesz, 0, m_header.p_memsz - m_header.p_filesz);
	}
}

//-----------------------------------------------------------------------------
// ElfSegment64 Destructor

ElfSegment64::~ElfSegment64()
{
	if(m_base) { VirtualFree(m_base, m_header.p_memsz, MEM_RELEASE); }
	m_base = NULL;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
