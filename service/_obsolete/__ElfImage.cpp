//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
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

//-----------------------------------------------------------------------------
// ::InProcessRead
//
// Reads data from a Handle instance directly into a memory buffer
//
// Arguments:
//
//	handle		- FileSystem object handle instance
//	offset		- Offset from the beginning of the stream to read from
//	destination	- Destination buffer
//	count		- Number of bytes to be read

inline size_t InProcessRead(const FileSystem::HandlePtr& handle, size_t offset, void* destination, size_t count)
{
	// Set the file pointer to the specified position and read the data; assume truncation on error reading
	if(static_cast<size_t>(handle->Seek(offset, LINUX_SEEK_SET)) != offset) throw Exception(E_ELFIMAGETRUNCATED);
	return handle->Read(destination, count);
}

//-----------------------------------------------------------------------------
// ::OutOfProcessRead
//
// Reads data from a StreamReader instance into another process using an
// intermediate heap buffer
//
// Arguments:
//
//	handle		- FileSystem object handle instance
//	process		- Destination process handle, or INVALID_HANDLE_VALUE
//	offset		- Offset from the beginning of the stream to read from
//	destination	- Destination buffer
//	count		- Number of bytes to be read

inline size_t OutOfProcessRead(const FileSystem::HandlePtr& handle, const std::unique_ptr<Host>& host, size_t offset, void* destination, size_t count)
{
	uintptr_t			dest = uintptr_t(destination);		// Easier pointer math as uintptr_t
	size_t				total = 0;							// Total bytes written
	
	// This function seems to perform the best with allocation granularity chunks of data (64KiB)
	HeapBuffer<uint8_t> buffer(SystemInformation::AllocationGranularity);

	// Seek the file handle to the specified offset value, if unable to assume that it's truncated
	if(static_cast<size_t>(handle->Seek(offset, LINUX_SEEK_SET)) != offset) throw Exception(E_ELFIMAGETRUNCATED);

	while(count) {

		// Read the next chunk of memory into the heap buffer, break early if there is no more
		size_t read = handle->Read(buffer, min(count, buffer.Size));
		if(read == 0) break;

		// Write the data into the target native operating system process
		total += host->WriteMemory(reinterpret_cast<void*>(dest + total), buffer, read);

		count -= read;					// Decrement bytes left to be read
	};

	return total;						// Return total bytes written
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
// ElfImage::Load (Architecture::x86)
//
// Loads a 32-bit ELF image into virtual memory
//
// Arguments:
//
//	handle		- FileSystem object handle instance for the binary image
//	host		- Native operating system process to load the ELF image into

template <> std::unique_ptr<ElfImage> ElfImage::Load<Architecture::x86>(const FileSystem::HandlePtr& handle, const std::unique_ptr<Host>& host)
{
	// Invoke the 32-bit version of LoadBinary() to parse out and load the ELF image
	return LoadBinary<Architecture::x86>(handle, host);
}

//-----------------------------------------------------------------------------
// ElfImage::Load (Architecture::x86_64)
//
// Loads a 64-bit ELF image into virtual memory
//
// Arguments:
//
//	handle		- FileSystem object handle instance for the binary image
//	host		- Native operating system process to load the ELF image into

#ifdef _M_X64
template <> std::unique_ptr<ElfImage> ElfImage::Load<Architecture::x86_64>(const FileSystem::HandlePtr& handle, const std::unique_ptr<Host>& host)
{
	// Invoke the 64-bit version of LoadBinary() to parse out and load the ELF image
	return LoadBinary<Architecture::x86_64>(handle, host);
}
#endif

//-----------------------------------------------------------------------------
// ElfImage::LoadBinary (static, private)
//
// Loads an ELF binary image into virtual memory
//
// Arguments:
//
//	handle		- FileSystem object handle instance for the binary image
//	host		- Native operating system process to load the ELF image into

template <Architecture architecture>
std::unique_ptr<ElfImage> ElfImage::LoadBinary(const FileSystem::HandlePtr& handle, const std::unique_ptr<Host>& host)
{
	using elf = elf_traits<architecture>;

	Metadata						metadata;		// Metadata to return about the loaded image
	typename elf::elfheader_t		elfheader;		// ELF binary image header structure

	// Acquire a copy of the ELF header from the binary file and validate it
	size_t read = InProcessRead(handle, 0, &elfheader, sizeof(typename elf::elfheader_t));
	if(read != sizeof(typename elf::elfheader_t)) throw Exception(E_ELFTRUNCATEDHEADER);
	ValidateHeader<architecture>(&elfheader);

	// Read all of the program headers from the binary image file into a heap buffer
	HeapBuffer<typename elf::progheader_t> progheaders(elfheader.e_phnum);
	read = InProcessRead(handle, elfheader.e_phoff, &progheaders, progheaders.Size);
	if(read != progheaders.Size) throw Exception(E_ELFIMAGETRUNCATED);

	// PROGRAM HEADERS PASS ONE - GET MEMORY FOOTPRINT AND CHECK INVARIANTS
	uintptr_t minvaddr = UINTPTR_MAX, maxvaddr = 0;
	for(size_t index = 0; index < progheaders.Count; index++) {

		// Pull out a reference to the current program header structure
		const typename elf::progheader_t& progheader = progheaders[index];

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
			if(progheader.p_flags & LINUX_PF_X) throw Exception(E_ELFEXECUTABLESTACK);
		}
	}

	// MEMORY ALLOCATION
	try {

		// ET_EXEC images must be reserved at the proper virtual address; ET_DYN images can go anywhere so
		// reserve them at the highest available virtual address to allow for as much heap space as possible.
		// Note that the section length is aligned to the allocation granularity of the system to prevent holes
		if(elfheader.e_type == LINUX_ET_EXEC) metadata.BaseAddress = host->AllocateMemory(reinterpret_cast<void*>(minvaddr), maxvaddr - minvaddr, PAGE_NOACCESS);
		else metadata.BaseAddress = host->AllocateMemory(maxvaddr - minvaddr, PAGE_NOACCESS);

	} catch(Exception& ex) { throw Exception(E_ELFRESERVEREGION, ex); }

	// ET_EXEC images are loaded at their virtual address, whereas ET_DYN images need a load delta to work with
	intptr_t vaddrdelta = (elfheader.e_type == LINUX_ET_EXEC) ? 0 : uintptr_t(metadata.BaseAddress) - minvaddr;

	// PROGRAM HEADERS PASS TWO - LOAD, COMMIT AND PROTECT SEGMENTS
	for(size_t index = 0; index < progheaders.Count; index++) {

		// Pull out a reference to the current program header structure
		const typename elf::progheader_t& progheader = progheaders[index];

		// PT_PHDR - if it falls within the boundaries of the loadable segments, set this so that it can be passed into
		// the hosted process as an auxiliary vector
		if((progheader.p_type == LINUX_PT_PHDR) && (progheader.p_vaddr >= minvaddr) && ((progheader.p_vaddr + progheader.p_memsz) <= maxvaddr)) {

			metadata.ProgramHeaders = reinterpret_cast<void*>(uintptr_t(progheader.p_vaddr) + vaddrdelta);
			metadata.NumProgramHeaders = progheader.p_memsz / elfheader.e_phentsize;
		}

		// PT_LOAD - only load segments that have a non-zero memory footprint defined
		else if((progheader.p_type == LINUX_PT_LOAD) && (progheader.p_memsz)) {

			// Get the base address of the loadable segment and set it as PAGE_READWRITE to load it
			uintptr_t segbase = progheader.p_vaddr + vaddrdelta;
			try { host->ProtectMemory(reinterpret_cast<void*>(segbase), progheader.p_memsz, PAGE_READWRITE); }
			catch(Exception& ex) { throw Exception(E_ELFCOMMITSEGMENT, ex); }

			// Not all segments contain data that needs to be copied from the source image
			if(progheader.p_filesz) {

				// Read the data from the input stream into the target process address space at segbase
				try { read = OutOfProcessRead(handle, host, progheader.p_offset, reinterpret_cast<void*>(segbase), progheader.p_filesz); }
				catch(Exception& ex) { throw Exception(E_ELFWRITESEGMENT, ex); }

				if(read != progheader.p_filesz) throw Exception(E_ELFIMAGETRUNCATED);
			}

			// Attempt to apply the proper virtual memory protection flags to the segment now that it's been written
			try { host->ProtectMemory(reinterpret_cast<void*>(segbase), progheader.p_memsz, FlagsToProtection(progheader.p_flags)); }
			catch(Exception& ex) { throw Exception(E_ELFPROTECTSEGMENT, ex); }
		}

		// PT_INTERP
		//
		// Segment contains an ANSI/UTF-8 interpreter string
		else if(progheader.p_type == LINUX_PT_INTERP) {

			// Allocate a heap buffer to temporarily store the interpreter string
			HeapBuffer<char_t> interpreter(progheader.p_filesz);
			read = InProcessRead(handle, progheader.p_offset, &interpreter, interpreter.Size);
			if(read != progheader.p_filesz) throw Exception(E_ELFIMAGETRUNCATED);

			// Ensure that the string is NULL terminated and convert it into an std::tstring
			if(interpreter[interpreter.Count - 1] != 0) throw Exception(E_ELFINVALIDINTERPRETER);
			metadata.Interpreter = interpreter;
		}
	}

	// The initial program break address is the page just beyond the committed image
	metadata.ProgramBreak = align::up(reinterpret_cast<void*>(maxvaddr + vaddrdelta), SystemInformation::PageSize);

	// Calculate the address of the image entry point, if one has been specified in the header
	metadata.EntryPoint = (elfheader.e_entry) ? reinterpret_cast<void*>(elfheader.e_entry + vaddrdelta) : nullptr;

	// Construct and return a new ElfImage instance from the section and metadata
	return std::make_unique<ElfImage>(std::move(metadata));
}

//-----------------------------------------------------------------------------
// ElfImage::ValidateHeader (static, private)
//
// Validates an ELF binary header; this is a helper function to LoadBinary
//
// Arguments:
//
//	elfheader	- Pointer to the ELF header loaded by LoadBinary

template <Architecture architecture>
void ElfImage::ValidateHeader(const typename elf_traits<architecture>::elfheader_t* elfheader)
{
	using elf = elf_traits<architecture>;

	if(!elfheader) throw Exception(E_ARGUMENTNULL, "elfheader");

	// Check the ELF header magic number
	if(memcmp(&elfheader->e_ident[LINUX_EI_MAG0], LINUX_ELFMAG, LINUX_SELFMAG) != 0) throw Exception(E_ELFINVALIDMAGIC);

	// Verify the ELF class is appropriate for this image loader instance
	if(elfheader->e_ident[LINUX_EI_CLASS] != elf::elfclass) throw Exception(E_ELFINVALIDCLASS, elfheader->e_ident[LINUX_EI_CLASS]);

	// Verify the endianness and version of the ELF binary
	if(elfheader->e_ident[LINUX_EI_DATA] != LINUX_ELFDATA2LSB) throw Exception(E_ELFINVALIDENCODING, elfheader->e_ident[LINUX_EI_DATA]);
	if(elfheader->e_ident[LINUX_EI_VERSION] != LINUX_EV_CURRENT) throw Exception(E_ELFINVALIDVERSION, elfheader->e_ident[LINUX_EI_VERSION]);

	// Only ET_EXEC and ET_DYN images can currently be loaded
	if((elfheader->e_type != LINUX_ET_EXEC) && (elfheader->e_type != LINUX_ET_DYN)) throw Exception(E_ELFINVALIDTYPE, elfheader->e_type);

	// The machine type must match the value defined for the elf_traits<>
	if(elfheader->e_machine != elf::machinetype) throw Exception(E_ELFINVALIDMACHINETYPE, elfheader->e_machine);

	// Verify that the version code matches the ELF headers used
	if(elfheader->e_version != LINUX_EV_CURRENT) throw Exception(E_ELFINVALIDVERSION, elfheader->e_version);

	// Verify that the length of the header is the same size as the Elfxx_Ehdr struct and that the
	// header entries are at least as big as the known structures
	if(elfheader->e_ehsize != sizeof(typename elf::elfheader_t)) throw Exception(E_ELFHEADERFORMAT);
	if((elfheader->e_phentsize) && (elfheader->e_phentsize < sizeof(typename elf::progheader_t))) throw Exception(E_ELFPROGHEADERFORMAT);
	if((elfheader->e_shentsize) && (elfheader->e_shentsize < sizeof(typename elf::sectheader_t))) throw Exception(E_ELFSECTHEADERFORMAT);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
