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

// TODO: Load can be done with a StreamReader to avoid decompressing the 
// image first.  Just detect overlapping segments and read the overlap from
// the previously loaded segment.  They are presented in order.

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
//	reader		- StreamReader open against the ELF image to load

template <class ehdr_t, class phdr_t, class shdr_t>
ElfImageT<ehdr_t, phdr_t, shdr_t>::ElfImageT(std::unique_ptr<StreamReader>& reader)
{
	uint32_t				out;				// Number of bytes read from the stream
	DWORD					result;				// Result from function call

	// Query the host system information so the page boundaries can be validated
	SYSTEM_INFO sysinfo;
	GetNativeSystemInfo(&sysinfo);

	// Read and validate the ELF header from the beginning of the stream
	ehdr_t	elfheader;
	if(!reader->TryRead(&elfheader, sizeof(ehdr_t), &out)) throw Exception(E_TRUNCATEDELFHEADER);
	ValidateHeader(&elfheader, out);
	
	// Make sure there is at least one program header in the ELF image
	if(elfheader.e_phnum == 0) throw Exception(E_INVALIDELFPROGRAMTABLE);

#ifdef _M_X64
	if(elfheader.e_phoff > UINT32_MAX) throw Exception(E_INVALIDELFPROGRAMTABLE);
#endif
	
	// Create a collection of all the ELF program headers
	std::vector<phdr_t>	progheaders;
	for(int index = 0; index < elfheader.e_phnum; index++) {

		// Move the input stream to the location of the next section header
		if(!reader->TrySeek(static_cast<uint32_t>(elfheader.e_phoff + (index * elfheader.e_phentsize))))
			throw Exception(E_INVALIDELFPROGRAMTABLE);

		// Attept to read the program header structure from the input stream
		phdr_t progheader;
		if((!reader->TryRead(&progheader, sizeof(phdr_t), &out)) || (out != sizeof(phdr_t)))
			throw Exception(E_INVALIDELFPROGRAMTABLE);

		progheaders.push_back(progheader);			// Insert into the collection
	}

	// Determine the memory requirements of the loaded image
	intptr_t minaddress = 0, maxaddress = 0;
	for_each(progheaders.begin(), progheaders.end(), [&](phdr_t& phdr) {

		if(phdr.p_type == PT_LOAD) {

			// The segment must start on a host system page boundary
			if((phdr.p_paddr % sysinfo.dwPageSize) != 0) throw Exception(E_UNEXPECTED);	// <--- TODO: exception
			
			// Calculate the minimum and maximum physical addresses of the segment
			// and adjust the overall minimum and maximums accordingly
			intptr_t minsegaddr(phdr.p_paddr);
			intptr_t maxsegaddr(phdr.p_paddr + phdr.p_memsz);

			minaddress = (minaddress == 0) ? minsegaddr : min(minsegaddr, minaddress);
			maxaddress = (maxaddress == 0) ? maxsegaddr : max(maxsegaddr, maxaddress);
		}
	});

	// Attempt to allocate the virtual memory for the image.  First try to use the physical address specified
	// by the image to avoid relocations, but go ahead and put it anywhere if that doesn't work
	try { m_memory.reset(MemoryRegion::Allocate(maxaddress - minaddress, PAGE_READWRITE, reinterpret_cast<void*>(minaddress))); }
	catch(Exception&) { m_memory.reset(MemoryRegion::Allocate(maxaddress - minaddress, PAGE_READWRITE, MEM_TOP_DOWN)); }

#ifdef _DEBUG
	// Fill the memory with some junk bytes in DEBUG builds to better detect uninitialized memory
	memset(m_memory->Pointer, 0xCD, maxaddress - minaddress);
#endif
	
	// Load the program segments into memory
	intptr_t baseptr = intptr_t(m_memory->Pointer);
	for_each(progheaders.begin(), progheaders.end(), [&](phdr_t& phdr) {

		//
		// NEED TO DEAL WITH PT_PHDR TYPE HERE

		if((phdr.p_type == PT_LOAD) && (phdr.p_memsz)) {

			intptr_t segbase = baseptr + (phdr.p_paddr - minaddress);

			if(phdr.p_filesz) {

				//
				// NEED TO DEAL WITH OVERLAPPING OFFSETS HERE
				//

#ifdef _M_X64
				// TODO: Make special program table exceptions like the elf header
				if(phdr.p_offset > UINT32_MAX) throw Exception(E_INVALIDELFPROGRAMTABLE);
				if(phdr.p_filesz > UINT32_MAX) throw Exception(E_INVALIDELFPROGRAMTABLE);
				if(phdr.p_memsz > UINT32_MAX) throw Exception(E_INVALIDELFPROGRAMTABLE);
#endif
				reader->Seek(static_cast<uint32_t>(phdr.p_offset));
				out = reader->Read(reinterpret_cast<void*>(segbase), static_cast<uint32_t>(phdr.p_filesz));
				if(out != phdr.p_filesz) throw Exception(E_ELF_TRUNCATED);
			}

			// Memory that was not loaded from the ELF image must be initialized to zero
			memset(reinterpret_cast<void*>(intptr_t(segbase) + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);

			// Attempt to apply the proper virtual memory protection flags to the segment
			if(!VirtualProtect(reinterpret_cast<void*>(segbase), phdr.p_memsz, FlagsToProtection(phdr.p_flags), &result))
				throw Win32Exception();		// <--- make custom exception here
		}
	});

	// Calculate entry point
	// Handle relocations
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

template <class ehdr_t, class phdr_t, class shdr_t>
ElfImageT<ehdr_t, phdr_t, shdr_t>* ElfImageT<ehdr_t, phdr_t, shdr_t>::Load(std::shared_ptr<MappedFile>& mapping)
{
	(mapping);
	throw Exception(E_NOTIMPL);
//
//#ifdef _M_X64
//	if(mapping->Capacity > UINT32_MAX) throw Exception(E_UNEXPECTED);					// <-- TODO: better error
//#endif
//
//	// Validate the header that should be at the view base address
//	std::unique_ptr<MappedFileView> view(MappedFileView::Create(mapping, FILE_MAP_READ, 0, sizeof(ehdr_t)));
//	ValidateHeader(view->Pointer, view->Length);
//	view.reset();
//
//	// Create a new ElfImageT instance fron the provided view
//	return new ElfImageT<ehdr_t, phdr_t, shdr_t>(mapping);
}

//-----------------------------------------------------------------------------
// ElfImageT::Load (static)
//
// Parses and loads the specified ELF image into virtual memory
//
// Arguments:
//
//	reader		- Pointer to a StreamReader used to access the raw image

template <class ehdr_t, class phdr_t, class shdr_t>
ElfImageT<ehdr_t, phdr_t, shdr_t>* ElfImageT<ehdr_t, phdr_t, shdr_t>::Load(std::unique_ptr<StreamReader>& reader)
{
	return new ElfImageT<ehdr_t, phdr_t, shdr_t>(reader);

//	ehdr_t				header;					// ELF header structure
//	uint32_t			out;					// Bytes read from the input stream
//	size_t				length;					// Length of the uncompressed image
//
//	// Read the ELF header from the beginning of the stream
//	out = reader->Read(&header, sizeof(ehdr_t));
//	if(out != sizeof(ehdr_t)) throw Exception(E_TRUNCATEDELFHEADER);
//
//	// Validate the contents of the header data
//	ValidateHeader(&header, out);
//
//	// Determine the length of the uncompressed image.  If there is a section header table, use
//	// that since it's at the end of the file, otherwise the program header table has to be 
//	// examined to calculate the length of the available segments
//	length = sizeof(ehdr_t);
//	if(header.e_shnum) length = header.e_shoff + (header.e_shnum * header.e_shentsize);
//	else if(header.e_phnum) {
//
//		// Skip to the beginning of the program header table in the input stream
//#ifdef _M_X64
//		if(header.e_phoff > UINT32_MAX) throw Exception(E_UNEXPECTED);	// <-- TODO: better error
//#endif
//		reader->Seek(static_cast<uint32_t>(header.e_phoff));
//
//		std::unique_ptr<uint8_t[]> buffer(new uint8_t[header.e_phentsize]);
//		for(int index = 0; index < header.e_phnum; index++) {
//
//			// Read in the next program header table entry
//			out = reader->Read(&buffer[0], header.e_phentsize);
//			if(out != header.e_phentsize) throw Exception(E_INVALIDELFPROGRAMTABLE);
//
//			// Cast a pointer to the ELF program header table structure
//			phdr_t* phdr = reinterpret_cast<phdr_t*>(&buffer[0]);
//			
//			// Check this segment offset + length against the current known length
//			size_t max = phdr->p_offset + phdr->p_filesz;
//			if(max > length) length = max;
//		}
//	}
//
//	// Ensure the length does not exceed UINT32_MAX and reset the stream
//#ifdef _M_X64
//	if(length > UINT32_MAX) throw Exception(E_UNEXPECTED);					// <-- TODO: better error
//#endif
//
//	reader->Reset();
//
//	// Create a pagefile-backed memory mapped file view to hold the uncompressed binary image
//	std::shared_ptr<MappedFile> mapping(MappedFile::CreateNew(PAGE_EXECUTE_READWRITE | SEC_COMMIT, length));
//
//	// Decompress the ELF image into the memory mapped file
//	std::unique_ptr<MappedFileView> view(MappedFileView::Create(mapping, FILE_MAP_WRITE, 0, length));
//	out = reader->Read(view->Pointer, static_cast<uint32_t>(length));
//	if(out != length) throw Exception(E_ELF_TRUNCATED);
//	view.reset();
//
//	return new ElfImageT<ehdr_t, phdr_t, shdr_t>(mapping);
}

//-----------------------------------------------------------------------------
// ElfImageT::TryValidateHeader
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
// ElfImageT::ValidateHeader
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
