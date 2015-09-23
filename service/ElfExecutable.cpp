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

#include <array>
#include "Exception.h"
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

// uint32_t --> ProcessMemory::Protection
//
// Converts ELF protection flags into a ProcessMemory::Protection bitmask
template<> ProcessMemory::Protection convert<ProcessMemory::Protection>(uint32_t flags)
{
	_ASSERTE((flags & ~(LINUX_PF_R | LINUX_PF_W | LINUX_PF_X)) == 0);

	ProcessMemory::Protection result(ProcessMemory::Protection::None);

	if(flags & LINUX_PF_R) result = (result | ProcessMemory::Protection::Read);
	if(flags & LINUX_PF_W) result = (result | ProcessMemory::Protection::Write);
	if(flags & LINUX_PF_X) result = (result | ProcessMemory::Protection::Execute);

	return result;
}

//-----------------------------------------------------------------------------
// ElfExecutable Constructor (private)
//
// Arguments:
//
//	architecture	- Executable image architecture flag
//	handle			- Handle to the primary executable image
//	interpreter		- Handle to the interpreter image
//	originalpath	- Original path provided for the executable
//	headers			- ELF headers extracted from the image
//	arguments		- Vector of command line arguments
//	environment		- Vector of environment variables

ElfExecutable::ElfExecutable(enum class Architecture architecture, fshandle_t handle, fshandle_t interpreter, char_t const* originalpath, headerblob_t&& headers,
	stringvector_t&& arguments, stringvector_t&& environment) : m_architecture(architecture), m_handle(std::move(handle)), m_interpreter(std::move(interpreter)),
	m_originalpath(originalpath), m_headers(std::move(headers)), m_arguments(std::move(arguments)), m_environment(std::move(environment))
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
//	resolver		- Callback used to open additional file handles
//	arguments		- Vector of command-line arguments
//	environment		- Vector of environment variables
//	originalpath	- Original path specified for the executable

std::unique_ptr<ElfExecutable> ElfExecutable::FromHandle(std::shared_ptr<FileSystem::Handle> handle, PathResolver resolver, std::vector<std::string>&& arguments, 
		std::vector<std::string>&& environment, char_t const* originalpath)
{
	if(originalpath == nullptr) throw LinuxException{ LINUX_EFAULT };

	// Read the ELF identification data from the image handle
	std::array<uint8_t, LINUX_EI_NIDENT> identification;
	if(handle->ReadAt(0, &identification, LINUX_EI_NIDENT) != LINUX_EI_NIDENT) throw LinuxException{ LINUX_ENOEXEC };

	// LINUX_ELFCLASS32 --> Architecture::x86
	if(identification[LINUX_EI_CLASS] == LINUX_ELFCLASS32) 
		return FromHandle<Architecture::x86>(std::move(handle), resolver, std::move(arguments), std::move(environment), originalpath);

#ifdef _M_X64
	// LINUX_ELFCLASS64 --> Architecture::x86_64
	else if(identification[LINUX_EI_CLASS] == LINUX_ELFCLASS64) 
		return FromHandle<Architecture::x86_64>(std::move(handle), resolver, std::move(arguments), std::move(environment), originalpath);
#endif

	else throw LinuxException{ LINUX_ENOEXEC };
}

//-----------------------------------------------------------------------------
// ElfExecutable::FromHandle<Architecture> (static, private)
//
// Creates a new ElfExecutable instance from an existing file handle
//
// Arguments:
//
//	handle			- Open executable file handle
//	resolver		- Callback used to open additional file handles
//	arguments		- Vector of command-line arguments
//	environment		- Vector of environment variables
//	originalpath	- Original path specified for the executable

template<enum class Architecture architecture>
std::unique_ptr<ElfExecutable> ElfExecutable::FromHandle(fshandle_t handle, PathResolver resolver, stringvector_t&& arguments, stringvector_t&& environment, 
	char_t const* originalpath)
{
	fshandle_t					interpreter;				// Handle to the specified interpreter binary

	// Extract the headers from the image file and acquire the path to the interpreter binary
	auto headers = ReadHeaders<architecture>(handle);
	auto interpreterpath = ReadInterpreterPath<architecture>(headers.get(), handle);

	// If an interpreter path was specified in the image, open a file handle for it, otherwise leave it null
	if(interpreterpath.length()) interpreter = resolver(interpreterpath.c_str());

	// Construct the ElfExecutable instance, transferring ownership of collected information
	return std::make_unique<ElfExecutable>(architecture, std::move(handle), std::move(interpreter), originalpath, std::move(headers), 
		std::move(arguments), std::move(environment));
}

//-----------------------------------------------------------------------------
// ElfExecutable::Load
//
// Loads the executable into a process
//
// Arguments:
//
//	mem				- ProcessMemory implementation for the target process
//	stacklength		- Length of the stack to create in the process

std::unique_ptr<Executable::Layout> ElfExecutable::Load(ProcessMemory* mem, size_t stacklength)
{
	if(mem == nullptr) throw LinuxException{ LINUX_EFAULT };
	if(stacklength == 0) throw LinuxException{ LINUX_EINVAL };

	// Architecture::x86
	if(m_architecture == Architecture::x86) return Load<Architecture::x86>(mem, stacklength);

#ifdef _M_X64
	// Architecture::x86_64
	else if(m_architecture == Architecture::x86_64) return Load<Architecture::x86_64>(mem, stacklength);
#endif

	else throw LinuxException{ LINUX_ENOEXEC };
}	

//-----------------------------------------------------------------------------
// ElfExecutable::Load<Architecture> (private)
//
// Loads the executable into a process
//
// Arguments:
//
//	mem				- ProcessMemory implementation for the target process
//	stacklength		- Length of the stack to create in the process

template<enum class Architecture architecture>
std::unique_ptr<Executable::Layout> ElfExecutable::Load(ProcessMemory* mem, size_t stacklength)
{
	using elf = format_traits_t<architecture>;

	// Load the primary executable image into the process
	auto layout = LoadImage<architecture>(m_headers.get(), m_handle, mem);

	if(m_interpreter) {

		// An interpreter binary has been specified for this executable, attempt to load it into the process
		auto interpreter = LoadImage<architecture>(ReadHeaders<architecture>(m_interpreter).get(), m_interpreter, mem);

		// The interpreter's base address, break address and entry point override that of the main executable
		layout.baseaddress = interpreter.baseaddress;
		layout.breakaddress = interpreter.breakaddress;
		layout.entrypoint = interpreter.entrypoint;
	}

	// Create the stack image for the executable in the process
	// todo
	(stacklength);
	auto stacklayout = stacklayout_t();

	return std::make_unique<ElfExecutable::Layout>(architecture, std::move(layout), std::move(stacklayout));
}

//-----------------------------------------------------------------------------
// ElfExecutable::LoadImage<Architecture> (static, private)
//
// Loads an image into a process
//
// Arguments:
//
//	headers		- Pointer to the ELF headers
//	handle		- File system object handle for the image
//	mem			- ProcessMemory implementation for the process

template<enum class Architecture architecture>
ElfExecutable::imagelayout_t ElfExecutable::LoadImage(void const* headers, fshandle_t handle, ProcessMemory* mem)
{
	using elf = format_traits_t<architecture>;

	ElfExecutable::imagelayout_t			layout;			// Layout of the loaded executable image

	// Get pointers to the main ELF header and the first program header
	auto elfheader = reinterpret_cast<elf::elfheader_t const*>(headers);
	auto progheaders = reinterpret_cast<elf::progheader_t const*>(uintptr_t(headers) + elfheader->e_phoff);

	// Determine the memory footprint of the image by scanning all PT_LOAD segments
	uintptr_t minvaddr = UINTPTR_MAX, maxvaddr = 0;
	for(size_t index = 0; index < elfheader->e_phnum; index++) {

		if((progheaders[index].p_type == LINUX_PT_LOAD) && (progheaders[index].p_memsz)) {

			// Calculate the minimum and maximum physical addresses of the segment and adjust accordingly
			minvaddr = std::min(uintptr_t{ progheaders[index].p_vaddr }, minvaddr);
			maxvaddr = std::max(uintptr_t{ progheaders[index].p_vaddr + progheaders[index].p_memsz }, maxvaddr);
		}
	}

	// ET_EXEC images must be reserved at the proper virtual address; ET_DYN images can go anywhere
	try { layout.baseaddress = mem->ReserveMemory((elfheader->e_type == LINUX_ET_EXEC) ? minvaddr : uintptr_t(0), maxvaddr - minvaddr); }
	catch(Exception& ex) { throw LinuxException{ LINUX_ENOMEM, ex }; }

	//
	// MAP THE SECTION INTO THE CALLING PROCESS HERE
	// WILL NEED TRY/CATCH TO ENSURE IT GETS UNMAPPED
	//

	// ET_EXEC images are loaded at their virtual address, whereas ET_DYN images need a load delta to work with
	intptr_t vaddrdelta = (elfheader->e_type == LINUX_ET_EXEC) ? 0 : layout.baseaddress - minvaddr;

	// Iterate over and load/process all of the program header sections
	for(size_t index = 0; index < elfheader->e_phnum; index++) {

		// PT_PHDR - if it falls within the boundaries of the loadable segments, provide the layout values
		//
		if((progheaders[index].p_type == LINUX_PT_PHDR) && (progheaders[index].p_vaddr >= minvaddr) && ((progheaders[index].p_vaddr + progheaders[index].p_memsz) <= maxvaddr)) {

			layout.progheaders = uintptr_t(progheaders[index].p_vaddr) + vaddrdelta;
			layout.numprogheaders = progheaders[index].p_memsz / elfheader->e_phentsize;
		}

		// PT_LOAD - load the segment into the process and set the protection flags
		//
		else if((progheaders[index].p_type == LINUX_PT_LOAD) && (progheaders[index].p_memsz)) {

			// Get the base address of the loadable segment and allocate it with the proper protection
			uintptr_t segbase = progheaders[index].p_vaddr + vaddrdelta;
			try { mem->AllocateMemory(segbase, progheaders[index].p_memsz, convert<ProcessMemory::Protection>(progheaders[index].p_flags)); }
			catch(Exception& ex) { throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFCOMMITSEGMENT, ex } }; }

			// Not all segments contain data that needs to be copied from the source image
			if(progheaders[index].p_filesz) {

				// TODO: UNBREAK - NEEDS MEMORY MAPPED INTO THIS CALLING PROCESS NOW

				//size_t written = 0;						// Bytes written into the target process

				// Read the data from the image file into the target process address space at segbase
				//try { written = mem->WriteMemoryFrom(handle, progheaders[index].p_offset, reinterpret_cast<void*>(segbase), progheaders[index].p_filesz); }
				//catch(Exception& ex) { throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFWRITESEGMENT, ex } }; }

				//if(written != progheaders[index].p_filesz) throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFIMAGETRUNCATED } };
			}

			// Attempt to apply the proper virtual memory protection flags to the segment now that it's been written
			// TODO: NO LONGER NECESSARY
			//try { mem->ProtectMemory(segbase, progheaders[index].p_memsz, convert<ProcessMemory::Protection>(progheaders[index].p_flags)); }
			//catch(Exception& ex) { throw LinuxException{ LINUX_ENOEXEC, Exception{ E_ELFPROTECTSEGMENT, ex } }; }
		}
	}

	// The initial program break address is the page just beyond the last allocated image segment
	layout.breakaddress = align::up(maxvaddr + vaddrdelta, SystemInformation::PageSize);

	// Calculate the address of the image entry point, if one has been specified in the header
	if(elfheader->e_entry) layout.entrypoint = uintptr_t(elfheader->e_entry) + vaddrdelta;

	return layout;
}
	
//-----------------------------------------------------------------------------
// ElfExecutable::ReadHeaders<Architecture> (static, private)
//
// Reads and validates the headers from an ELF image file
//
// Arguments:
//
//	handle		- File system object handle from which to read

template<enum class Architecture architecture>
ElfExecutable::headerblob_t ElfExecutable::ReadHeaders(fshandle_t handle)
{
	using elf = format_traits_t<architecture>;

	typename elf::elfheader_t	elfheader;			// Primary ELF header information

	// Read the primary ELF header from the file at offset zero
	if(handle->ReadAt(0, &elfheader, sizeof(typename elf::elfheader_t)) != sizeof(typename elf::elfheader_t)) throw LinuxException{ LINUX_ENOEXEC };

	// Verify the ELF header magic number
	if(memcmp(&elfheader.e_ident[LINUX_EI_MAG0], LINUX_ELFMAG, LINUX_SELFMAG) != 0) throw LinuxException{ LINUX_ENOEXEC };

	// Verify the ELF class is appropriate for this architecture
	if(elfheader.e_ident[LINUX_EI_CLASS] != elf::elfclass) throw LinuxException{ LINUX_ENOEXEC };

	// Verify the endianness and version of the ELF binary image
	if(elfheader.e_ident[LINUX_EI_DATA] != LINUX_ELFDATA2LSB) throw LinuxException{ LINUX_ENOEXEC };
	if(elfheader.e_ident[LINUX_EI_VERSION] != LINUX_EV_CURRENT) throw LinuxException{ LINUX_ENOEXEC };

	// Only ET_EXEC and ET_DYN images are currently supported by the virtual machine
	if((elfheader.e_type != LINUX_ET_EXEC) && (elfheader.e_type != LINUX_ET_DYN)) throw LinuxException{ LINUX_ENOEXEC };

	// The machine type must match the value defined for the format_traits_t<>
	if(elfheader.e_machine != elf::machinetype) throw LinuxException{ LINUX_ENOEXEC };

	// Verify that the version code matches the ELF headers used
	if(elfheader.e_version != LINUX_EV_CURRENT) throw LinuxException{ LINUX_ENOEXEC };

	// Verify that the header length matches the architecture and that the program and section header entries are appropriate
	if(elfheader.e_ehsize != sizeof(typename elf::elfheader_t)) throw LinuxException{ LINUX_ENOEXEC };;
	if((elfheader.e_phentsize) && (elfheader.e_phentsize < sizeof(typename elf::progheader_t))) throw LinuxException{ LINUX_ENOEXEC };
	if((elfheader.e_shentsize) && (elfheader.e_shentsize < sizeof(typename elf::sectheader_t))) throw LinuxException{ LINUX_ENOEXEC };

	// Determine how much data needs to be loaded from the file to contain all the program headers
	size_t headerslength = elfheader.e_phoff + (elfheader.e_phnum * elfheader.e_phentsize);
	
	// Load the portion of the header required for loading the image into a heap buffer
	auto headers = std::make_unique<uint8_t[]>(headerslength);
	if(handle->ReadAt(0, &headers[0], headerslength) != headerslength) throw LinuxException{ LINUX_ENOEXEC };

	return headers;
}

//-----------------------------------------------------------------------------
// ElfExecutable::ReadInterpreterPath<Architecture> (static, private)
//
// Reads the interpreter (dynamic linker) path from an ELF binary image
//
// Arguments:
//
//	headers		- Pointer to the ELF headers
//	handle		- File system object handle from which to read

template<enum class Architecture architecture>
std::string ElfExecutable::ReadInterpreterPath(void const* headers, fshandle_t handle)
{
	using elf = format_traits_t<architecture>;

	if(headers == nullptr) throw LinuxException{ LINUX_EFAULT };

	// Cast out an architecture-appropriate pointer to the main ELF header
	typename elf::elfheader_t const* elfheader = reinterpret_cast<typename elf::elfheader_t const*>(headers);

	// Iterate over all of the program headers in the blob to find the PT_INTERP section
	for(size_t index = 0; index < elfheader->e_phnum; index++) {

		// Calcuate a pointer to the next program header in the headers blob
		uintptr_t offset(uintptr_t(headers) + elfheader->e_phoff + (index * elfheader->e_phentsize));
		typename elf::progheader_t const* progheader = reinterpret_cast<typename elf::progheader_t const*>(offset);

		// PT_INTERP - The section contains the interpreter binary path
		if(progheader->p_type == LINUX_PT_INTERP) {

			// Copy the interpreter path from the binary image into a local heap buffer
			auto interpreter = std::make_unique<char_t[]>(progheader->p_filesz);
			if(handle->ReadAt(progheader->p_offset, &interpreter[0], progheader->p_filesz) != progheader->p_filesz) throw LinuxException{ LINUX_ENOEXEC };

			// Convert the path into an std::string and return it to the caller
			return std::string(&interpreter[0], progheader->p_filesz);
		}
	}

	return std::string();				// No PT_INTERP section was located in the file
}

//
// ELFEXECUTABLE:IMAGELAYOUT IMPLEMENTATION
//

//-----------------------------------------------------------------------------
// ElfExecutable::Layout Constructor
//
// Arguments:
//
//	architecture	- Architecture flag for the loaded image
//	layout			- Layout information for the loaded executable
//	stacklayout		- Layout information for the created stack

ElfExecutable::Layout::Layout(enum class Architecture architecture, imagelayout_t&& layout, stacklayout_t&& stacklayout) : 
	m_architecture(architecture), m_layout(std::move(layout)), m_stacklayout(std::move(stacklayout))
{
}

//-----------------------------------------------------------------------------
// ElfExecutable::Layout::getArchitecture
//
// Gets the architecture flag for the executable

enum class Architecture ElfExecutable::Layout::getArchitecture(void) const
{
	return m_architecture;
}

//-----------------------------------------------------------------------------
// ElfExecutable::Layout::getBreakAddress
//
// Pointer to the initial program break address

uintptr_t ElfExecutable::Layout::getBreakAddress(void) const
{
	return m_layout.breakaddress;
}

//-----------------------------------------------------------------------------
// ElfExecutable::Layout::getEntryPoint
//
// Gets the entry point of the loaded executable image

uintptr_t ElfExecutable::Layout::getEntryPoint(void) const
{
	return m_layout.entrypoint;
}

//-----------------------------------------------------------------------------
// ElfExecutable::Layout::getStackPointer
//
// Gets the stack pointer for the created stack image

uintptr_t ElfExecutable::Layout::getStackPointer(void) const
{
	return m_stacklayout.stackpointer;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
