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
#include "Executable.h"

#include "ElfArguments.h"
#include "ElfImage.h"
#include "HeapBuffer.h"
#include "LinuxException.h"
#include "Namespace.h"
#include "ProcessMemory.h"
#include "Random.h"
#include "SystemInformation.h"

#pragma warning(push, 4)

// INTERPRETER_SCRIPT_MAGIC_ANSI
//
// Magic number present at the head of an ANSI interpreter script
static uint8_t INTERPRETER_SCRIPT_MAGIC_ANSI[] = { 0x23, 0x21 };

// INTERPRETER_SCRIPT_MAGIC_UTF8
//
// Magic number present at the head of a UTF-8 interpreter script
static uint8_t INTERPRETER_SCRIPT_MAGIC_UTF8[] = { 0xEF, 0xBB, 0xBF, 0x23, 0x21 };

//-----------------------------------------------------------------------------
// Executable Constructor (private)
//
// Arguments:
//
//	architecture	- Executable architecture flag
//	format			- Executable binary format
//	handle			- File system handle open for execute access
//	filename		- Original file name provided for the executable
//	arguments		- Executable command-line arguments
//	environment		- Executable environment variables
//	ns				- Namespace in which the executable was looked up
//	rootdir			- Root directory used to resolve the executable
//	workingdir		- Working directory used to resolve the executable

Executable::Executable(::Architecture architecture, BinaryFormat format, std::shared_ptr<FileSystem::Handle> handle, const char_t* filename, 
	const char_t* const* arguments, const char_t* const* environment, std::shared_ptr<class Namespace> ns, std::shared_ptr<FileSystem::Path> rootdir, 
	std::shared_ptr<FileSystem::Path> workingdir) : m_architecture{ architecture }, m_format{ format }, m_handle{ std::move(handle) },
	m_filename{ filename }, m_ns{ std::move(ns) }, m_rootdir{ std::move(rootdir) }, m_workingdir{ std::move(workingdir) }
{
	// Convert the argument and environment variable arrays into vectors of string objects
	while((arguments) && (*arguments)) { m_arguments.push_back(*arguments); arguments++; }
	while((environment) && (*environment)) { m_environment.push_back(*environment); environment++; }
}

//-----------------------------------------------------------------------------
// Executable::getArchitecture
//
// Gets the architecture flag for the referenced executable

::Architecture Executable::getArchitecture(void) const
{
	return m_architecture;
}

//-----------------------------------------------------------------------------
// Executable::getArgument
//
// Gets a pointer to a command line argument string

const char_t* Executable::getArgument(int index) const
{
	// The argument collection will exist without modification as long as
	// this class is alive, can return a direct pointer to the string
	if(index >= static_cast<int>(m_arguments.size())) throw LinuxException{ LINUX_EINVAL };
	return m_arguments[index].c_str();
}

//-----------------------------------------------------------------------------
// Executable::getArgumentCount
//
// Gets the number of command line argument strings

size_t Executable::getArgumentCount(void) const
{
	return m_arguments.size();
}

//-----------------------------------------------------------------------------
// Executable::getEnvironmentVariable
//
// Gets a pointer to an environment variable string

const char_t* Executable::getEnvironmentVariable(int index) const
{
	// The environment variable collection will exist without modification as long as
	// this class is alive, can return a direct pointer to the string
	if(index >= static_cast<int>(m_environment.size())) throw LinuxException{ LINUX_EINVAL };
	return m_environment[index].c_str();
}

//-----------------------------------------------------------------------------
// Executable::getEnvironmentVariableCount
//
// Gets the number of environment variable strings

size_t Executable::getEnvironmentVariableCount(void) const
{
	return m_environment.size();
}

//-----------------------------------------------------------------------------
// Executable::getFileName
//
// Gets the original file name provided for the executable

const char_t* Executable::getFileName(void) const
{
	return m_filename.c_str();
}

//-----------------------------------------------------------------------------
// Executable::getFormat
//
// Gets the format of the referenced executable

Executable::BinaryFormat Executable::getFormat(void) const
{
	return m_format;
}

//-----------------------------------------------------------------------------
// Executable::FromFile (static)
//
// Creates an executable instance from a file system file
//
// Arguments:
//
//	ns				- Namespace in which to operate
//	rootdir			- Root directory to assign to the process
//	workingdir		- Working directory to assign to the process
//	filename		- Path to the executable image
//	arguments		- Command line arguments for the executable
//	environment		- Environment variables to assign to the process

std::unique_ptr<Executable> Executable::FromFile(std::shared_ptr<class Namespace> ns, std::shared_ptr<FileSystem::Path> rootdir, 
	std::shared_ptr<FileSystem::Path> workingdir, const char_t* filename, const char_t* const* arguments, const char_t* const* environment)
{
	// Invoke the private version using the provided file name as the 'original' file name.  This needs to be
	// tracked in order to support the ELF AT_EXECFN auxiliary vector value
	return FromFile(std::move(ns), std::move(rootdir), std::move(workingdir), filename, filename, arguments, environment);
}

//-----------------------------------------------------------------------------
// Executable::FromFile (private, static)
//
// Creates an executable instance from a file system file
//
// Arguments:
//
//	ns					- Namespace in which to operate
//	rootdir				- Root directory to assign to the process
//	workingdir			- Working directory to assign to the process
//	originalfilename	- Original file name passed into public FromFile()
//	filename			- Path to the executable image
//	arguments			- Command line arguments for the executable
//	environment			- Environment variables to assign to the process

std::unique_ptr<Executable> Executable::FromFile(std::shared_ptr<class Namespace> ns, std::shared_ptr<FileSystem::Path> rootdir, 
	std::shared_ptr<FileSystem::Path> workingdir, const char_t* originalfilename, const char_t* filename, const char_t* const* arguments, 
	const char_t* const* environment)
{
	if(filename == nullptr) throw LinuxException{ LINUX_EFAULT };

	// Attempt to open an executable handle to the specified file
	fshandle_t handle = FileSystem::OpenExecutable(ns, rootdir, workingdir, filename);

	// Ensure at compile-time that EI_NIDENT will be a big enough magic number buffer
	static_assert(LINUX_EI_NIDENT >= sizeof(INTERPRETER_SCRIPT_MAGIC_ANSI), "Executable::FromFile -- Magic number buffer too small");
	static_assert(LINUX_EI_NIDENT >= sizeof(INTERPRETER_SCRIPT_MAGIC_UTF8), "Executable::FromFile -- Magic number buffer too small");

	// Read in enough data from the head of the file to determine the executable type
	uint8_t magic[LINUX_EI_NIDENT];
	size_t read = handle->Read(magic, LINUX_EI_NIDENT);

	// ELF BINARY
	if((read >= LINUX_EI_NIDENT) && (memcmp(magic, LINUX_ELFMAG, LINUX_SELFMAG) == 0)) {

		// Move the file pointer back to the beginning of the file
		handle->Seek(0, LINUX_SEEK_SET);

		// This is a binary file, determine the architecture and complete the operation
		switch(magic[LINUX_EI_CLASS]) {

			// ELFCLASS32 --> Architecture::x86
			case LINUX_ELFCLASS32: return std::make_unique<Executable>(Architecture::x86, BinaryFormat::ELF, std::move(handle), 
				originalfilename, arguments, environment, std::move(ns), std::move(rootdir), std::move(workingdir));
#ifdef _M_X64
			// ELFCLASS64: --> Architecture::x86_64
			case LINUX_ELFCLASS64:  return std::make_unique<Executable>(Architecture::x86_64, BinaryFormat::ELF, std::move(handle), 
				originalfilename, arguments, environment, std::move(ns), std::move(rootdir), std::move(workingdir));
#endif
			// Unknown ELFCLASS --> ENOEXEC	
			default: throw LinuxException{ LINUX_ENOEXEC };
		}
	}

	// A.OUT BINARY
	// TODO

	// INTERPRETER SCRIPT (UTF-8)
	else if((read >= sizeof(INTERPRETER_SCRIPT_MAGIC_UTF8)) && (memcmp(magic, &INTERPRETER_SCRIPT_MAGIC_UTF8, sizeof(INTERPRETER_SCRIPT_MAGIC_UTF8)) == 0)) {

		// Move the file pointer back to the position immediately after the magic number and fall through
		handle->Seek(sizeof(INTERPRETER_SCRIPT_MAGIC_UTF8), LINUX_SEEK_SET);
	}

	// INTERPRETER SCRIPT (ANSI)
	else if((read >= sizeof(INTERPRETER_SCRIPT_MAGIC_ANSI)) && (memcmp(magic, &INTERPRETER_SCRIPT_MAGIC_ANSI, sizeof(INTERPRETER_SCRIPT_MAGIC_ANSI)) == 0)) {

		// Move the file pointer back to the position immediately after the magic number and fall through
		handle->Seek(sizeof(INTERPRETER_SCRIPT_MAGIC_ANSI), LINUX_SEEK_SET);
	}

	// UNSUPPORTED FORMAT
	else throw LinuxException{ LINUX_ENOEXEC };

	//
	// This is an interpreter script, pull out the necessary strings and recursively call this function.
	// The file handle is expected to be at the position immediately after the magic number
	//

	char_t *begin, *end;					// String tokenizing pointers

	// Read up to MAX_PATH data from the interpreter script file
	HeapBuffer<char_t> buffer(MAX_PATH);
	char_t *eof = &buffer + handle->Read(&buffer, buffer.Size);

	// Find the interperter path, if not present the script is not a valid target
	for(begin = &buffer; (begin < eof) && (*begin) && (*begin != '\n') && (isspace(*begin)); begin++);
	for(end = begin; (end < eof) && (*end) && (*end != '\n') && (!isspace(*end)); end++);
	if(begin == end) throw LinuxException{ LINUX_ENOEXEC };
	std::string interpreter(begin, end);

	// Find the optional argument string that follows the interpreter path
	for(begin = end; (begin < eof) && (*begin) && (*begin != '\n') && (isspace(*begin)); begin++);
	for(end = begin; (end < eof) && (*end) && (*end != '\n') && (!isspace(*end)); end++);
	std::string argument(begin, end);

	// Create a new argument array to pass back in, using the parsed interpreter and argument
	std::vector<const char_t*> newarguments;
	newarguments.push_back(interpreter.c_str());
	if(argument.length()) newarguments.push_back(argument.c_str());
	newarguments.push_back(filename);

	// Append the original argv[1] .. argv[n] pointers to the new argument array
	if(arguments && (*arguments)) arguments++;
	while((arguments) && (*arguments)) { newarguments.push_back(*arguments); arguments++; }
	newarguments.push_back(nullptr);

	// Recursively call back into FromFile with the interpreter path and modified arguments
	return FromFile(ns, std::move(rootdir), std::move(workingdir), originalfilename, interpreter.c_str(), newarguments.data(), environment);
}

//-----------------------------------------------------------------------------
// Executable::getHandle
//
// Gets the file system handle from which to load/execute the binary

std::shared_ptr<FileSystem::Handle> Executable::getHandle(void) const
{
	return m_handle;
}

//-----------------------------------------------------------------------------
// Executable::Load
//
// Loads the executable into a process virtual address space
//
// Arguments:
//
//	memory			- ProcessMemory instance for the target process
//	stackpointer	- Pointer to the stack to be initialized

Executable::LoadResult Executable::Load(const std::unique_ptr<ProcessMemory>& memory, const void* stackpointer) const
{
	// Invoke the format-specific load function
	switch(m_format) {

		// ELF -> LoadELF
		case BinaryFormat::ELF: return LoadELF(memory, stackpointer);
	}

	// Unsupported binary file format
	throw LinuxException{ LINUX_ENOEXEC };
}

//-----------------------------------------------------------------------------
// Executable::LoadELF (private)
//
// Loads an ELF binary into a process virtual address space
//
// Arguments:
//
//	memory			- ProcessMemory instance for the target process
//	stackpointer	- Pointer to the stack to be initialized

Executable::LoadResult Executable::LoadELF(const std::unique_ptr<ProcessMemory>& memory, const void* stackpointer) const
{
	// Architecture::x86 --> 32-bit ELF binary
	if(m_architecture == Architecture::x86) return LoadELF<Architecture::x86>(memory, stackpointer);

#ifdef _M_X64
	// Architecture::x86_64 --> 64-bit ELF binary
	else if(m_architecture == Architecture::x86_64) return LoadELF<Architecture::x86_64>(memory, stackpointer;
#endif

	// Unsupported architecture
	throw LinuxException{ LINUX_ENOEXEC };
}

//-----------------------------------------------------------------------------
// Executable::LoadELF<Architecture> (private)
//
// Loads an ELF binary into a process virtual address space
//
// Arguments:
//
//	memory			- ProcessMemory instance for the target process
//	stackpointer	- Pointer to the stack to be initialized

template <::Architecture architecture>
Executable::LoadResult Executable::LoadELF(const std::unique_ptr<ProcessMemory>& memory, const void* stackpointer) const
{
	using elf = elf_traits<architecture>;

	std::unique_ptr<ElfImage>	executable;			// Main executable image
	std::unique_ptr<ElfImage>	interpreter;		// Optional interpreter image
	uint8_t						random[16];			// Random data for AT_RANDOM
	LoadResult					result;				// Result from load operation

	// Load the main executable image into the process
	executable = ElfImage::Load<architecture>(m_handle, memory);

	// If an interpreter is specified by the main executable, open and load that into the process
	if(executable->Interpreter) interpreter = ElfImage::Load<architecture>(FileSystem::OpenExecutable(m_ns, m_rootdir, m_workingdir, executable->Interpreter), memory);

	// Generate the AT_RANDOM auxiliary vector data
	Random::Generate(random, sizeof(random));

	// Construct the ELF arguments to write into the specified stack
	ElfArguments arguments(m_arguments, m_environment);

	(LINUX_AT_EXECFD);																			//  2 - TODO MAY NOT NEED TO IMPLEMENT
	if(executable->ProgramHeaders) {

		arguments.AppendAuxiliaryVector(LINUX_AT_PHDR, executable->ProgramHeaders);				//  3
		arguments.AppendAuxiliaryVector(LINUX_AT_PHENT, sizeof(typename elf::progheader_t));	//  4
		arguments.AppendAuxiliaryVector(LINUX_AT_PHNUM, executable->NumProgramHeaders);			//  5
	}

	arguments.AppendAuxiliaryVector(LINUX_AT_PAGESZ, SystemInformation::PageSize);				//  6
	if(interpreter) arguments.AppendAuxiliaryVector(LINUX_AT_BASE, interpreter->BaseAddress);	//  7
	arguments.AppendAuxiliaryVector(LINUX_AT_FLAGS, 0);											//  8
	arguments.AppendAuxiliaryVector(LINUX_AT_ENTRY, executable->EntryPoint);					//  9
	(LINUX_AT_NOTELF);																			// 10 - NOT IMPLEMENTED
	(LINUX_AT_UID);																				// 11 - TODO
	(LINUX_AT_EUID);																			// 12 - TODO
	(LINUX_AT_GID);																				// 13 - TODO
	(LINUX_AT_EGID);																			// 14 - TODO
	arguments.AppendAuxiliaryVector(LINUX_AT_PLATFORM, elf::platform);							// 15
	arguments.AppendAuxiliaryVector(LINUX_AT_HWCAP, SystemInformation::ProcessorFeatureMask);	// 16
	arguments.AppendAuxiliaryVector(LINUX_AT_CLKTCK, 100);										// 17
	arguments.AppendAuxiliaryVector(LINUX_AT_SECURE, 0);										// 23 - TODO (SUID) SEE GETAUXVAL(3)
	(LINUX_AT_BASE_PLATFORM);																	// 24 - NOT IMPLEMENTED
	arguments.AppendAuxiliaryVector(LINUX_AT_RANDOM, random, sizeof(random));					// 25
	(LINUX_AT_HWCAP2);																			// 26 - NOT IMPLEMENTED
	arguments.AppendAuxiliaryVector(LINUX_AT_EXECFN, m_filename.c_str());						// 31
	(LINUX_AT_SYSINFO);																			// 32 - TODO MAY NOT IMPLEMENT?
	(LINUX_AT_SYSINFO_EHDR);																	// 33 - TODO NEEDS VDSO

	// Set the entry point and program break information from the load operation
	result.EntryPoint = (interpreter) ? interpreter->EntryPoint : executable->EntryPoint;
	result.ProgramBreak = executable->ProgramBreak;

	// Write the ELF arguments to the provided stack and set the adjusted stack pointer
	result.StackPointer = arguments.WriteStack<architecture>(memory, stackpointer);

	return result;
}

//-----------------------------------------------------------------------------
// Executable::getNamespace
//
// Gets the namespace from which the executable was resolved

std::shared_ptr<class Namespace> Executable::getNamespace(void) const
{
	return m_ns;
}

//-----------------------------------------------------------------------------
// Executable::getRootDirectory
//
// Gets the root directory used to resolve the executable binary

std::shared_ptr<FileSystem::Path> Executable::getRootDirectory(void) const
{
	return m_rootdir;
}

//-----------------------------------------------------------------------------
// Executable::getWorkingDirectory
//
// Gets the working directory used to resolve the executable binary

std::shared_ptr<FileSystem::Path> Executable::getWorkingDirectory(void) const
{
	return m_rootdir;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
