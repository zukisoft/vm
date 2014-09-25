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

uint8_t ansi_magic[]	= { 0x23, 0x21, 0x20 };
uint8_t utf8_magic[]	= { 0xEF, 0xBB, 0xBF, 0x23, 0x21, 0x20 };
uint8_t utf16_magic[]	= { 0xFF, 0xFE, 0x23, 0x00, 0x21, 0x00, 0x20, 0x00 };

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
	BinaryMagic			magics;				// <-- TODO: dumb name

	if(!path) throw LinuxException(LINUX_EFAULT);

	// todo: need to use working directory, requires a calling process context?

	// Attempt to open an execute handle for the specified path
	// todo: put an ExecuteHandle in TempFileSystem and return that, it will always
	// be called by the kernel so there is a lot less to watch out for
	FileSystem::HandlePtr handle = vm->FileSystem->OpenExec(std::to_tstring(path).c_str());

	// Read in just enough from the head of the file to look for magic numbers
	size_t read = handle->Read(&magics, sizeof(BinaryMagic));
	handle->Seek(0, LINUX_SEEK_SET);

	// Check for an ELF binary image
	if((read >= sizeof(magics.elf_ident)) && (memcmp(&magics.elf_ident, LINUX_ELFMAG, LINUX_SELFMAG) == 0)) {

		switch(magics.elf_ident[LINUX_EI_CLASS]) {

			// ELFCLASS32: Create a 32-bit host process for the binary
			case LINUX_ELFCLASS32: return CreateELF32(vm, handle);

#ifdef _M_X64
			// ELFCLASS64: Create a 64-bit host process for the binary
			case LINUX_ELFCLASS64: return CreateELF64(vm, handle);
#endif
			// Any other ELFCLASS -> ENOEXEC	
			default : throw LinuxException(LINUX_ENOEXEC);
		}
	}

	// Check for UTF-16 interpreter script
	else if((read >= sizeof(utf16_magic)) && (memcmp(&magics.utf16_magic, &utf16_magic, sizeof(utf16_magic)) == 0)) {

		return CreateScriptInterpreter(vm, handle);
	}

	// Check for UTF-8 interpreter script
	else if((read >= sizeof(utf8_magic)) && (memcmp(&magics.utf8_magic, &utf8_magic, sizeof(utf8_magic)) == 0)) {

		return CreateScriptInterpreter(vm, handle);
	}

	// Check for ANSI interpreter script
	else if((read >= sizeof(ansi_magic)) && (memcmp(&magics.ansi_magic, &ansi_magic, sizeof(ansi_magic)) == 0)) {

		return CreateScriptInterpreter(vm, handle);
	}

	// No other formats are recognized binaries
	else throw LinuxException(LINUX_ENOEXEC);

	// ELF:
	// Create a host instance
	// Load the binary
	// LOOP to load interpreters:
	//	if interpreter, vm->FileSystem->OpenExec(interpreter)
	//	adjust entry point and whatever else needs adjusting
	// Unsuspend host to launch?
	
	// SCRIPT:
	// Process interperter, recursively call this function with that 

	// return a Process instance

	// Build the auxiliary vector for this process from the provided arguments and environment variables
	std::unique_ptr<AuxiliaryVector> auxvec = std::make_unique<AuxiliaryVector>(arguments, environment);
	// SET AUXVEC stuff here


	return std::make_unique<Process>(Host::Create(nullptr, nullptr, 0));
}

// CreateELF32 (static)
//
// Constructs a new process instance from an ELF32 binary file
std::unique_ptr<Process> Process::CreateELF32(const std::shared_ptr<VirtualMachine>& vm, const FileSystem::HandlePtr& handle)
{
	throw Exception(E_NOTIMPL);
	return nullptr;
}

// CreateELF64 (static)
//
// Constructs a new process instance from an ELF64 binary file
std::unique_ptr<Process> Process::CreateELF64(const std::shared_ptr<VirtualMachine>& vm, const FileSystem::HandlePtr& handle)
{
	throw Exception(E_NOTIMPL);
	return nullptr;
}

// CreateScriptInterpreter (static)
//
// Constructs a new process instance from an interpreter script
std::unique_ptr<Process> Process::CreateScriptInterpreter(const std::shared_ptr<VirtualMachine>& vm, const FileSystem::HandlePtr& handle)
{
	throw Exception(E_NOTIMPL);
	return nullptr;
}

//-----------------------------------------------------------------------------
// Process::Host Constructor
//
// Arguments:
//
//	procinfo		- PROCESS_INFORMATION for the hosted process

Process::Host::Host(const PROCESS_INFORMATION& procinfo)
{
	m_procinfo = procinfo;
}

//-----------------------------------------------------------------------------
// Process::Host Destructor

Process::Host::~Host()
{
	// Close the process handles created along with the host process
	CloseHandle(m_procinfo.hThread);
	CloseHandle(m_procinfo.hProcess);
}

//-----------------------------------------------------------------------------
// Proces::Host::Create (static)
//
// Creates a new Host process instance
//
// Arguments:
//
//	binarypath		- Path to the host binary
//	bindingstring	- RPC binding string to pass to the host binary
//	handles			- Handles to be inherited by the host binary
//	count			- Number of handles in the array

std::unique_ptr<Process::Host> Process::Host::Create(const tchar_t* binarypath, const tchar_t* bindingstring, HANDLE* handles, size_t count)
{
	PROCESS_INFORMATION				procinfo;			// Process information

	// Generate the command line for the child process, which includes the RPC binding string as argv[1] and
	// a 32-bit serialized copy of the inheritable event handle as argv[2]
	tchar_t commandline[MAX_PATH];
	_sntprintf_s(commandline, MAX_PATH, MAX_PATH, _T("\"%s\" \"%s\" \"%ld\""), binarypath, bindingstring, reinterpret_cast<__int32>(handles[0]));

	// Determine the size of the attributes buffer required to hold the inheritable handles property
	SIZE_T required = 0;
	InitializeProcThreadAttributeList(nullptr, 1, 0, &required);
	if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) throw Win32Exception();

	// Allocate a buffer large enough to hold the attribute data and initialize it
	HeapBuffer<uint8_t> buffer(required);
	PPROC_THREAD_ATTRIBUTE_LIST attributes = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(&buffer);
	if(!InitializeProcThreadAttributeList(attributes, 1, 0, &required)) throw Win32Exception();

	try {

		// Add the array of handles as inheritable by the client process
		if(!UpdateProcThreadAttribute(attributes, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, handles, count * sizeof(HANDLE), nullptr, nullptr)) throw Win32Exception();

		// Attempt to launch the process using the CREATE_SUSPENDED and EXTENDED_STARTUP_INFO_PRESENT flags
		// TODO: NEEDS SECURITY SETTINGS AND REMAINING OPTIONS
		zero_init<STARTUPINFOEX> startinfo;
		startinfo.StartupInfo.cb = sizeof(STARTUPINFOEX);
		startinfo.lpAttributeList = attributes;
		if(!CreateProcess(binarypath, commandline, nullptr, nullptr, TRUE, CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT,
			nullptr, nullptr, &startinfo.StartupInfo, &procinfo)) throw Win32Exception();

		DeleteProcThreadAttributeList(attributes);			// Clean up the PROC_THREAD_ATTRIBUTE_LIST
	}

	catch(...) { DeleteProcThreadAttributeList(attributes); throw; }

	// Process was successfully created and initialized, pass it off to a Host instance
	return std::make_unique<Host>(procinfo);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
