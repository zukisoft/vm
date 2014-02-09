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
#include "ElfSegment.h"					// Include ELFSegment declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// Explicit Instantiations

#ifdef _M_X64
template ElfSegmentT<Elf64_Phdr>;
#else
template ElfSegmentT<Elf32_Phdr>;
#endif

//-----------------------------------------------------------------------------
// ElfSegmentT Constructor
//
// Arguments:
//
//	header		- Pointer to the segment program header
//	view		- Memory mapped view of the ELF binary image

template <class phdr_t>
ElfSegmentT<phdr_t>::ElfSegmentT(const phdr_t* header, std::unique_ptr<MappedFileView>& view) : m_base(NULL)
{
	DWORD				result;						// Result from function call

	if(!header) throw Exception(E_POINTER);
	if(header->p_type != PT_LOAD) throw Exception(E_INVALIDARG);

	// Create a copy of the provided segment program header
	memcpy(&m_header, header, sizeof(phdr_t));

	// Get the base address of the binary image as an intptr_t for offset calculations
	intptr_t viewbase = intptr_t(view->Pointer);

	// Only allocate virtual memory if the segment memory size is non-zero
	if(m_header.p_memsz) {

		// TODO: FlushInstructionCache() when making PAGE_EXECUTE
		// TODO: p_align needs to be dealt with
		// TODO: load at the proper address??

		// Allocate the segment memory as READWRITE first to allow for manipulations
		m_base = VirtualAlloc(NULL, m_header.p_memsz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		if(!m_base) throw Win32Exception();

		// Copy the segment data from the image file into the allocated memory
		if(m_header.p_filesz)
			memcpy(m_base, reinterpret_cast<void*>(viewbase + m_header.p_offset), m_header.p_filesz);

		// Memory that was not loaded from the ELF image must be initialized to zero
		memset(reinterpret_cast<void*>(intptr_t(m_base) + m_header.p_filesz), 0, m_header.p_memsz - m_header.p_filesz);

		// Apply the proper virtual memory protection flags to the segment
		if(!VirtualProtect(m_base, m_header.p_memsz, FlagsToProtection(m_header.p_flags), &result)) {

			result = GetLastError();
			VirtualFree(m_base, m_header.p_memsz, MEM_RELEASE);
			m_base = NULL;

			throw Win32Exception(result);
		}
	}
}

//-----------------------------------------------------------------------------
// ElfSegmentT Destructor

template <class phdr_t>
ElfSegmentT<phdr_t>::~ElfSegmentT()
{
	if(m_base) { VirtualFree(m_base, m_header.p_memsz, MEM_RELEASE); }
	m_base = NULL;
}


//-----------------------------------------------------------------------------
// ElfSegmentT::FlagsToProtection (private, static)
//
// Converts an ELF program header p_flags into VirtualAlloc() protection flags
//
// Arguments:
//
//	flags		- ELF program header p_flags value

template <class phdr_t>
DWORD ElfSegmentT<phdr_t>::FlagsToProtection(uint32_t flags)
{
	switch(flags) {

		case PF_X:					return PAGE_EXECUTE;
		case PF_W :					return PAGE_READWRITE;
		case PF_R :					return PAGE_READONLY;
		case PF_X | PF_W :			return PAGE_EXECUTE_READWRITE;
		case PF_X | PF_R :			return PAGE_EXECUTE_READ;
		case PF_W | PF_R :			return PAGE_READWRITE;
		case PF_X | PF_W | PF_R :	return PAGE_EXECUTE_READWRITE;
	}

	return PAGE_NOACCESS;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
