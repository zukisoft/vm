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

// XXXX_MAGIC
//
// Arrays that define the supported binary magic numbers
static uint8_t ANSI_MAGIC[]		= { 0x23, 0x21, 0x20 };
static uint8_t UTF8_MAGIC[]		= { 0xEF, 0xBB, 0xBF, 0x23, 0x21, 0x20 };
static uint8_t UTF16_MAGIC[]	= { 0xFF, 0xFE, 0x23, 0x00, 0x21, 0x00, 0x20, 0x00 };

//-----------------------------------------------------------------------------
// Process::Create (static)
//
// Creates a new Process instance
//
// Arguments:
//
//	vm				- Reference to the VirtualMachine instance
//	path			- Path to the file system object to execute as a process
//	arguments		- Pointer to an array of command line argument strings
//	environment		- Pointer to the process environment variables

std::unique_ptr<Process> Process::Create(const std::shared_ptr<VirtualMachine>& vm, const uapi::char_t* path,
	const uapi::char_t** arguments, const uapi::char_t** environment)
{
	if(!path) throw LinuxException(LINUX_EFAULT);

	// Attempt to open an execute handle for the specified path
	FileSystem::HandlePtr handle = vm->FileSystem->OpenExec(std::to_tstring(path).c_str());

	// todo: put an ExecuteHandle in TempFileSystem and return that, it will always
	// be called by the kernel so there is a lot less to watch out for

	// Read in just enough from the head of the file to look for magic numbers
	MagicNumbers magics;
	size_t read = handle->Read(&magics, sizeof(MagicNumbers));
	handle->Seek(0, LINUX_SEEK_SET);

	// Check for an ELF binary image
	if((read >= sizeof(magics.ELF)) && (memcmp(&magics.ELF, LINUX_ELFMAG, LINUX_SELFMAG) == 0)) {

		switch(magics.ELF[LINUX_EI_CLASS]) {

			// ELFCLASS32: Create a 32-bit host process for the binary
			case LINUX_ELFCLASS32: 
				return Create<LINUX_ELFCLASS32>(vm, handle, arguments, environment, vm->Settings->Process.Host32.c_str(), vm->Listener32Binding);

#ifdef _M_X64
			// ELFCLASS64: Create a 64-bit host process for the binary
			case LINUX_ELFCLASS32: 
				return Create<LINUX_ELFCLASS64>(vm, handle, arguments, environment, vm->Settings->Process.Host64.c_str(), vm->Listener64Binding);
#endif
			// Any other ELFCLASS -> ENOEXEC	
			default: throw LinuxException(LINUX_ENOEXEC);
		}
	}

	// Check for UTF-16 interpreter script
	else if((read >= sizeof(UTF16_MAGIC)) && (memcmp(&magics.UTF16, &UTF16_MAGIC, sizeof(UTF16_MAGIC)) == 0)) {

		// parse binary and command line, recursively call back into Create()
		throw Exception(E_NOTIMPL);
	}

	// Check for UTF-8 interpreter script
	else if((read >= sizeof(UTF8_MAGIC)) && (memcmp(&magics.UTF8, &UTF8_MAGIC, sizeof(UTF8_MAGIC)) == 0)) {

		// parse binary and command line, recursively call back into Create()
		throw Exception(E_NOTIMPL);
	}

	// Check for ANSI interpreter script
	else if((read >= sizeof(ANSI_MAGIC)) && (memcmp(&magics.ANSI, &ANSI_MAGIC, sizeof(ANSI_MAGIC)) == 0)) {

		// parse binary and command line, recursively call back into Create()
		throw Exception(E_NOTIMPL);
	}

	// No other formats are currently recognized as valid executable binaries
	throw LinuxException(LINUX_ENOEXEC);
}

//-----------------------------------------------------------------------------
// Process::Create (static, private)
//
// Constructs a new process instance from an ELF binary
//
// Arguments:
//
//	vm			- Reference to the VirtualMachine instance
//	handle		- Open FileSystem::Handle against the ELF binary to load
//	argv		- ELF command line arguments from caller
//	envp		- ELF environment variables from caller
//	hostpath	- Path to the external host to load
//	hostargs	- Command line arguments to pass to the host

template <int elfclass>
static std::unique_ptr<Process> Process::Create(const std::shared_ptr<VirtualMachine>& vm, const FileSystem::HandlePtr& handle,
	const uapi::char_t** argv, const uapi::char_t** envp, const tchar_t* hostpath, const tchar_t* hostargs)
{
	std::unique_ptr<ElfImage>		executable;				// The main ELF binary image to be loaded
	std::unique_ptr<ElfImage>		interpreter;			// Optional interpreter image specified by executable

	(argv);		// TODO --> pass to auxiliary vector generator
	(envp);		// TODO --> pass to auxiliary vector generator

	// Create the external host process (suspended by default)
	std::unique_ptr<Host> host = Host::Create(hostpath, hostargs, nullptr, 0);

	try {

		// Attempt to load the binary image into the process, then check for an interpreter
		executable = ElfImage::Load<elfclass>(HandleStreamReader(handle), host->ProcessHandle);
		if(executable->Interpreter) {

			// Acquire a handle to the interpreter binary and attempt to load that into the process
			FileSystem::HandlePtr interphandle = vm->FileSystem->OpenExec(std::to_tstring(executable->Interpreter).c_str());
			interpreter = ElfImage::Load<elfclass>(HandleStreamReader(interphandle), host->ProcessHandle);
		}

		//
		// TODO: CONSTRUCT AUXILIARY VECTORS HERE
		//

		return std::make_unique<Process>(std::move(host));
	}

	// Terminate the host process on exception since it doesn'y get killed by the Host destructor
	catch(...) { host->Terminate(E_FAIL); throw; }
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
