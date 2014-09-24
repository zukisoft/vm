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
#include "Process.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Process::Create (static)
//
// Creates a new Process instance
//
// Arguments:
//
//	vm			- Reference to the VirtualMachine instance
//	path		- Path to the file system object to execute as a process

std::unique_ptr<Process> Process::Create(const std::shared_ptr<VirtualMachine>& vm, const tchar_t* path)
{
	ElfCommon_Ehdr			elfheader;				// <-- TODO: needs to be a union with script bytes

	if(!path) throw LinuxException(LINUX_EFAULT);

	// need to use working directory, requires a calling process context
	// will impersonate on the Rpc interface, that should be fine

	// Attempt to open an execute handle for the specified path
	FileSystem::HandlePtr handle = vm->FileSystem->OpenExec(path);

	// Try to read in the first bytes of the file to examine it in more detail and move
	// the file pointer back to the beginning so it can be cleanly handed off to a loader

	// this needs to be a union with the interpreter script stuff (ANSI, UTF8+CHARS, UTF16+WCHARS),
	// read up to the size of the Union, then check stuff.  Scripts, look for "#!", "[UTF8]#!", "[UTF16]#!",
	// elf look for magic and then the ELF class flag (32/64)
	size_t read = handle->Read(&elfheader, sizeof(ElfCommon_Ehdr));
	handle->Seek(0, LINUX_SEEK_SET);

	if(handle->Read(&elfheader, sizeof(ElfCommon_Ehdr)) == sizeof(ElfCommon_Ehdr)) {

		// Reset the file pointer before creating a StreamReader instance against it
		// to ensure that it's read from the beginning
		handle->Seek(0, LINUX_SEEK_SET);
	}


	// CHECK IF INTERPRETER SCRIPT
	// CHECK IF ELF 32
	// CHECK IF ELF 64

	// ELF:
	// Create a host instance
	// Load the binary
	// LOOP to load interpreters:
	//	if interpreter, vm->FileSystem->OpenExec(interpreter)
	//	adjust entry point and whatever else needs adjusting
	// Unsuspend host to launch?
	
	// SCRIPT:
	// Process interperter, try to open it with OpenExec()
	// Create a host instance
	// do interesting stuff

	// return a Process instance

	vm->SystemLog->Push("Hello World");

	return std::make_unique<Process>(Host::Create(nullptr, nullptr, 0));
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
