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
#include "ElfBinaryImage.h"

#include "Executable.h"
#include "Exception.h"
#include "Host.h"
#include "LinuxException.h"
#include "SystemInformation.h"

#pragma warning(push, 4)

// ElfBinaryImage::format_traits_t<x86>
//
template <> struct ElfBinaryImage::format_traits_t<Architecture::x86>
{
	using addr_t		= uapi::Elf32_Addr;
	using auxv_t		= uapi::Elf32_auxv_t;
	using elfheader_t	= uapi::Elf32_Ehdr;
	using pflags_t		= uapi::Elf32_Word;
	using progheader_t	= uapi::Elf32_Phdr;
	using sectheader_t	= uapi::Elf32_Shdr;

	static int const elfclass		= LINUX_ELFCLASS32;
	static int const machinetype	= LINUX_EM_386;
};

// ElfBinaryImage::format_traits_t<x86_64>
//
template <> struct ElfBinaryImage::format_traits_t<Architecture::x86_64>
{
	using addr_t		= uapi::Elf64_Addr;
	using auxv_t		= uapi::Elf64_auxv_t;
	using elfheader_t	= uapi::Elf64_Ehdr;
	using pflags_t		= uapi::Elf64_Word;
	using progheader_t	= uapi::Elf64_Phdr;
	using sectheader_t	= uapi::Elf64_Shdr;

	static int const elfclass		= LINUX_ELFCLASS64;
	static int const machinetype	= LINUX_EM_X86_64;
};

//-----------------------------------------------------------------------------
// Conversions
//-----------------------------------------------------------------------------

// uint32_t --> Host::MemoryProtection (local)
//
// Convert ELF protection flags into a Host::MemoryProtection bitmask
template<> inline Host::MemoryProtection convert<Host::MemoryProtection>(uint32_t flags)
{
	int result = LINUX_PROT_NONE;		// Default to PROT_NONE (0x00)

	// The ELF flags are the same as the Linux flags, but the bitmask values
	// are different.  Accumulate flags in the result based on source
	if(flags & LINUX_PF_R) result |= LINUX_PROT_READ;
	if(flags & LINUX_PF_W) result |= LINUX_PROT_WRITE;
	if(flags & LINUX_PF_X) result |= LINUX_PROT_EXEC;

	return Host::MemoryProtection(result);
}

//-----------------------------------------------------------------------------
// LoadElfBinary
//
// Loads an ELF binary image into a host instance and returns the metadata

template<Architecture architecture>
std::unique_ptr<BinaryImage> LoadElfBinary(Host* host, Executable const* executable)
{
	using elf = ElfBinaryImage::format_traits_t<architecture>;

	typename elf::elfheader_t		elfheader;			// ELF header structure
	ElfBinaryImage::metadata_t		metadata;			// Metadata from the load operation

	// Read the ELF header from the binary image, validate it and get the execution view size
	size_t read = executable->Handle->ReadAt(0, &elfheader, sizeof(typename elf::elfheader_t));
	ValidateElfHeader<architecture>(&elfheader, read);

	// Create a local heap buffer to hold a copy of the program header table
	size_t cbprogheaders = sizeof(typename elf::progheader_t) * elfheader.e_phnum;
	auto progheaders = std::make_unique<typename elf::progheader_t[]>(elfheader.e_phnum);

	// Read the program header table into the local heap buffer
	read = executable->Handle->ReadAt(elfheader.e_phoff, &progheaders[0], cbprogheaders);
	if(read != cbprogheaders) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFIMAGETRUNCATED } };

	// Determine the memory footprint of the binary image by scanning all PT_LOAD segments
	uintptr_t minvaddr = UINTPTR_MAX, maxvaddr = 0;
	for(size_t index = 0; index < elfheader.e_phnum; index++) {

		if((progheaders[index].p_type == LINUX_PT_LOAD) && (progheaders[index].p_memsz)) {

			// Calculate the minimum and maximum physical addresses of the segment and adjust accordingly
			minvaddr = std::min(uintptr_t{ progheaders[index].p_vaddr }, minvaddr);
			maxvaddr = std::max(uintptr_t{ progheaders[index].p_vaddr + progheaders[index].p_memsz }, maxvaddr);
		}
	}

	try {

		// ET_EXEC images must be reserved at the proper virtual address; ET_DYN images can go anywhere so reserve
		// them at the highest available virtual address to allow for as much heap space as possible in the process
		if(elfheader.e_type == LINUX_ET_EXEC) metadata.baseaddress = host->AllocateMemory(reinterpret_cast<void*>(minvaddr), maxvaddr - minvaddr, Host::MemoryProtection::None);
		else metadata.baseaddress = host->AllocateMemory(maxvaddr - minvaddr, Host::MemoryProtection::None);

	}

	catch(Exception& ex) { throw LinuxException{ LINUX_ENOMEM, Exception{ E_ELFRESERVEREGION, ex } }; }

	// ET_EXEC images are loaded at their virtual address, whereas ET_DYN images need a load delta to work with
	intptr_t vaddrdelta = (elfheader.e_type == LINUX_ET_EXEC) ? 0 : uintptr_t(metadata.baseaddress) - minvaddr;

	// Iterate over and load/process all of the program header sections
	for(size_t index = 0; index < elfheader.e_phnum; index++) {

		// PT_PHDR - if it falls within the boundaries of the loadable segments, provide the metadata values
		//
		if((progheaders[index].p_type == LINUX_PT_PHDR) && (progheaders[index].p_vaddr >= minvaddr) && ((progheaders[index].p_vaddr + progheaders[index].p_memsz) <= maxvaddr)) {

			metadata.progheaders = reinterpret_cast<void*>(uintptr_t(progheaders[index].p_vaddr) + vaddrdelta);
			metadata.numprogheaders = progheaders[index].p_memsz / elfheader.e_phentsize;
		}

		// PT_LOAD - load the segment into the host process and set the protection flags
		//
		else if((progheaders[index].p_type == LINUX_PT_LOAD) && (progheaders[index].p_memsz)) {

			// Get the base address of the loadable segment and set it as PAGE_READWRITE to load it
			uintptr_t segbase = progheaders[index].p_vaddr + vaddrdelta;
			try { host->ProtectMemory(reinterpret_cast<void*>(segbase), progheaders[index].p_memsz, LINUX_PROT_READ | LINUX_PROT_WRITE); }
			catch(Exception& ex) { throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFCOMMITSEGMENT, ex } }; }

			// Not all segments contain data that needs to be copied from the source image
			if(progheaders[index].p_filesz) {

				// Read the data from the image file into the target process address space at segbase
				try { read = host->WriteMemoryFrom(executable->Handle, progheaders[index].p_offset, reinterpret_cast<void*>(segbase), progheaders[index].p_filesz); }
				catch(Exception& ex) { throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFWRITESEGMENT, ex } }; }

				if(read != progheaders[index].p_filesz) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFIMAGETRUNCATED } };
			}

			// Attempt to apply the proper virtual memory protection flags to the segment now that it's been written
			try { host->ProtectMemory(reinterpret_cast<void*>(segbase), progheaders[index].p_memsz, convert<Host::MemoryProtection>(progheaders[index].p_flags)); }
			catch(Exception& ex) { throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFPROTECTSEGMENT, ex } }; }
		}

		// PT_INTERP - Segment contains an ASCII/UTF-8 interpreter binary path string
		//
		else if(progheaders[index].p_type == LINUX_PT_INTERP) {

			// Allocate a heap buffer to temporarily store the interpreter string
			auto interpreter = std::make_unique<char_t[]>(progheaders[index].p_filesz);
			read = executable->Handle->ReadAt(progheaders[index].p_offset, &interpreter[0], progheaders[index].p_filesz);
			if(read != progheaders[index].p_filesz) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFIMAGETRUNCATED } };

			// Convert the local heap buffer into an std::string
			metadata.interpreter = std::string(&interpreter[0], progheaders[index].p_filesz);
		}
	}

	// The initial program break address is the page just beyond the last allocated image segment
	metadata.breakaddress = align::up(reinterpret_cast<void*>(maxvaddr + vaddrdelta), SystemInformation::PageSize);

	// Calculate the address of the image entry point, if one has been specified in the header
	if(elfheader.e_entry) metadata.entrypoint = reinterpret_cast<void*>(elfheader.e_entry + vaddrdelta);

	// Construct and return a new ElfBinaryImage instance from the metadata
	return std::make_unique<ElfBinaryImage>(std::move(metadata));
}

//-----------------------------------------------------------------------------
// ValidateElfHeader
//
// Validates an ELF binary image header
//
// Arguments:
//
//	buffer		- Pointer to the start of the ELF header to validate
//	cb			- Size of the buffer in bytes

template <Architecture architecture>
void ValidateElfHeader(void const* buffer, size_t cb)
{
	using elf = ElfBinaryImage::format_traits_t<architecture>;

	if(buffer == nullptr) throw LinuxException{ LINUX_EFAULT };
	if(cb < sizeof(typename elf::elfheader_t)) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFTRUNCATEDHEADER } };

	typename elf::elfheader_t const* header = reinterpret_cast<typename elf::elfheader_t const*>(buffer);

	// Check the ELF header magic number
	if(memcmp(&header->e_ident[LINUX_EI_MAG0], LINUX_ELFMAG, LINUX_SELFMAG) != 0) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFINVALIDMAGIC } };

	// Verify the ELF class is appropriate for this architecture
	if(header->e_ident[LINUX_EI_CLASS] != elf::elfclass) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFINVALIDCLASS, header->e_ident[LINUX_EI_CLASS] } };

	// Verify the endianness and version of the ELF binary
	if(header->e_ident[LINUX_EI_DATA] != LINUX_ELFDATA2LSB) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFINVALIDENCODING, header->e_ident[LINUX_EI_DATA] } };
	if(header->e_ident[LINUX_EI_VERSION] != LINUX_EV_CURRENT) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFINVALIDVERSION, header->e_ident[LINUX_EI_VERSION] } };

	// Only ET_EXEC and ET_DYN images can currently be loaded
	if((header->e_type != LINUX_ET_EXEC) && (header->e_type != LINUX_ET_DYN)) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFINVALIDTYPE, header->e_type } };

	// The machine type must match the value defined for the elf_traits<>
	if(header->e_machine != elf::machinetype) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFINVALIDMACHINETYPE, header->e_machine } };

	// Verify that the version code matches the ELF headers used
	if(header->e_version != LINUX_EV_CURRENT) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFINVALIDVERSION, header->e_version } };

	// Verify that the length of the header is the same size as the Elfxx_Ehdr struct and that the
	// header entries are at least as big as the known structures
	if(header->e_ehsize != sizeof(typename elf::elfheader_t)) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFHEADERFORMAT } };
	if((header->e_phentsize) && (header->e_phentsize < sizeof(typename elf::progheader_t))) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFPROGHEADERFORMAT } };
	if((header->e_shentsize) && (header->e_shentsize < sizeof(typename elf::sectheader_t))) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFSECTHEADERFORMAT } };
}

//-----------------------------------------------------------------------------
// ElfBinaryImage Constructor (private)
//
// Arguments:
//
//	metadata		- Metadata created when loading the image

ElfBinaryImage::ElfBinaryImage(metadata_t&& metadata) : m_metadata(std::move(metadata))
{
}

//-----------------------------------------------------------------------------
// ElfBinaryImage::getBaseAddress
//
// Gets the base address of the loaded binary image

void const* ElfBinaryImage::getBaseAddress(void) const
{
	return m_metadata.baseaddress;
}

//-----------------------------------------------------------------------------
// ElfBinaryImage::getBreakAddress
//
// Gets the address of the program break

void const* ElfBinaryImage::getBreakAddress(void) const
{
	return m_metadata.breakaddress;
}

//-----------------------------------------------------------------------------
// ElfBinaryImage::getEntryPoint
//
// Gets the entry point of the loaded binary image

void const* ElfBinaryImage::getEntryPoint(void) const
{
	return m_metadata.entrypoint;
}

//-----------------------------------------------------------------------------
// ElfBinaryImage::getInterpreter
//
// Indicates the path to the program interpreter binary, if one is present

char_t const* ElfBinaryImage::getInterpreter(void) const
{
	return (m_metadata.interpreter.length()) ? m_metadata.interpreter.c_str() : nullptr;
}
	
//-----------------------------------------------------------------------------
// ElfBinaryImage::Load (static)
//
// Loads an ELF binary image into a virtual address space
//
// Arguments:
//
//	host			- Host instance to load the binary image into
//	executable		- Executable instance to be loaded

std::unique_ptr<BinaryImage> ElfBinaryImage::Load(Host* host, Executable const* executable)
{
	if(host == nullptr) throw LinuxException{ LINUX_EFAULT };
	if(executable == nullptr) throw LinuxException{ LINUX_EFAULT };

	_ASSERTE(executable->Format == BinaryFormat::ELF);
	if(executable->Format != BinaryFormat::ELF) throw LinuxException{ LINUX_ENOEXEC };

	// The actual Load implementation is specialized based on the ELF image architecture
	switch(executable->Architecture) {

		case Architecture::x86: return LoadElfBinary<Architecture::x86>(host, executable);
#ifdef _M_X64
		case Architecture::x86_64: return LoadElfBinary<Architecture::x86_64>(host, executable);
#endif
	}

	throw LinuxException{ LINUX_ENOEXEC };		// Unknown architecture
}

//-----------------------------------------------------------------------------
// ElfBinaryImage::getProgramHeadersAddress
//
// Pointer to program headers that were defined as part of the loaded image

void const* ElfBinaryImage::getProgramHeadersAddress(void) const
{
	return m_metadata.progheaders;
}

//-----------------------------------------------------------------------------
// ElfBinaryImage::getProgramHeaderCount
//
// Pointer to program headers that were defined as part of the loaded image

size_t ElfBinaryImage::getProgramHeaderCount(void) const
{
	return m_metadata.numprogheaders;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
