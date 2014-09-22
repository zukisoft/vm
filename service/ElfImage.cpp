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

#include "stdafx.h"
#include "ElfImage.h"

#pragma warning(push, 4)

ElfImage::load_binary_func ElfImage::LoadBinary32 = 
ElfImage::LoadBinary<LINUX_ELFCLASS32, uapi::Elf32_Ehdr, uapi::Elf32_Phdr, uapi::Elf32_Shdr>;

#ifdef _M_X64
ElfImage::load_binary_func ElfImage::LoadBinary64 = 
ElfImage::LoadBinary<LINUX_ELFCLASS64, uapi::Elf64_Ehdr, uapi::Elf64_Phdr, uapi::Elf64_Shdr>;
#endif

//-----------------------------------------------------------------------------
// InProcessRead
//
// Reads data from a StreamReader instance directly into a memory buffer
//
// Arguments:
//
//	reader		- StreamReader instance
//	offset		- Offset from the beginning of the stream to read from
//	destination	- Destination buffer
//	count		- Number of bytes to be read

inline size_t InProcessRead(StreamReader& reader, size_t offset, void* destination, size_t count)
{
	// Seek the stream forward as necessary and just read the requested count
	if(reader.Position != offset) reader.Seek(offset);
	return reader.Read(destination, count);
}

//-----------------------------------------------------------------------------
// OutOfProcessRead
//
// Reads data from a StreamReader instance into another process using an
// intermediate heap buffer
//
// Arguments:
//
//	reader		- StreamReader instance
//	process		- Destination process handle, or INVALID_HANDLE_VALUE
//	offset		- Offset from the beginning of the stream to read from
//	destination	- Destination buffer
//	count		- Number of bytes to be read

inline size_t OutOfProcessRead(StreamReader& reader, HANDLE process, size_t offset, void* destination, size_t count)
{
	// If the process handle is not valid, this is actually an in-process read
	if(process == INVALID_HANDLE_VALUE) return InProcessRead(reader, offset, destination, count);

	// Allocate a heap buffer large enough to accomodate the data transfer
	HeapBuffer<uint8_t> intermediate(count);

	// Read up to count bytes from the stream into the intermediate buffer
	if(reader.Position != offset) reader.Seek(offset);
	size_t read = reader.Read(&intermediate, count);

	// Write the amount of data read into the external process
	SIZE_T written;
	if(!WriteProcessMemory(process, destination, &intermediate, read, &written)) throw Win32Exception();

	return written;
}

//-----------------------------------------------------------------------------
// ElfImage::FlagsToProtection (static, private)
//
// Converts an ELF program header p_flags into VirtualAlloc[Ex] protection flags
//
// Arguments:
//
//	flags		- ELF program header p_flags value

DWORD ElfImage::FlagsToProtection(uint32_t flags)
{
	switch(flags) {

		case LINUX_PF_X : return PAGE_EXECUTE;
		case LINUX_PF_W : return PAGE_READWRITE;
		case LINUX_PF_R : return PAGE_READONLY;
		case LINUX_PF_X | LINUX_PF_W : return PAGE_EXECUTE_READWRITE;
		case LINUX_PF_X | LINUX_PF_R : return PAGE_EXECUTE_READ;
		case LINUX_PF_W | LINUX_PF_R : return PAGE_READWRITE;
		case LINUX_PF_X | LINUX_PF_W | LINUX_PF_R :	return PAGE_EXECUTE_READWRITE;
	}

	return PAGE_NOACCESS;
}

//-----------------------------------------------------------------------------

std::unique_ptr<ElfImage> ElfImage::Load(ElfImage::Type type, HANDLE process, const FileSystem::HandlePtr& handle)
{
	HandleStreamReader reader(handle);
	return Load(type, process, reader);
}

std::unique_ptr<ElfImage> ElfImage::Load(ElfImage::Type type, HANDLE process, StreamReader& reader)
{
#ifdef _M_X64
	// 64 bit: Type has to be ELF32 or ELF64
	if(type == Type::None) throw Exception(E_NOTELFIMAGE);
#else
	// 32 bit: Type has to be ELF32
	if(type != Type::i386) throw Exception(E_NOTELFIMAGE);
#endif

#ifdef _M_X64
	// 64-bit builds can load either 32 or 64 bit binaries into the host process
	load_binary_func loader = (type == Type::i386) ? LoadBinary32 : LoadBinary64;
#else
	// 32-bit builds can only load 32 bit binaries into the host process
	load_binary_func loader = LoadBinary32;
#endif

	return std::make_unique<ElfImage>(loader(reader, process));
}

//-----------------------------------------------------------------------------
// ElfImage::LoadBinary (static, private)
//
// Loads an ELF binary image into virtual memory
//
// Template Arguments:
//
//	elfclass	- Expected ELF binary class value
//	ehdr_t		- ELF header structure type
//	phdr_t		- ELF program header structure type
//	shdr_t		- ELF section header structure type
//
// Arguments:
//
//	reader				- StreamReader instance for the binary image data
//	process				- Handle to the process in which to load the image

template <int elfclass, class ehdr_t, class phdr_t, class shdr_t>
ElfImage::Metadata ElfImage::LoadBinary(StreamReader& reader, HANDLE process)
{
	Metadata						metadata;		// Metadata to return about the loaded image
	ehdr_t							elfheader;		// ELF binary image header structure
	std::unique_ptr<MemoryRegion>	region;			// Allocated virtual memory region

	// Acquire a copy of the ELF header from the binary file and validate it
	size_t read = InProcessRead(reader, 0, &elfheader, sizeof(ehdr_t));
	if(read != sizeof(ehdr_t)) throw Exception(E_TRUNCATEDELFHEADER);
	ValidateHeader<elfclass, ehdr_t, phdr_t, shdr_t>(&elfheader);

	// Read all of the program headers from the binary image file into a heap buffer
	HeapBuffer<phdr_t> progheaders(elfheader.e_phnum);
	read = InProcessRead(reader, elfheader.e_phoff, &progheaders, progheaders.Size);
	if(read != progheaders.Size) throw Exception(E_ELFIMAGETRUNCATED);

	// PROGRAM HEADERS PASS ONE

	// Make an initial pass over the program headers to determine the memory footprint
	uintptr_t minvaddr = UINTPTR_MAX, maxvaddr = 0;
	for(size_t index = 0; index < progheaders.Count; index++) {

		// Pull out a reference to the current program header structure
		const phdr_t& progheader = progheaders[index];

		// PT_LOAD - Loadable segment
		if((progheader.p_type == LINUX_PT_LOAD) && (progheader.p_memsz)) {

			// Calculate the minimum and maximum physical addresses of the segment
			// and adjust the overall minimum and maximums accordingly
			minvaddr = min(uintptr_t(progheader.p_vaddr), minvaddr);
			maxvaddr = max(uintptr_t(progheader.p_vaddr + progheader.p_memsz), maxvaddr);
		}

		// PT_GNU_STACK - GNU executable stack segment
		else if(progheader.p_type == LINUX_PT_GNU_STACK) {

			// If the segment flags are executable, that's not currently supported
			if(progheader.p_flags & LINUX_PF_X) throw Exception(E_EXECUTABLESTACKFLAG);
		}
	}

	// MEMORY ALLOCATION

	try {

		// ET_EXEC images must be reserved at the proper virtual address; ET_DYN images can go anywhere
		// so reserve them at the highest available virtual address to try and avoid conflicts
		if(elfheader.e_type == LINUX_ET_EXEC) region = MemoryRegion::Reserve(process, maxvaddr - minvaddr, reinterpret_cast<void*>(minvaddr));
		else region = MemoryRegion::Reserve(process, maxvaddr - minvaddr, MEM_TOP_DOWN);

	} catch(Exception& ex) { throw Exception(E_RESERVEIMAGEREGION, ex); }

	// ET_EXEC images are loaded at their virtual address, whereas ET_DYN images need a load delta to work with
	intptr_t vaddrdelta = (elfheader.e_type == LINUX_ET_EXEC) ? 0 : uintptr_t(region->Pointer) - minvaddr;

	// PROGRAM HEADERS PASS TWO

	// Second pass over the program headers to load, commit and protect the program segments
	for(size_t index = 0; index < progheaders.Count; index++) {

		// Pull out a reference to the current program header structure
		const phdr_t& progheader = progheaders[index];

		// PT_PHDR - if it falls within the boundaries of the loadable segments, set this so that it can be passed into
		// the hosted process as an auxiliary vector
		if((progheader.p_type == LINUX_PT_PHDR) && (progheader.p_vaddr >= minvaddr) && ((progheader.p_vaddr + progheader.p_memsz) <= maxvaddr)) {

			metadata.ProgramHeaders = reinterpret_cast<void*>(uintptr_t(progheader.p_vaddr) + vaddrdelta);
			metadata.NumProgramHeaders = progheader.p_memsz / elfheader.e_phentsize;
		}

		// PT_LOAD - only load segments that have a non-zero memory footprint defined
		else if((progheader.p_type == LINUX_PT_LOAD) && (progheader.p_memsz)) {

			// Get the base address of the loadable segment and commit the virtual memory
			uintptr_t segbase = progheader.p_vaddr + vaddrdelta;
			try { region->Commit(reinterpret_cast<void*>(segbase), progheader.p_memsz, PAGE_READWRITE); }
			catch(Exception& ex) { throw Exception(E_COMMITIMAGESEGMENT, ex); }

			// Not all segments contain data that needs to be copied from the source image
			if(progheader.p_filesz) {

				// Read the data from the input stream into the target process address space at segbase
				read = OutOfProcessRead(reader, process, progheader.p_offset, reinterpret_cast<void*>(segbase), progheader.p_filesz);
				if(read != progheader.p_filesz) throw Exception(E_ELFIMAGETRUNCATED);
			}

			// Memory that was not loaded from the ELF image must be initialized to zero
			// TODO - should not be necessary, VirtualAlloc does this for us
			// memset(reinterpret_cast<void*>(segbase + progheader->p_filesz), 0, progheader->p_memsz - progheader->p_filesz);

			// Attempt to apply the proper virtual memory protection flags to the segment
			try { region->Protect(reinterpret_cast<void*>(segbase), progheader.p_memsz, FlagsToProtection(progheader.p_flags)); }
			catch(Exception& ex) { throw Exception(E_PROTECTIMAGESEGMENT, ex); }
		}

		// PT_INTERP
		//
		// Segment contains an ANSI/UTF-8 interpreter string
		else if(progheader.p_type == LINUX_PT_INTERP) {

			// Allocate a heap buffer to temporarily store the interpreter string
			HeapBuffer<char_t> interpreter(progheader.p_filesz);
			read = InProcessRead(reader, progheader.p_offset, &interpreter, interpreter.Size);
			if(read != progheader.p_filesz) throw Exception(E_ELFIMAGETRUNCATED);

			// Ensure that the string is NULL terminated and convert it into an std::tstring
			if(interpreter[interpreter.Count - 1] != 0) throw Exception(E_INVALIDINTERPRETER);
			metadata.Interpreter = std::to_tstring(static_cast<char_t*>(interpreter));
		}
	}

	// COMPLETION

	// Base address of the image is the original minimum virtual address, adjusted for load delta
	metadata.BaseAddress = reinterpret_cast<void*>(minvaddr + vaddrdelta);

	// Calculate the address of the image entry point, if one has been specified in the header
	metadata.EntryPoint = (elfheader.e_entry) ? reinterpret_cast<void*>(elfheader.e_entry + vaddrdelta) : nullptr;

	region->Detach();		// Successful; detach the region from auto release
	return metadata;		// Return the metadata about the loaded binary image
}

//-----------------------------------------------------------------------------
// ElfImage::ValidateHeader (static, private)
//
// Validates an ELF binary header; this is a helper function to LoadBinary
//
// Template Arguments:
//
//	elfclass	- Expected ELF binary class value
//	ehdr_t		- ELF header structure type
//	phdr_t		- ELF program header structure type
//	shdr_t		- ELF section header structure type
//
// Arguments:
//
//	elfheader	- Pointer to the ELF header loaded by LoadBinary

template <int elfclass, class ehdr_t, class phdr_t, class shdr_t>
void ElfImage::ValidateHeader(const ehdr_t* elfheader)
{
	if(!elfheader) throw Exception(E_POINTER);

	// Check the ELF header magic number
	if(memcmp(&elfheader->e_ident[LINUX_EI_MAG0], LINUX_ELFMAG, LINUX_SELFMAG) != 0) throw Exception(E_INVALIDELFMAGIC);

	// Verify the ELF class is appropriate for this image loader instance
	if(elfheader->e_ident[LINUX_EI_CLASS] != elfclass) throw Exception(E_INVALIDELFCLASS, elfheader->e_ident[LINUX_EI_CLASS]);

	// Verify the endianness and version of the ELF binary
	if(elfheader->e_ident[LINUX_EI_DATA] != LINUX_ELFDATA2LSB) throw Exception(E_INVALIDELFENCODING, elfheader->e_ident[LINUX_EI_DATA]);
	if(elfheader->e_ident[LINUX_EI_VERSION] != LINUX_EV_CURRENT) throw Exception(E_INVALIDELFVERSION, elfheader->e_ident[LINUX_EI_VERSION]);

	// Only ET_EXEC and ET_DYN images can currently be loaded
	if((elfheader->e_type != LINUX_ET_EXEC) && (elfheader->e_type != LINUX_ET_DYN)) throw Exception(E_INVALIDELFTYPE, elfheader->e_type);

	// The machine type must either be x86 (32 bit) or x86-64 (64 bit)
	const int elfmachinetype = (elfclass == LINUX_ELFCLASS32) ? LINUX_EM_386 : LINUX_EM_X86_64;
	if(elfheader->e_machine != elfmachinetype) throw Exception(E_INVALIDELFMACHINETYPE, elfheader->e_machine);

	// Verify that the version code matches the ELF headers used
	if(elfheader->e_version != LINUX_EV_CURRENT) throw Exception(E_INVALIDELFVERSION, elfheader->e_version);

	// Verify that the length of the header is the same size as the Elfxx_Ehdr struct and that the
	// header entries are at least as big as the known structures
	if(elfheader->e_ehsize != sizeof(ehdr_t)) throw Exception(E_ELFHEADERFORMAT);
	if((elfheader->e_phentsize) && (elfheader->e_phentsize < sizeof(phdr_t))) throw Exception(E_ELFPROGHEADERFORMAT);
	if((elfheader->e_shentsize) && (elfheader->e_shentsize < sizeof(shdr_t))) throw Exception(E_ELFSECTHEADERFORMAT);
}

//-----------------------------------------------------------------------------
// ElfImage::HandleStreamReader::Read
//
// Reads data from a FileSystem::Handle instance
//
// Arguments:
//
//	buffer		- Destination buffer
//	length		- Number of bytes to be read from the file stream

size_t ElfImage::HandleStreamReader::Read(void* buffer, size_t length)
{
	// Just ask the Handle instance to read the data into the destination
	return m_handle->Read(buffer, length);
}

//-----------------------------------------------------------------------------
// ElfImage::HandleStreamReader::Seek
//
// Seeks the stream to the specified position
//
// Arguments:
//
//	position		- Position to set the stream pointer to

void ElfImage::HandleStreamReader::Seek(size_t position)
{
	// Handles use uapi::loff_t, which is a signed 64-bit value
	if(position > INT64_MAX) throw Exception(E_INVALIDARG);

	// StreamReaders are supposed to be forward-only; even though the Handle
	// can technically support this, follow the interface contract
	if(position < m_position) throw Exception(E_INVALIDARG);

	// Attempt to set the file pointer to the specified position
	uapi::loff_t offset = static_cast<uapi::loff_t>(position);
	if(m_handle->Seek(offset, LINUX_SEEK_SET) != offset) throw Exception(E_INVALIDARG);
}


//-----------------------------------------------------------------------------

#pragma warning(pop)
