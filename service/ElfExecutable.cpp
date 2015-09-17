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
#include "ElfExecutable.h"

#include "Host.h"
#include "LinuxException.h"
#include "SystemInformation.h"

#pragma warning(push, 4)

// ElfExecutable::format_traits_t<x86>
//
template <> struct ElfExecutable::format_traits_t<Architecture::x86>
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

// ElfExecutable::format_traits_t<x86_64>
//
template <> struct ElfExecutable::format_traits_t<Architecture::x86_64>
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

// todo: this is inconsistenly applied below, most places use the LINUX_PROT values

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

////-----------------------------------------------------------------------------
//// CreateElfStack<Architecture>
////
//// Creates an ELF executable stack in a host instance and returns the metadata
////
//// Arguments:
////
////	host			- Host instance in which to create the stack
////	length			- Length of the stack image to create
////	executable		- ElfExecutable instance for which to crate the stack
////	layout			- Main executable layout from LoadImage
////	interpreter		- Optional interpreter layout from LoadImage
//
//template<enum class Architecture architecture>
//std::unique_ptr<Executable::StackLayout> CreateElfStack(Host* host, size_t length, ElfExecutable const* executable, 
//	Executable::ImageLayout const* layout)
//{
//	using elf = ElfExecutable::format_traits_t<architecture>;
//
//	auto exec = dynamic_cast<ElfExecutable::ImageLayout const*>(layout);
//	(exec);
//
//	(host);
//	(length);
//	(executable);
//
//	return nullptr;
//}
//
////-----------------------------------------------------------------------------
//// LoadElfBinary<Architecture>
////
//// Loads an ELF binary image into a host instance and returns the metadata
////
//// Arguments:
////
////	host		- Host instance in which to load the image
////	executable	- ElfExecutable instance to be loaded
//
//template<enum class Architecture architecture>
//std::unique_ptr<Executable::ImageLayout> LoadElfBinary(Host* host, ElfExecutable const* executable)
//{
//	using elf = ElfExecutable::format_traits_t<architecture>;
//
//	ElfExecutable::imagelayout_t			layout;			// Layout of the loaded executable image
//
//	// Get pointers to the main ELF header and the first program header
//	auto elfheader = reinterpret_cast<elf::elfheader_t const*>(&executable->m_headers[0]);
//	auto progheaders = reinterpret_cast<elf::progheader_t const*>(&executable->m_headers[0] + elfheader->e_phoff);
//
//	// Determine the memory footprint of the binary image by scanning all PT_LOAD segments
//	uintptr_t minvaddr = UINTPTR_MAX, maxvaddr = 0;
//	for(size_t index = 0; index < elfheader->e_phnum; index++) {
//
//		if((progheaders[index].p_type == LINUX_PT_LOAD) && (progheaders[index].p_memsz)) {
//
//			// Calculate the minimum and maximum physical addresses of the segment and adjust accordingly
//			minvaddr = std::min(uintptr_t{ progheaders[index].p_vaddr }, minvaddr);
//			maxvaddr = std::max(uintptr_t{ progheaders[index].p_vaddr + progheaders[index].p_memsz }, maxvaddr);
//		}
//	}
//
//	try {
//
//		// ET_EXEC images must be reserved at the proper virtual address; ET_DYN images can go anywhere so reserve
//		// them at the highest available virtual address to allow for as much heap space as possible in the process
//		if(elfheader->e_type == LINUX_ET_EXEC) layout.baseaddress = host->AllocateMemory(reinterpret_cast<void*>(minvaddr), maxvaddr - minvaddr, LINUX_PROT_NONE);
//		else layout.baseaddress = host->AllocateMemory(maxvaddr - minvaddr, LINUX_PROT_NONE);
//	}
//	catch(Exception& ex) { throw LinuxException{ LINUX_ENOMEM, Exception{ E_ELFRESERVEREGION, ex } }; }
//
//	// ET_EXEC images are loaded at their virtual address, whereas ET_DYN images need a load delta to work with
//	intptr_t vaddrdelta = (elfheader->e_type == LINUX_ET_EXEC) ? 0 : uintptr_t(layout.baseaddress) - minvaddr;
//
//	// Iterate over and load/process all of the program header sections
//	for(size_t index = 0; index < elfheader->e_phnum; index++) {
//
//		// PT_PHDR - if it falls within the boundaries of the loadable segments, provide the layout values
//		//
//		if((progheaders[index].p_type == LINUX_PT_PHDR) && (progheaders[index].p_vaddr >= minvaddr) && ((progheaders[index].p_vaddr + progheaders[index].p_memsz) <= maxvaddr)) {
//
//			layout.progheaders = reinterpret_cast<void*>(uintptr_t(progheaders[index].p_vaddr) + vaddrdelta);
//			layout.numprogheaders = progheaders[index].p_memsz / elfheader->e_phentsize;
//		}
//
//		// PT_LOAD - load the segment into the host process and set the protection flags
//		//
//		else if((progheaders[index].p_type == LINUX_PT_LOAD) && (progheaders[index].p_memsz)) {
//
//			// Get the base address of the loadable segment and set it as PAGE_READWRITE to load it
//			uintptr_t segbase = progheaders[index].p_vaddr + vaddrdelta;
//			try { host->ProtectMemory(reinterpret_cast<void*>(segbase), progheaders[index].p_memsz, LINUX_PROT_READ | LINUX_PROT_WRITE); }
//			catch(Exception& ex) { throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFCOMMITSEGMENT, ex } }; }
//
//			// Not all segments contain data that needs to be copied from the source image
//			if(progheaders[index].p_filesz) {
//
//				size_t written = 0;						// Bytes written into the target process
//
//				// Read the data from the image file into the target process address space at segbase
//				try { written = host->WriteMemoryFrom(executable->m_handle, progheaders[index].p_offset, reinterpret_cast<void*>(segbase), progheaders[index].p_filesz); }
//				catch(Exception& ex) { throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFWRITESEGMENT, ex } }; }
//
//				if(written != progheaders[index].p_filesz) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFIMAGETRUNCATED } };
//			}
//
//			// Attempt to apply the proper virtual memory protection flags to the segment now that it's been written
//			try { host->ProtectMemory(reinterpret_cast<void*>(segbase), progheaders[index].p_memsz, convert<Host::MemoryProtection>(progheaders[index].p_flags)); }
//			catch(Exception& ex) { throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFPROTECTSEGMENT, ex } }; }
//		}
//	}
//
//	// The initial program break address is the page just beyond the last allocated image segment
//	layout.breakaddress = align::up(reinterpret_cast<void*>(maxvaddr + vaddrdelta), SystemInformation::PageSize);
//
//	// Calculate the address of the image entry point, if one has been specified in the header
//	if(elfheader->e_entry) layout.entrypoint = reinterpret_cast<void*>(elfheader->e_entry + vaddrdelta);
//
//	// Construct and return a new ElfExecutable::ImageLayout instance from the collected layout data
//	return std::make_unique<ElfExecutable::ImageLayout>(std::move(layout));
//}
//
////-----------------------------------------------------------------------------
//// ReadElfHeaders<Architecture>
////
//// Reads and validates the headers from an ELF binary image
////
//// Arguments:
////
////	handle		- File system object handle from which to read
//
//template <enum class Architecture architecture>
//std::unique_ptr<uint8_t[]> ReadElfHeaders(std::shared_ptr<FileSystem::Handle> handle)
//{
//	using elf = ElfExecutable::format_traits_t<architecture>;
//
//	typename elf::elfheader_t	elfheader;			// Primary ELF header information
//
//	// Read the primary ELF header from the file at offset zero
//	if(handle->ReadAt(0, &elfheader, sizeof(typename elf::elfheader_t)) != sizeof(typename elf::elfheader_t)) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFTRUNCATEDHEADER } };
//
//	// Verify the ELF header magic number
//	if(memcmp(&elfheader.e_ident[LINUX_EI_MAG0], LINUX_ELFMAG, LINUX_SELFMAG) != 0) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFINVALIDMAGIC } };
//
//	// Verify the ELF class is appropriate for this architecture
//	if(elfheader.e_ident[LINUX_EI_CLASS] != elf::elfclass) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFINVALIDCLASS, elfheader.e_ident[LINUX_EI_CLASS] } };
//
//	// Verify the endianness and version of the ELF binary image
//	if(elfheader.e_ident[LINUX_EI_DATA] != LINUX_ELFDATA2LSB) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFINVALIDENCODING, elfheader.e_ident[LINUX_EI_DATA] } };
//	if(elfheader.e_ident[LINUX_EI_VERSION] != LINUX_EV_CURRENT) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFINVALIDVERSION, elfheader.e_ident[LINUX_EI_VERSION] } };
//
//	// Only ET_EXEC and ET_DYN images are currently supported by the virtual machine
//	if((elfheader.e_type != LINUX_ET_EXEC) && (elfheader.e_type != LINUX_ET_DYN)) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFINVALIDTYPE, elfheader.e_type } };
//
//	// The machine type must match the value defined for the format_traits_t<>
//	if(elfheader.e_machine != elf::machinetype) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFINVALIDMACHINETYPE, elfheader.e_machine } };
//
//	// Verify that the version code matches the ELF headers used
//	if(elfheader.e_version != LINUX_EV_CURRENT) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFINVALIDVERSION, elfheader.e_version } };
//
//	// Verify that the header length matches the architecture and that the program and section header entries are appropriate
//	if(elfheader.e_ehsize != sizeof(typename elf::elfheader_t)) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFHEADERFORMAT } };
//	if((elfheader.e_phentsize) && (elfheader.e_phentsize < sizeof(typename elf::progheader_t))) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFPROGHEADERFORMAT } };
//	if((elfheader.e_shentsize) && (elfheader.e_shentsize < sizeof(typename elf::sectheader_t))) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFSECTHEADERFORMAT } };
//
//	// Determine how much data needs to be loaded from the file to contain all the program headers
//	size_t bloblength = elfheader.e_phoff + (elfheader.e_phnum * elfheader.e_phentsize);
//	
//	// Load the portion of the header required for loading the image into a heap buffer
//	auto blob = std::make_unique<uint8_t[]>(bloblength);
//	if(handle->ReadAt(0, &blob[0], bloblength) != bloblength) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFIMAGETRUNCATED } };
//
//	return blob;
//}
//
////-----------------------------------------------------------------------------
//// ReadElfInterpreterPath<Architecture>
////
//// Reads the interpreter (dynamic linker) path from an ELF binary image
////
//// Arguments:
////
////	headers		- Pointer to the ELF headers (main and program)
////	handle		- File system object handle from which to read
//
//template<enum class Architecture architecture>
//std::string ReadElfInterpreterPath(void const* headers, std::shared_ptr<FileSystem::Handle> handle)
//{
//	using elf = ElfExecutable::format_traits_t<architecture>;
//
//	if(headers == nullptr) throw LinuxException{ LINUX_EFAULT };
//
//	// Cast out an architecture-appropriate pointer to the main ELF header
//	typename elf::elfheader_t const* elfheader = reinterpret_cast<typename elf::elfheader_t const*>(headers);
//
//	// Iterate over all of the program headers in the blob to find the interpreter section
//	for(size_t index = 0; index < elfheader->e_phnum; index++) {
//
//		// Calcuate a pointer to the next program header in the blob
//		uintptr_t offset(uintptr_t(headers) + elfheader->e_phoff + (index * elfheader->e_phentsize));
//		typename elf::progheader_t const* progheader = reinterpret_cast<typename elf::progheader_t const*>(offset);
//
//		// PT_INTERP - The section contains the interpreter binary path
//		if(progheader->p_type == LINUX_PT_INTERP) {
//
//			// Copy the interpreter path from the binary image into a local heap buffer
//			auto interpreter = std::make_unique<char_t[]>(progheader->p_filesz);
//			if(handle->ReadAt(progheader->p_offset, &interpreter[0], progheader->p_filesz) != progheader->p_filesz)
//				throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFIMAGETRUNCATED } };
//
//			// Convert the path into an std::string and return it to the caller
//			return std::string(&interpreter[0], progheader->p_filesz);
//		}
//	}
//
//	return std::string();				// No PT_INTERP section was located in the file
//}

//-----------------------------------------------------------------------------
// ElfExecutable Constructor (private)
//
// Arguments:
//
//	architecture	- Executable image architecture flag
//	interpreter		- Path to the binary interpreter / dynamic linker

ElfExecutable::ElfExecutable(enum class Architecture architecture, std::string&& interpreter) : 
	m_architecture(architecture), m_interpreter(std::move(interpreter))
{
}

//-----------------------------------------------------------------------------
// ElfExecutable::getArchitecture
//
// Gets the architecture flag for the executable

enum class Architecture ElfExecutable::getArchitecture(void) const
{
	return m_architecture;
}

//-----------------------------------------------------------------------------
// ElfExecutable::getFormat
//
// Gets the binary format of the executable

enum class ExecutableFormat ElfExecutable::getFormat(void) const
{
	return ExecutableFormat::ELF;
}

//-----------------------------------------------------------------------------
// ElfExecutable::FromHandle (static)
//
// Creates a new ElfExecutable instance from an existing file handle
//
// Arguments:
//
//	handle			- Open executable file handle
//	host			- Host instance in which to load the executable
//	ns				- Namespace to use for additional path resolution
//	root			- Root path to use for additional path resolution
//	current			- Current path to use for additional path resolution
//	originalpath	- Originally specified executable path
//	arguments		- Vector of command-line arguments
//	environment		- Vector of environment variables
//	stacklength		- Length of the stack to be created in the host

std::unique_ptr<ElfExecutable> ElfExecutable::FromHandle(std::shared_ptr<FileSystem::Handle> handle, PathResolver resolver, std::vector<std::string>&& arguments, 
		std::vector<std::string>&& environment, char_t const* originalpath)
{
	if(originalpath == nullptr) throw LinuxException{ LINUX_EFAULT };

	// Read in the ELF identification data from the head of the provided file handle
	uint8_t ident[LINUX_EI_NIDENT];
	if(handle->ReadAt(0, &ident[0], LINUX_EI_NIDENT) != LINUX_EI_NIDENT) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFIMAGETRUNCATED } };

	// LINUX_ELFCLASS32 --> Architecture::x86
	//
	if(ident[LINUX_EI_CLASS] == LINUX_ELFCLASS32) {

		(environment);
		(arguments);
		return nullptr;
		//auto headers = ReadElfHeaders<Architecture::x86>(handle);
		//auto interpreter = ReadElfInterpreterPath<Architecture::x86>(headers.get(), handle);

		//return std::make_unique<ElfExecutable>(Architecture::x86, originalpath, std::move(handle), std::move(headers), std::move(interpreter),
		//	std::move(arguments), std::move(environment));
	}

#ifdef _M_X64
	// LINUX_ELFCLASS64 --> Architecture::x86_64
	//
	else if(ident[LINUX_EI_CLASS] == LINUX_ELFCLASS64) {

		//auto headers = ReadElfHeaders<Architecture::x86_64>(handle);
		//auto interpreter = ReadElfInterpreterPath<Architecture::x86_64>(headers.get(), handle);

		//return std::make_unique<ElfExecutable>(Architecture::x86_64, originalpath, std::move(handle), std::move(headers), std::move(interpreter),
		//	std::move(arguments), std::move(environment));
	}
#endif	// _M_X64

	// Unknown or unsupported ELF binary image class
	else throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFINVALIDCLASS, ident[LINUX_EI_CLASS] } };
}

//-----------------------------------------------------------------------------
// ElfExecutable::getInterpreter
//
// Gets the path to the specified interpreter/dynamic linker binary

std::string ElfExecutable::getInterpreter(void) const
{
	return m_interpreter;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
