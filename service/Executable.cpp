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
#include "ElfExecutable.h"
#include "Host.h"
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
// Executable::FromFile (static)
//
// Creates an Executable instance from a file system file
//
// Arguments:
//
//	resolver		- Function to use to resolve a file system path
//	path			- Path to the executable image
//	arguments		- Array of command-line arguments
//	environment		- Array of environment variables

std::unique_ptr<Executable> Executable::FromFile(PathResolver resolver, char_t const* path, char_t const* const* arguments, char_t const* const* environment)
{
	if(path == nullptr) throw LinuxException{ LINUX_EFAULT };

	// Convert the C-style string arrays into vector<string> containers and invoke the internal implementation.  Note that
	// the path is provided twice, once as the path to resolve and once to track the 'original' path argument that was sent in
	return FromFile(resolver, path, StringArrayToVector(arguments), StringArrayToVector(environment), path);
}

//-----------------------------------------------------------------------------
// Executable::FromFile (private, static)
//
// Creates an Executable instance from a file system file
//
// Arguments:
//
//	resolver		- Function to use to resolve a file system path
//	path			- Path to the executable image
//	arguments		- Vector of command-line arguments
//	environment		- Vector of environment variables
//	originalpath	- Original path provided for the executable

std::unique_ptr<Executable> Executable::FromFile(PathResolver resolver, char_t const* path, string_vector_t&& arguments, string_vector_t&& environment, char_t const* originalpath)
{
	_ASSERTE((originalpath != nullptr) && (path != nullptr));

	auto handle = resolver(path);					// Acquire execute handle for the target path

	// Read in enough data from the beginning of the file to determine the executable type
	uint8_t magic[LINUX_EI_NIDENT];
	size_t read = handle->ReadAt(0, magic, LINUX_EI_NIDENT);

	// ELF
	//
	if((read >= LINUX_EI_NIDENT) && (memcmp(magic, LINUX_ELFMAG, LINUX_SELFMAG) == 0))
		return ElfExecutable::FromHandle(std::move(handle), resolver, std::move(arguments), std::move(environment), originalpath);

	// A.OUT
	//
	// TODO - FUTURE (OMAGIC, NMAGIC, QMAGIC, etc)

	// ANSI INTERPRETER SCRIPT
	//
	else if((read >= sizeof(INTERPRETER_SCRIPT_MAGIC_ANSI)) && (memcmp(magic, &INTERPRETER_SCRIPT_MAGIC_ANSI, sizeof(INTERPRETER_SCRIPT_MAGIC_ANSI)) == 0))
		return FromScriptFile(std::move(handle), sizeof(INTERPRETER_SCRIPT_MAGIC_ANSI), resolver, std::move(arguments), std::move(environment), originalpath);

	// UTF-8 INTERPRETER SCRIPT (UTF-8)
	//
	else if((read >= sizeof(INTERPRETER_SCRIPT_MAGIC_UTF8)) && (memcmp(magic, &INTERPRETER_SCRIPT_MAGIC_UTF8, sizeof(INTERPRETER_SCRIPT_MAGIC_UTF8)) == 0))
		return FromScriptFile(std::move(handle), sizeof(INTERPRETER_SCRIPT_MAGIC_UTF8), resolver, std::move(arguments), std::move(environment), originalpath);

	// UNSUPPORTED FORMAT
	//
	else throw LinuxException{ LINUX_ENOEXEC };
}

//-----------------------------------------------------------------------------
// Executable::FromScriptFile (private, static)
//
// Creates an executable instance from an interpreter script
//
// Arguments:
//
//	handle			- Handle to the interpreter script
//	offset			- Offset within the script to begin processing
//	originalpath	- Original path provided for the executable
//	arguments		- Vector of command-line arguments
//	environment		- Vector of environment variables

std::unique_ptr<Executable> Executable::FromScriptFile(std::shared_ptr<FileSystem::Handle> handle, size_t offset, PathResolver resolver, 
		string_vector_t&& arguments, string_vector_t&& environment, char_t const* originalpath)
{
	char_t					buffer[MAX_PATH];			// Script data buffer
	string_vector_t			newarguments;				// New executable arguments

	_ASSERTE(originalpath != nullptr);

	char_t *begin, *end;					// String tokenizing pointers

	// Read up to MAX_PATH data from the interpreter script file
	char_t *eof = &buffer[0] + handle->ReadAt(offset, &buffer[0], sizeof(buffer));

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

	// Call back into FromFile with the resolved interpreter binary as the target and updated arguments
	return FromFile(resolver, interpreter.c_str(), std::move(newarguments), std::move(environment), originalpath);
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
