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
#include "VmProcessManager.h"

#pragma warning(push, 4)

// XXXX_MAGIC
//
// Arrays that define the supported binary magic numbers
static uint8_t ANSI_SCRIPT_MAGIC[]	= { 0x23, 0x21 };							// "#!"
static uint8_t UTF8_SCRIPT_MAGIC[]	= { 0xEF, 0xBB, 0xBF, 0x23, 0x21 };			// "#!"
static uint8_t UTF16_SCRIPT_MAGIC[]	= { 0xFF, 0xFE, 0x23, 0x00, 0x21, 0x00 };	// "#!"

//-----------------------------------------------------------------------------
// VmProcessManager::CreateProcess
//
// Creates a new Process instance from a file system binary
//
// Arguments:
//
//	vm				- Pointer to the VirtualMachine instance
//	path			- Path to the file system object to execute as a process
//	arguments		- Pointer to an array of command line argument strings
//	environment		- Pointer to the process environment variables

std::shared_ptr<Process> VmProcessManager::CreateProcess(const std::shared_ptr<VirtualMachine>& vm, const uapi::char_t* path, 
	const uapi::char_t** arguments, const uapi::char_t** environment)
{
	if(!path) throw LinuxException(LINUX_EFAULT);

	// Attempt to open an execute handle for the specified path
	FileSystem::HandlePtr handle = vm->OpenExecutable(path);

	// Read in just enough from the head of the file to look for magic numbers
	uint8_t magic[LINUX_EI_NIDENT];

	MagicNumbers magics;
	size_t read = handle->Read(&magics, sizeof(MagicNumbers));
	handle->Seek(0, LINUX_SEEK_SET);

	// Check for an ELF binary image
	if((read >= sizeof(magics.ElfBinary)) && (memcmp(&magics.ElfBinary, LINUX_ELFMAG, LINUX_SELFMAG) == 0)) {

		switch(magics.ElfBinary[LINUX_EI_CLASS]) {

			// ELFCLASS32: Create a 32-bit host process for the binary
			case LINUX_ELFCLASS32: 
				return Process::Create<ElfClass::x86>(vm, handle, arguments, environment, m_hostpath32.c_str(), m_hostargs32.c_str());
#ifdef _M_X64
			// ELFCLASS64: Create a 64-bit host process for the binary
			case LINUX_ELFCLASS64: 
				return Process::Create<ElfClass::x86_64>(vm, handle, arguments, environment, m_hostpath64.c_str(), m_hostargs64.c_str());
#endif
			// Any other ELFCLASS -> ENOEXEC	
			default: throw LinuxException(LINUX_ENOEXEC);
		}
	}

	// TODO: Linux doesn't support UTF-16 or UTF-8 encoding; these can go. If the file is UTF-8 that's fine as long as it
	// doesn't have the encoding prefix bytes

	// Check for UTF-16 interpreter script
	else if((read >= sizeof(UTF16_SCRIPT_MAGIC)) && (memcmp(&magics.UTF16Script, &UTF16_SCRIPT_MAGIC, sizeof(UTF16_SCRIPT_MAGIC)) == 0)) {

		// TODO : need to use locale
		// parse binary and command line, recursively call back into Create()
		throw Exception(E_NOTIMPL);
	}

	// Check for UTF-8 interpreter script
	else if((read >= sizeof(UTF8_SCRIPT_MAGIC)) && (memcmp(&magics.UTF8Script, &UTF8_SCRIPT_MAGIC, sizeof(UTF8_SCRIPT_MAGIC)) == 0)) {

		// TODO : need to use locale
		// parse binary and command line, recursively call back into Create()
		throw Exception(E_NOTIMPL);
	}

	// Check for ANSI interpreter script
	else if((read >= sizeof(ANSI_SCRIPT_MAGIC)) && (memcmp(&magics.AnsiScript, &ANSI_SCRIPT_MAGIC, sizeof(ANSI_SCRIPT_MAGIC)) == 0)) {

		char_t *begin, *end;					// String tokenizing pointers

		// Reset the file pointer back to the immediately after the magic numbers
		handle->Seek(sizeof(ANSI_SCRIPT_MAGIC), LINUX_SEEK_SET);

		// Read up to the allocated buffer worth of data from the file
		HeapBuffer<uapi::char_t> buffer(MAX_PATH);
		char_t *eof = &buffer + handle->Read(&buffer, buffer.Size);

		// Find the interperter string, if not present the script is invalid
		for(begin = &buffer; (begin < eof) && (*begin) && (*begin != '\n') && (isspace(*begin)); begin++);
		for(end = begin; (end < eof) && (*end) && (*end != '\n') && (!isspace(*end)); end++);
		if(begin == end) throw LinuxException(LINUX_ENOEXEC);
		std::string interpreter(begin, end);

		// Find the optional argument string
		for(begin = end; (begin < eof) && (*begin) && (*begin != '\n') && (isspace(*begin)); begin++);
		for(end = begin; (end < eof) && (*end) && (*end != '\n') && (!isspace(*end)); end++);
		std::string argument(begin, end);

		// Create a new argument array to pass back in, using the parsed interpreter and argument
		std::vector<const char_t*> newarguments;
		newarguments.push_back(interpreter.c_str());
		if(argument.length()) newarguments.push_back(argument.c_str());
		newarguments.push_back(path);

		// Append the original argv[1] .. argv[n] to the new argument array
		if(arguments && (*arguments)) arguments++;
		while((arguments) && (*arguments)) { newarguments.push_back(*arguments); arguments++; }
		newarguments.push_back(nullptr);

		// Recursively call CreateProcess with the new path and arguments
		return CreateProcess(vm, interpreter.c_str(), newarguments.data(), environment);
	}

	// No other formats are currently recognized as valid executable binaries
	throw LinuxException(LINUX_ENOEXEC);
}

//---------------------------------------------------------------------------

#pragma warning(pop)
