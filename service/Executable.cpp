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

#include <cctype>
#include "LinuxException.h"

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
//	arch			- Executable architecture flag
//	format			- Executable binary format flag
//	originalpath	- Originally specified executable path
//	handle			- Handle instance open against the target file
//	arguments		- Processed command line arguments
//	environment		- Processed environment variables

Executable::Executable(enum class Architecture arch, enum class BinaryFormat format, char_t const* originalpath, std::shared_ptr<FileSystem::Handle> handle,
	string_vector_t&& arguments, string_vector_t&& environment) : m_architecture(arch), m_format(format), m_originalpath(originalpath),
	m_handle(std::move(handle)), m_arguments(std::move(arguments)), m_environment(std::move(environment))
{
}

//-----------------------------------------------------------------------------
// Executable::getArchitecture
//
// Gets the architecture flag for the executable

enum class Architecture Executable::getArchitecture(void) const
{
	return m_architecture;
}

//-----------------------------------------------------------------------------
// Executable::getArguments
//
// Gets a reference to the contained arguments vector

std::vector<std::string> const& Executable::getArguments(void) const
{
	return m_arguments;
}

//-----------------------------------------------------------------------------
// Executable::getEnvironmentVariables
//
// Gets a reference to the contained environment variables vector

std::vector<std::string> const& Executable::getEnvironmentVariables(void) const
{
	return m_environment;
}

//-----------------------------------------------------------------------------
// Executable::getFormat
//
// Gets the binary format of the executable

enum class BinaryFormat Executable::getFormat(void) const
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
//	root			- Root directory to assign to the process
//	current			- Working directory to assign to the process
//	path			- Path to the executable image

std::unique_ptr<Executable> Executable::FromFile(std::shared_ptr<class Namespace> ns, std::shared_ptr<FileSystem::Path> root, 
	std::shared_ptr<FileSystem::Path> current, char_t const* path)
{
	if(path == nullptr) throw LinuxException{ LINUX_EFAULT };
	
	// Provide empty vectors for the arguments and environment variables
	return FromFile(std::move(ns), std::move(root), std::move(current), path, path, string_vector_t(), string_vector_t());
}

//-----------------------------------------------------------------------------
// Executable::FromFile (static)
//
// Creates an executable instance from a file system file
//
// Arguments:
//
//	ns				- Namespace in which to operate
//	root			- Root directory to assign to the process
//	current			- Working directory to assign to the process
//	path			- Path to the executable image
//	arguments		- Array of command-line arguments
//	environment		- Array of environment variables

std::unique_ptr<Executable> Executable::FromFile(std::shared_ptr<class Namespace> ns, std::shared_ptr<FileSystem::Path> root, 
	std::shared_ptr<FileSystem::Path> current, char_t const* path, char_t const* const* arguments, char_t const* const* environment)
{
	if(path == nullptr) throw LinuxException{ LINUX_EFAULT };
	
	// Convert the C-style string arrays into vector<string> containers
	return FromFile(std::move(ns), std::move(root), std::move(current), path, path, StringArrayToVector(arguments), StringArrayToVector(environment));
}

//-----------------------------------------------------------------------------
// Executable::FromFile (private, static)
//
// Creates an executable instance from a file system file
//
// Arguments:
//
//	ns				- Namespace in which to operate
//	root			- Root directory to assign to the process
//	current			- Working directory to assign to the process
//	originalpath	- Original path provided for the executable
//	path			- Path to the executable image
//	arguments		- Vector of command-line arguments
//	environment		- Vector of environment variables

std::unique_ptr<Executable> Executable::FromFile(namespace_t ns, fspath_t root, fspath_t current, char_t const* originalpath,
	char_t const* path, string_vector_t&& arguments, string_vector_t&& environment)
{
	if(originalpath == nullptr) throw LinuxException{ LINUX_EFAULT };
	if(path == nullptr) throw LinuxException{ LINUX_EFAULT };
	
	// Acquire an execute-only handle for the provided path
	auto handle = FileSystem::OpenExecutable(ns, root, current, path);

	// Read in enough data from the beginning of the file to determine the executable type
	uint8_t magic[LINUX_EI_NIDENT];
	size_t read = handle->ReadAt(0, magic, LINUX_EI_NIDENT);

	// ELF BINARY
	//
	if((read >= LINUX_EI_NIDENT) && (memcmp(magic, LINUX_ELFMAG, LINUX_SELFMAG) == 0)) {

		// This is a binary file, determine the architecture and complete the operation
		switch(magic[LINUX_EI_CLASS]) {

			// ELFCLASS32 --> Architecture::x86
			case LINUX_ELFCLASS32: 
				return std::make_unique<Executable>(Architecture::x86, BinaryFormat::ELF, originalpath, std::move(handle), std::move(arguments), std::move(environment));
#ifdef _M_X64
			// ELFCLASS64: --> Architecture::x86_64
			case LINUX_ELFCLASS64: 
				return std::make_unique<Executable>(Architecture::x86_64, BinaryFormat::ELF, originalpath, std::move(handle), std::move(arguments), std::move(environment));
#endif
			// Unknown or unsupported ELFCLASS --> ENOEXEC	
			default: throw LinuxException{ LINUX_ENOEXEC };
		}
	}

	// A.OUT BINARIES
	//
	// TODO - FUTURE (OMAGIC, NMAGIC, QMAGIC, etc)

	// INTERPRETER SCRIPT (UTF-8)
	//
	else if((read >= sizeof(INTERPRETER_SCRIPT_MAGIC_UTF8)) && (memcmp(magic, &INTERPRETER_SCRIPT_MAGIC_UTF8, sizeof(INTERPRETER_SCRIPT_MAGIC_UTF8)) == 0)) {

		return FromScript(std::move(ns), std::move(root), std::move(current), originalpath, std::move(handle), sizeof(INTERPRETER_SCRIPT_MAGIC_UTF8),
			std::move(arguments), std::move(environment));
	}

	// INTERPRETER SCRIPT (ANSI)
	//
	else if((read >= sizeof(INTERPRETER_SCRIPT_MAGIC_ANSI)) && (memcmp(magic, &INTERPRETER_SCRIPT_MAGIC_ANSI, sizeof(INTERPRETER_SCRIPT_MAGIC_ANSI)) == 0)) {

		return FromScript(std::move(ns), std::move(root), std::move(current), originalpath, std::move(handle), sizeof(INTERPRETER_SCRIPT_MAGIC_ANSI),
			std::move(arguments), std::move(environment));
	}

	// UNSUPPORTED FORMAT
	//
	else throw LinuxException{ LINUX_ENOEXEC };
}

//-----------------------------------------------------------------------------
// Executable::FromScript (private, static)
//
// Creates an executable instance from an interpreter script
//
// Arguments:
//
//	ns				- Namespace in which to operate
//	root			- Root directory to assign to the process
//	current			- Working directory to assign to the process
//	originalpath	- Original path provided for the executable
//	scripthandle	- Handle to the interpreter script
//	dataoffset		- Offset of data within the interpreter script
//	arguments		- Vector of command-line arguments
//	environment		- Vector of environment variables

std::unique_ptr<Executable> Executable::FromScript(namespace_t ns, fspath_t root, fspath_t current, char_t const* originalpath,
	fshandle_t scripthandle, size_t dataoffset, string_vector_t&& arguments, string_vector_t&& environment)
{
	char_t					buffer[MAX_PATH];			// Script data buffer
	string_vector_t			newarguments;				// New executable arguments

	if(originalpath == nullptr) throw LinuxException{ LINUX_EFAULT };

	char_t *begin, *end;					// String tokenizing pointers

	// Read up to MAX_PATH data from the interpreter script file
	char_t *eof = &buffer[0] + scripthandle->ReadAt(dataoffset, &buffer[0], sizeof(buffer));

	// Find the interperter path, if not present the script is not a valid target
	for(begin = &buffer[0]; (begin < eof) && (*begin) && (*begin != '\n') && (std::isspace(*begin)); begin++);
	for(end = begin; (end < eof) && (*end) && (*end != '\n') && (!std::isspace(*end)); end++);
	if(begin == end) throw LinuxException{ LINUX_ENOEXEC };
	std::string interpreter(begin, end);

	// Find the optional argument string that follows the interpreter path
	for(begin = end; (begin < eof) && (*begin) && (*begin != '\n') && (std::isspace(*begin)); begin++);
	for(end = begin; (end < eof) && (*end) && (*end != '\n') && (!std::isspace(*end)); end++);
	std::string argument(begin, end);

	// Create a new arguments vector for the target interpreter binary
	newarguments.push_back(interpreter);
	if(argument.length()) newarguments.push_back(std::move(argument));
	newarguments.emplace_back(originalpath);

	if(!arguments.empty()) {
	
		// Append the original argv[1] .. argv[n] arguments to the new vector (argv[0] is discarded if present)
		for(auto iterator = arguments.begin() + 1; iterator != arguments.end(); iterator++) newarguments.push_back(*iterator);
	}

	// Call back into FromFile with the interpreter binary as the target and new arguments
	return FromFile(ns, std::move(root), std::move(current), originalpath, interpreter.c_str(), std::move(newarguments), std::move(environment));
}

//-----------------------------------------------------------------------------
// Executable::getHandle
//
// Gets a reference to the handle instance opened for the executable

std::shared_ptr<FileSystem::Handle> Executable::getHandle(void) const
{
	return m_handle;
}

//-----------------------------------------------------------------------------
// Executable::getOriginalPath
//
// Gets the originally specified path of the executable

char_t const* Executable::getOriginalPath(void) const
{
	return m_originalpath.c_str();
}

//-----------------------------------------------------------------------------
// Executable::StringArrayToVector (static, private)
//
// Converts a null-terminated array of C-style strings into a vector<>
//
// Arguments:
//
//	strings		- Null-terminated array of C-style strings

Executable::string_vector_t Executable::StringArrayToVector(char_t const* const* strings)
{
	string_vector_t vec;
	while((strings) && (*strings)) { vec.push_back(*strings); strings++; }

	return vec;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
