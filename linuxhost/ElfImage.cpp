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
#include "ElfImage.h"					// Include ELFImage declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// Explicit Instantiations

#ifdef _M_X64
template ElfImageT<Elf64_Ehdr, Elf64_Phdr, Elf64_Shdr>;
#else
template ElfImageT<Elf32_Ehdr, Elf32_Phdr, Elf32_Shdr>;
#endif

//-----------------------------------------------------------------------------
// ElfImageT Constructor (private)
//
// Arguments:
//
//	mapping		- Memory-mapped ELF image file to load
//	length		- Optional length to use from mapping

template <class ehdr_t, class phdr_t, class shdr_t>
ElfImageT<ehdr_t, phdr_t, shdr_t>::ElfImageT(std::shared_ptr<MappedFile>& mapping, size_t length)
{
	// Create a read-only view of the memory-mapped ELF image
	std::unique_ptr<MappedFileView> view(MappedFileView::Create(mapping, FILE_MAP_READ, 0, length));
	intptr_t viewptr = intptr_t(view->Pointer);

	// Validate the ELF header that should be at the start of the view
	ValidateHeader(view->Pointer, view->Length);
	ehdr_t*	elfheader = reinterpret_cast<ehdr_t*>(view->Pointer);

	// Create a collection of all the ELF program headers
	std::vector<phdr_t>	progheaders;
	for(int index = 0; index < elfheader->e_phnum; index++) {

		// Get the offset into the file for the next program header and check it
		intptr_t offset = elfheader->e_phoff + (index * elfheader->e_phentsize);
		if(view->Length < (offset + sizeof(phdr_t))) throw Exception(E_ELF_TRUNCATED);

		// Insert the program header into the collection
		progheaders.push_back(*reinterpret_cast<phdr_t*>(viewptr + offset));
	}

	// Determine the memory requirements of the loaded image
	uintptr_t minaddress = 0, maxaddress = 0;
	for_each(progheaders.begin(), progheaders.end(), [&](phdr_t& phdr) {

		if(phdr.p_type == PT_LOAD) {

			// The segment must align on the host system
			if((phdr.p_align % MemoryRegion::PageSize) != 0) throw Exception(E_ELFSEGMENTPAGEBOUNDARY);
			
			// Calculate the minimum and maximum physical addresses of the segment
			// and adjust the overall minimum and maximums accordingly
			uintptr_t minsegaddr(phdr.p_paddr);
			uintptr_t maxsegaddr(phdr.p_paddr + phdr.p_memsz);

			minaddress = (minaddress == 0) ? minsegaddr : min(minsegaddr, minaddress);
			maxaddress = (maxaddress == 0) ? maxsegaddr : max(maxsegaddr, maxaddress);
		}
	});

	// Attempt to allocate the virtual memory for the image.  First try to use the physical address specified
	// by the image to avoid relocations, but go ahead and put it anywhere if that doesn't work
	try { m_region.reset(MemoryRegion::Allocate(maxaddress - minaddress, PAGE_READWRITE, reinterpret_cast<void*>(minaddress))); }
	catch(Exception&) { m_region.reset(MemoryRegion::Allocate(maxaddress - minaddress, PAGE_READWRITE, MEM_TOP_DOWN)); }
	intptr_t regionptr = intptr_t(m_region->Pointer);

#ifdef _DEBUG
	// Fill the memory with some junk bytes in DEBUG builds to better detect uninitialized memory
	memset(m_region->Pointer, 0xCD, maxaddress - minaddress);
#endif
	
	// Load the PT_LOAD segments into virtual memory
	for_each(progheaders.begin(), progheaders.end(), [&](phdr_t& phdr) {

		if((phdr.p_type == PT_LOAD) && (phdr.p_memsz)) {

			// Get the base address of the loadable segment
			intptr_t segbase = regionptr + (phdr.p_paddr - minaddress);

			// Not all segments contain data that needs to be copied from the source image
			if(phdr.p_filesz) {

				// Ensure that there is enough source data to copy and copy it into the segment region
				if(view->Length < (phdr.p_offset + phdr.p_filesz)) throw Exception(E_ELF_TRUNCATED);
				memcpy(reinterpret_cast<void*>(segbase), reinterpret_cast<void*>(viewptr + phdr.p_offset), phdr.p_filesz);
			}

			// Memory that was not loaded from the ELF image must be initialized to zero
			memset(reinterpret_cast<void*>(segbase + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);

			// Attempt to apply the proper virtual memory protection flags to the segment
			try { m_region->Protect(segbase, phdr.p_memsz, FlagsToProtection(phdr.p_flags)); }
			catch(Exception& ex) { throw Exception(ex, E_ELFSEGMENTPROTECTION); }
		}
	});

	// Calculate the address of the image entry point, if one has been specified in the header
	m_entry = (elfheader->e_entry) ? reinterpret_cast<EntryPoint>(regionptr + (elfheader->e_entry - minaddress)) : nullptr;
}

//---------------------------------------------------------------------------
// ElfImageT::AlignDown (private, static)
//
// Aligns an offset down to the specified alignment
//
// Arguments:
//
//	address		- Address to be aligned
//	alignment	- Alignment

template <class ehdr_t, class phdr_t, class shdr_t>
uintptr_t ElfImageT<ehdr_t, phdr_t, shdr_t>::AlignDown(uintptr_t address, size_t alignment)
{
	if(alignment < 1) throw Exception(E_ARGUMENTOUTOFRANGE, _T("alignment"));

	if(address < alignment) return 0;
	else return AlignUp(address - (alignment - 1), alignment);
}

//---------------------------------------------------------------------------
// ElfImageT::AlignUp (private, static)
//
// Aligns an offset up to the specified alignment
//
// Arguments:
//
//	address		- Address to be aligned
//	alignment	- Alignment

template <class ehdr_t, class phdr_t, class shdr_t>
uintptr_t ElfImageT<ehdr_t, phdr_t, shdr_t>::AlignUp(uintptr_t address, size_t alignment)
{
	if(alignment < 1) throw Exception(E_ARGUMENTOUTOFRANGE, _T("alignment"));

	if(address == 0) return 0;
	else return address + ((alignment - (address % alignment)) % alignment);
}

//-----------------------------------------------------------------------------
// ElfImageT::FlagsToProtection (private, static)
//
// Converts an ELF program header p_flags into VirtualAlloc() protection flags
//
// Arguments:
//
//	flags		- ELF program header p_flags value

template <class ehdr_t, class phdr_t, class shdr_t>
DWORD ElfImageT<ehdr_t, phdr_t, shdr_t>::FlagsToProtection(uint32_t flags)
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
// ElfImageT::Load (static)
//
// Parses and loads the specified ELF image into virtual memory
//
// Arguments:
//
//	mapping			- Memory-mapped image file
//	length			- Optional length of mapping to use

template <class ehdr_t, class phdr_t, class shdr_t>
ElfImageT<ehdr_t, phdr_t, shdr_t>* ElfImageT<ehdr_t, phdr_t, shdr_t>::Load(std::shared_ptr<MappedFile>& mapping, size_t length)
{
	// All the validation code has been moved into the constructor
	return new ElfImageT<ehdr_t, phdr_t, shdr_t>(mapping, length);
}

//-----------------------------------------------------------------------------
// ElfImageT::TryValidateHeader (static)
//
// Validates that the provided pointer points to a 64-bit ELF binary header --
// returns a boolean flag rather than throwing an exception
//
// Arguments:
//
//	base		- Base address of the ELF binary to test
//	length		- Length of the buffer pointed to by base

template <class ehdr_t, class phdr_t, class shdr_t>
bool ElfImageT<ehdr_t, phdr_t, shdr_t>::TryValidateHeader(const void* base, size_t length)
{
	// Invoke the version that throws exceptions and just eat them
	try { ValidateHeader(base, length); return true; }
	catch(Exception&) { return false; }
}

//-----------------------------------------------------------------------------
// ElfImageT::ValidateHeader (static)
//
// Validates that the provided pointer points to a 64-bit ELF binary header
//
// Arguments:
//
//	base		- Base address of the ELF binary to test
//	length		- Length of the buffer pointed to by base

template <class ehdr_t, class phdr_t, class shdr_t>
void ElfImageT<ehdr_t, phdr_t, shdr_t>::ValidateHeader(const void* base, size_t length)
{
	const ehdr_t*				header;					// ELF header structure

	// Check the length and cast out a pointer to the header structure
	if(length < sizeof(ehdr_t)) throw Exception(E_TRUNCATEDELFHEADER);
	header = reinterpret_cast<const ehdr_t*>(base);

	// Check the ELF header magic number
	if(memcmp(&header->e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0) throw Exception(E_INVALIDELFMAGIC);

	// Verify the ELF class matches the build configuration (32-bit vs. 64-bit)
	int elfclass = (sizeof(ehdr_t) == sizeof(Elf32_Ehdr)) ? ELFCLASS32 : ELFCLASS64;
	if(header->e_ident[EI_CLASS] != elfclass) throw Exception(E_UNEXPECTEDELFCLASS, header->e_ident[EI_CLASS]);

	// Verify the endianness and version of the ELF binary
	if(header->e_ident[EI_DATA] != ELFDATA2LSB) throw Exception(E_UNEXPECTEDELFENCODING);
	if(header->e_ident[EI_VERSION] != EV_CURRENT) throw Exception(E_UNKNOWNELFVERSION);

	// TODO: Verify x86 / x86_64 machine - check size of ehdr_t to know which
	// TODO: Verify object file type

	// Verify that the length of the header is the same size as the Elfxx_Ehdr struct
	// and that the header entries are at least as big as the known structures
	if(header->e_ehsize != sizeof(ehdr_t)) throw Exception(E_UNKNOWNELFVERSION);
	if((header->e_phentsize) && (header->e_phentsize < sizeof(phdr_t))) throw Exception(E_INVALIDELFPROGRAMTABLE);
	if((header->e_shentsize) && (header->e_shentsize < sizeof(shdr_t))) throw Exception(E_INVALIDELFSECTIONTABLE);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
