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
#include "ElfBinary64.h"				// Include ELFBinary64 declarations

#include "BufferStreamReader.h"			// Include BufferStreamReader declarations
#include "ElfSegment64.h"				// Include ElfSegment64 declarations
#include "Exception.h"					// Include Exception declarations
#include "MappedFile.h"					// Include MappedFile declarations
#include "MappedFileView.h"				// Include MappedFileView declarations

#include <algorithm>					// Include STL algorithm declarations
#include <vector>						// Include STL vector<> declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// ElfBinary64 Constructor (private)
//
// Arguments:
//
//	view			- Mapped view of the ELF binary file image

ElfBinary64::ElfBinary64(std::unique_ptr<MappedFileView>& view)
{
	uint8_t*			base;			// Base pointer of the view

	// Cast the base image pointer into something more easily manipulated
	base = reinterpret_cast<uint8_t*>(view->Pointer);

	memcpy(&m_header, base, sizeof(Elf64_Ehdr));

	for(int index = 0; index < m_header.e_phnum; index++) {
	}

	//// Load all of the program headers into a collection
	//std::vector<Elf64_Phdr> headers;

	//for(int index = 0; index < m_header.e_phnum; index++) {

	//	// Read in the next program header table entry
	//	out = reader->Read(&buffer[0], m_header.e_phentsize);
	//	if(out != m_header.e_phentsize) throw Exception(E_INVALIDELFPROGRAMTABLE);

	//	// Cast a pointer to the ELF program header table structure and insert it
	//	Elf64_Phdr* pheader = reinterpret_cast<Elf64_Phdr*>(&buffer[0]);
	//	headers.push_back(*pheader);
	//}

	//// apply sort
	////std::sort(headers.begin(), headers.end(), [](Elf64_Phdr const & a, Elf64_Phdr const &b){return a.p_offset < b.p_offset;});

	//// Now that all the headers have been loaded, attempt to load all of the segments
	//for(std::vector<Elf64_Phdr>::const_iterator iterator = headers.begin(); iterator != headers.end(); iterator++) {

	//	// TEMP: only do PT_LOAD
	//	if(iterator->p_type == PT_LOAD) {
	//	// Ensure that the stream hasn't been seeked beyond the required location
	//	if(iterator->p_offset < reader->Position) throw Exception(E_ELFSEGMENTFILEORDER);
	//	if(iterator->p_offset > UINT32_MAX) throw Exception(E_UNEXPECTED);		// <--- TODO: better error
	//	reader->Seek(static_cast<uint32_t>(iterator->p_offset));

	//	std::unique_ptr<ElfSegment> segment(new ElfSegment64(&(*iterator), reader));
	//	}
	//}

}

//-----------------------------------------------------------------------------
// ElfBinary::Load (static)
//
// Parses and loads the specified ELF image into virtual memory
//
// Arguments:
//
//	reader		- Pointer to a StreamReader used to access the raw image

ElfBinary64* ElfBinary64::Load(std::unique_ptr<StreamReader>& reader)
{
	Elf64_Ehdr			header;					// ELF header structure
	uint32_t			out;					// Bytes read from the input stream
	size_t				length;					// Length of the uncompressed image

	// Read the ELF header from the beginning of the stream
	out = reader->Read(&header, sizeof(Elf64_Ehdr));
	if(out != sizeof(Elf64_Ehdr)) throw Exception(E_TRUNCATEDELFHEADER);

	// Check the ELF header magic number
	if(memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0) throw Exception(E_INVALIDELFMAGIC);

	// Verify that a 64-bit ELF binary is being processed by the class
	if(header.e_ident[EI_CLASS] != ELFCLASS64) throw Exception(E_UNEXPECTEDELFCLASS, header.e_ident[EI_CLASS]);

	// Verify the endianness and version of the ELF binary
	if(header.e_ident[EI_DATA] != ELFDATA2LSB) throw Exception(E_UNEXPECTEDELFENCODING);
	if(header.e_ident[EI_VERSION] != EV_CURRENT) throw Exception(E_UNKNOWNELFVERSION);

	// TODO: Verify x86_64 machine
	// TODO: Verify object file type

	// Verify that the length of the header is the same size as the Elfxx_Ehdr struct
	// and that the header entries are at least as big as the known structures
	if(header.e_ehsize != sizeof(Elf64_Ehdr)) throw Exception(E_UNKNOWNELFVERSION);
	if((header.e_phentsize) && (header.e_phentsize < sizeof(Elf64_Phdr))) throw Exception(E_INVALIDELFPROGRAMTABLE);
	if((header.e_shentsize) && (header.e_shentsize < sizeof(Elf64_Shdr))) throw Exception(E_INVALIDELFSECTIONTABLE);

	// Determine the length of the uncompressed image.  If there is a section header table, use
	// that since it's at the end of the file, otherwise the program header table has to be 
	// examined to calculate the length of the available segments

	length = sizeof(Elf64_Ehdr);
	if(header.e_shnum) length = header.e_shoff + (header.e_shnum * header.e_shentsize);
	else if(header.e_phnum) {

		// Skip to the beginning of the program header table in the input stream
		if(header.e_phoff > UINT32_MAX) throw Exception(E_UNEXPECTED);	// <-- TODO: better error
		reader->Seek(static_cast<uint32_t>(header.e_phoff));

		std::unique_ptr<uint8_t[]> buffer(new uint8_t[header.e_phentsize]);
		for(int index = 0; index < header.e_phnum; index++) {

			// Read in the next program header table entry
			out = reader->Read(&buffer[0], header.e_phentsize);
			if(out != header.e_phentsize) throw Exception(E_INVALIDELFPROGRAMTABLE);

			// Cast a pointer to the ELF program header table structure
			Elf64_Phdr* phdr = reinterpret_cast<Elf64_Phdr*>(&buffer[0]);
			
			// Check this segment offset + length against the current known length
			size_t max = phdr->p_offset + phdr->p_filesz;
			if(max > length) length = max;
		}
	}

	// Ensure the length does not exceed UINT32_MAX and reset the stream
	if(length > UINT32_MAX) throw Exception(E_UNEXPECTED);					// <-- TODO: better error
	reader->Reset();

	// Create a pagefile-backed memory mapped file view to hold the uncompressed binary image
	std::shared_ptr<MappedFile> mapping = std::make_shared<MappedFile>(INVALID_HANDLE_VALUE, PAGE_READWRITE | SEC_COMMIT, length);
	std::unique_ptr<MappedFileView> view(new MappedFileView(mapping, FILE_MAP_WRITE, 0, length));

	// Decompress the ELF image into the mapped file area
	out = reader->Read(view->Pointer, static_cast<uint32_t>(length));
	if(out != length) throw Exception(E_ABORT);

	return new ElfBinary64(view);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
