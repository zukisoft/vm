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
//	mapping			- Memory-mapped binary image file

template <class ehdr_t, class phdr_t, class shdr_t>
ElfImageT<ehdr_t, phdr_t, shdr_t>::ElfImageT(std::shared_ptr<MappedFile> mapping)
{
	// Map a read-only view of the entire image file so that it can be processed
	std::unique_ptr<MappedFileView> view(new MappedFileView(mapping, FILE_MAP_READ, 0, 0));

	// The header has already been validated by this point, just copy it out
	memcpy(&m_header, view->Pointer, sizeof(ehdr_t));
	
	// Iterate over all the entries in the program header table to build the segments
	intptr_t phdrbase = intptr_t(view->Pointer) + m_header.e_phoff;
	for(int index = 0; index < m_header.e_phnum; index++) {

		const phdr_t* phdr = reinterpret_cast<const phdr_t*>(phdrbase + (index * m_header.e_phentsize));

		// Currently only looking for loadable program segments
		if(phdr->p_type == PT_LOAD) {

			// Create the new segment and add to parent class collection
			std::unique_ptr<ElfSegment> segment(new ElfSegment(phdr, view));
			m_segments.push_back(std::move(segment));
		}
	}
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
ElfImageT<ehdr_t, phdr_t, shdr_t>* ElfImageT<ehdr_t, phdr_t, shdr_t>::Load(std::shared_ptr<MappedFile> mapping)
{

#ifdef _M_X64
	if(mapping->Length > UINT32_MAX) throw Exception(E_UNEXPECTED);					// <-- TODO: better error
#endif

	// Validate the header that should be at the view base address
	std::unique_ptr<MappedFileView> view(new MappedFileView(mapping, FILE_MAP_READ, 0, sizeof(ehdr_t)));
	ValidateHeader(view->Pointer, view->Length);
	view.release();

	// Create a new ElfImageT instance fron the provided view
	return new ElfImageT<ehdr_t, phdr_t, shdr_t>(mapping);
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
	ehdr_t				header;					// ELF header structure
	uint32_t			out;					// Bytes read from the input stream
	size_t				length;					// Length of the uncompressed image

	// Read the ELF header from the beginning of the stream
	out = reader->Read(&header, sizeof(ehdr_t));
	if(out != sizeof(ehdr_t)) throw Exception(E_TRUNCATEDELFHEADER);

	// Validate the contents of the header data
	ValidateHeader(&header, out);

	// Determine the length of the uncompressed image.  If there is a section header table, use
	// that since it's at the end of the file, otherwise the program header table has to be 
	// examined to calculate the length of the available segments
	length = sizeof(ehdr_t);
	if(header.e_shnum) length = header.e_shoff + (header.e_shnum * header.e_shentsize);
	else if(header.e_phnum) {

		// Skip to the beginning of the program header table in the input stream
#ifdef _M_X64
		if(header.e_phoff > UINT32_MAX) throw Exception(E_UNEXPECTED);	// <-- TODO: better error
#endif
		reader->Seek(static_cast<uint32_t>(header.e_phoff));

		std::unique_ptr<uint8_t[]> buffer(new uint8_t[header.e_phentsize]);
		for(int index = 0; index < header.e_phnum; index++) {

			// Read in the next program header table entry
			out = reader->Read(&buffer[0], header.e_phentsize);
			if(out != header.e_phentsize) throw Exception(E_INVALIDELFPROGRAMTABLE);

			// Cast a pointer to the ELF program header table structure
			phdr_t* phdr = reinterpret_cast<phdr_t*>(&buffer[0]);
			
			// Check this segment offset + length against the current known length
			size_t max = phdr->p_offset + phdr->p_filesz;
			if(max > length) length = max;
		}
	}

	// Ensure the length does not exceed UINT32_MAX and reset the stream
#ifdef _M_X64
	if(length > UINT32_MAX) throw Exception(E_UNEXPECTED);					// <-- TODO: better error
#endif
	reader->Reset();

	// Create a pagefile-backed memory mapped file view to hold the uncompressed binary image
	std::shared_ptr<MappedFile> mapping = 
		std::make_shared<MappedFile>(INVALID_HANDLE_VALUE, PAGE_EXECUTE_READWRITE | SEC_COMMIT, length);

	// Decompress the ELF image into the memory mapped file
	std::unique_ptr<MappedFileView> view(new MappedFileView(mapping, FILE_MAP_WRITE, 0, length));
	out = reader->Read(view->Pointer, static_cast<uint32_t>(length));
	if(out != length) throw Exception(E_ELF_TRUNCATED);
	view.release();

	return new ElfImageT<ehdr_t, phdr_t, shdr_t>(mapping);
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

	// TODO: Verify x86_64 machine
	// TODO: Verify object file type

	// Verify that the length of the header is the same size as the Elfxx_Ehdr struct
	// and that the header entries are at least as big as the known structures
	if(header->e_ehsize != sizeof(ehdr_t)) throw Exception(E_UNKNOWNELFVERSION);
	if((header->e_phentsize) && (header->e_phentsize < sizeof(phdr_t))) throw Exception(E_INVALIDELFPROGRAMTABLE);
	if((header->e_shentsize) && (header->e_shentsize < sizeof(shdr_t))) throw Exception(E_INVALIDELFSECTIONTABLE);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
