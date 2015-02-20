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
// Executable Constructor
//
// Arguments:
//
//	architecture	- Executable architecture flag
//	handle			- File system handle open for execute access
//	arguments		- Executable command-line arguments
//	environment		- Executable environment variables
//	rootdir			- Root directory used to resolve the executable
//	workingdir		- Working directory used to resolve the executable

Executable::Executable(::Architecture architecture, std::shared_ptr<FileSystem::Handle>&& handle, const char_t* const* arguments, 
	const char_t* const* environment, const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir) : 
	m_architecture(architecture), m_handle(std::move(handle)), m_rootdir(rootdir), m_workingdir(workingdir)
{
	// Convert the argument and environment variable arrays into vectors of string objects
	while((arguments) && (*arguments)) { m_arguments.push_back(*arguments); arguments++; }
	while((environment) && (*environment)) { m_environment.push_back(*environment); environment++; }
}

//-----------------------------------------------------------------------------
// Architecture::getArchitecture
//
// Gets the architecture flag for the referenced executable

::Architecture Executable::getArchitecture(void) const
{
	return m_architecture;
}

//-----------------------------------------------------------------------------
// Architecture::getArgument
//
// Gets a pointer to a command line argument string

const char_t* Executable::getArgument(int index) const
{
	// The argument collection will exist without modification as long as
	// this class is alive, can return a direct pointer to the string
	if(index >= static_cast<int>(m_arguments.size())) throw LinuxException(LINUX_EINVAL);
	return m_arguments[index].c_str();
}

//-----------------------------------------------------------------------------
// Architecture::getArgumentCount
//
// Gets the number of command line argument strings

size_t Executable::getArgumentCount(void) const
{
	return m_arguments.size();
}

//-----------------------------------------------------------------------------
// Architecture::getEnvironmentVariable
//
// Gets a pointer to an environment variable string

const char_t* Executable::getEnvironmentVariable(int index) const
{
	// The environment variable collection will exist without modification as long as
	// this class is alive, can return a direct pointer to the string
	if(index >= static_cast<int>(m_environment.size())) throw LinuxException(LINUX_EINVAL);
	return m_environment[index].c_str();
}

//-----------------------------------------------------------------------------
// Architecture::getEnvironmentVariableCount
//
// Gets the number of environment variable strings

size_t Executable::getEnvironmentVariableCount(void) const
{
	return m_environment.size();
}

//-----------------------------------------------------------------------------
// Executable::FromFile (static)
//
// Creates an executable instance from a file system file
//
// Arguments:
//
//	vm				- VirtualMachine instance creating the process
//	filename		- Path to the executable image
//	arguments		- Command line arguments for the executable
//	environment		- Environment variables to assign to the process
//	rootdir			- Root directory to assign to the process
//	workingdir		- Working directory to assign to the process

std::unique_ptr<Executable> Executable::FromFile(const std::shared_ptr<VirtualMachine>& vm, const char_t* filename, const char_t* const* arguments, 
	const char_t* const* environment, const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir)
{
	if(filename == nullptr) throw LinuxException(LINUX_EFAULT);

	// Attempt to open an executable handle to the specified file
	std::shared_ptr<FileSystem::Handle> handle = vm->OpenExecutable(rootdir, workingdir, filename);

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
			case LINUX_ELFCLASS32: return std::make_unique<Executable>(Architecture::x86, std::move(handle), arguments, environment, rootdir, workingdir);
#ifdef _M_X64
			// ELFCLASS64: --> Architecture::x86_64
			case LINUX_ELFCLASS64:  return std::make_unique<Executable>(Architecture::x86_64, std::move(handle), arguments, environment, rootdir, workingdir);
#endif
			// Unknown ELFCLASS --> ENOEXEC	
			default: throw LinuxException(LINUX_ENOEXEC);
		}
	}

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
	else throw LinuxException(LINUX_ENOEXEC);

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
	if(begin == end) throw LinuxException(LINUX_ENOEXEC);
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
	return FromFile(vm, interpreter.c_str(), newarguments.data(), environment, rootdir, workingdir);
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
// Executable::getRootDirectory
//
// Gets the root directory used to resolve the executable binary

std::shared_ptr<FileSystem::Alias> Executable::getRootDirectory(void) const
{
	return m_rootdir;
}

//-----------------------------------------------------------------------------
// Executable::getWorkingDirectory
//
// Gets the working directory used to resolve the executable binary

std::shared_ptr<FileSystem::Alias> Executable::getWorkingDirectory(void) const
{
	return m_rootdir;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
