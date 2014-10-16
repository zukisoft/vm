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
static uint8_t ANSI_SCRIPT_MAGIC[]		= { 0x23, 0x21, 0x20 };
static uint8_t UTF8_SCRIPT_MAGIC[]		= { 0xEF, 0xBB, 0xBF, 0x23, 0x21, 0x20 };
static uint8_t UTF16_SCRIPT_MAGIC[]		= { 0xFF, 0xFE, 0x23, 0x00, 0x21, 0x00, 0x20, 0x00 };

// Process::s_sysinfo
//
// Static SYSTEM_INFO information
Process::SystemInfo Process::s_sysinfo;

//-----------------------------------------------------------------------------
// Process::CheckHostProcessClass<x86> (static, private)
//
// Verifies that the created host process is 32-bit
//
// Arguments:
//
//	process		- Handle to the created host process

template <> inline void Process::CheckHostProcessClass<ElfClass::x86>(HANDLE process)
{
	BOOL			result;				// Result from IsWow64Process

	// 32-bit systems can only create 32-bit processes; nothing to worry about
	if(s_sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) return;

	// 64-bit system; verify that the process is running under WOW64
	if(!IsWow64Process(process, &result)) throw Win32Exception();
	if(!result) throw Exception(E_PROCESSINVALIDX86HOST);
}

//-----------------------------------------------------------------------------
// Process::CheckHostProcessClass<x86_64> (static, private)
//
// Verifies that the created host process is 64-bit
//
// Arguments:
//
//	process		- Handle to the created host process

#ifdef _M_X64
template <> inline void Process::CheckHostProcessClass<ElfClass::x86_64>(HANDLE process)
{
	BOOL				result;				// Result from IsWow64Process

	// 64-bit system; verify that the process is not running under WOW64
	if(!IsWow64Process(process, &result)) throw Win32Exception();
	if(result) throw Exception(E_PROCESSINVALIDX64HOST);
}
#endif

//-----------------------------------------------------------------------------
// Process::Create (static)
//
// Creates a new Process instance
//
// Arguments:
//
//	vm				- VirtualMachine instance
//	path			- Path to the file system object to execute as a process
//	arguments		- Pointer to an array of command line argument strings
//	environment		- Pointer to the process environment variables

std::shared_ptr<Process> Process::Create(std::shared_ptr<VirtualMachine> vm, const uapi::char_t* path,
	const uapi::char_t** arguments, const uapi::char_t** environment)
{
	if(!path) throw LinuxException(LINUX_EFAULT);

	// Attempt to open an execute handle for the specified path
	FileSystem::HandlePtr handle = vm->FileSystem->OpenExec(path);

	// Read in just enough from the head of the file to look for magic numbers
	MagicNumbers magics;
	size_t read = handle->Read(&magics, sizeof(MagicNumbers));
	handle->Seek(0, LINUX_SEEK_SET);

	// Check for an ELF binary image
	if((read >= sizeof(magics.ElfBinary)) && (memcmp(&magics.ElfBinary, LINUX_ELFMAG, LINUX_SELFMAG) == 0)) {

		switch(magics.ElfBinary[LINUX_EI_CLASS]) {

			// ELFCLASS32: Create a 32-bit host process for the binary
			case LINUX_ELFCLASS32: 
				return Create<ElfClass::x86>(vm, handle, arguments, environment, vm->Settings->Process.Host32.c_str(), vm->Listener32Binding);
#ifdef _M_X64
			// ELFCLASS64: Create a 64-bit host process for the binary
			case LINUX_ELFCLASS64: 
				return Create<ElfClass::x86_64>(vm, handle, arguments, environment, vm->Settings->Process.Host64.c_str(), vm->Listener64Binding);
#endif
			// Any other ELFCLASS -> ENOEXEC	
			default: throw LinuxException(LINUX_ENOEXEC);
		}
	}

	// Check for UTF-16 interpreter script
	else if((read >= sizeof(UTF16_SCRIPT_MAGIC)) && (memcmp(&magics.UTF16Script, &UTF16_SCRIPT_MAGIC, sizeof(UTF16_SCRIPT_MAGIC)) == 0)) {

		// TODO
		// parse binary and command line, recursively call back into Create()
		throw Exception(E_NOTIMPL);
	}

	// Check for UTF-8 interpreter script
	else if((read >= sizeof(UTF8_SCRIPT_MAGIC)) && (memcmp(&magics.UTF8Script, &UTF8_SCRIPT_MAGIC, sizeof(UTF8_SCRIPT_MAGIC)) == 0)) {

		// TODO
		// parse binary and command line, recursively call back into Create()
		throw Exception(E_NOTIMPL);
	}

	// Check for ANSI interpreter script
	else if((read >= sizeof(ANSI_SCRIPT_MAGIC)) && (memcmp(&magics.AnsiScript, &ANSI_SCRIPT_MAGIC, sizeof(ANSI_SCRIPT_MAGIC)) == 0)) {

		// TODO
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

template <ElfClass _class>
static std::shared_ptr<Process> Process::Create(const std::shared_ptr<VirtualMachine>& vm, const FileSystem::HandlePtr& handle,
	const uapi::char_t** argv, const uapi::char_t** envp, const tchar_t* hostpath, const tchar_t* hostargs)
{
	using elf = elf_traits<_class>;

	std::unique_ptr<ElfImage>		executable;				// The main ELF binary image to be loaded
	std::unique_ptr<ElfImage>		interpreter;			// Optional interpreter image specified by executable
	uint8_t							random[16];				// 16-bytes of random data for AT_RANDOM auxvec
	StartupInfo						startinfo;				// Hosted process startup information

	// Create the external host process (suspended by default) and verify the class/architecture
	// as this will all go south very quickly if it's not the expected architecture
	// todo: need the handles to stuff that are to be inherited (signals, etc)
	std::unique_ptr<Host> host = Host::Create(hostpath, hostargs, nullptr, 0);
	CheckHostProcessClass<_class>(host->ProcessHandle);

	try {

		// Generate the AT_RANDOM data to be associated with this process
		Random::Generate(random, 16);

		// Attempt to load the binary image into the process, then check for an interpreter
		executable = ElfImage::Load<_class>(handle, host->ProcessHandle);
		if(executable->Interpreter) {

			// Acquire a handle to the interpreter binary and attempt to load that into the process
			FileSystem::HandlePtr interphandle = vm->FileSystem->OpenExec(executable->Interpreter);
			interpreter = ElfImage::Load<_class>(interphandle, host->ProcessHandle);
		}

		// Construct the ELF arguments stack image for the hosted process
		ElfArguments args(argv, envp);

		(LINUX_AT_EXECFD);																		//  2 - TODO
		if(executable->ProgramHeaders) {

			args.AppendAuxiliaryVector(LINUX_AT_PHDR, executable->ProgramHeaders);				//  3
			args.AppendAuxiliaryVector(LINUX_AT_PHENT, sizeof(typename elf::progheader_t));		//  4
			args.AppendAuxiliaryVector(LINUX_AT_PHNUM, executable->NumProgramHeaders);			//  5
		}

		args.AppendAuxiliaryVector(LINUX_AT_PAGESZ, MemoryRegion::PageSize);					//  6
		if(interpreter) args.AppendAuxiliaryVector(LINUX_AT_BASE, interpreter->BaseAddress);	//  7
		args.AppendAuxiliaryVector(LINUX_AT_FLAGS, 0);											//  8
		args.AppendAuxiliaryVector(LINUX_AT_ENTRY, executable->EntryPoint);						//  9
		(LINUX_AT_NOTELF);																		// 10 - NOT IMPLEMENTED
		(LINUX_AT_UID);																			// 11 - TODO
		(LINUX_AT_EUID);																		// 12 - TODO
		(LINUX_AT_GID);																			// 13 - TODO
		(LINUX_AT_EGID);																		// 14 - TODO
		args.AppendAuxiliaryVector(LINUX_AT_PLATFORM, elf::platform);							// 15
		(LINUX_AT_HWCAP);																		// 16 - TODO
		(LINUX_AT_CLKTCK);																		// 17 - TODO
		args.AppendAuxiliaryVector(LINUX_AT_SECURE, 0);											// 23
		(LINUX_AT_BASE_PLATFORM);																// 24 - NOT IMPLEMENTED
		args.AppendAuxiliaryVector(LINUX_AT_RANDOM, random, 16);								// 25
		(LINUX_AT_HWCAP2);																		// 26 - TODO
		(LINUX_AT_EXECFN);																		// 31 - TODO WHICH PATH? ARGUMENT TO EXECVE() OR THE ACTUAL PATH?
		(LINUX_AT_SYSINFO);																		// 32 - TODO MAY NOT IMPLEMENT?
		//args.AppendAuxiliaryVector(LINUX_AT_SYSINFO_EHDR, vdso->BaseAddress);					// 33 - TODO NEEDS VDSO

		// Generate the stack image for the process in it's address space
		ElfArguments::StackImage stackimg = args.GenerateStackImage<_class>(host->ProcessHandle);

		// Load the StartupInfo structure with the necessary information to get the ELF binary running
		startinfo.EntryPoint = (interpreter) ? interpreter->EntryPoint : executable->EntryPoint;
		startinfo.ProgramBreak = executable->ProgramBreak;
		startinfo.StackImage = stackimg.BaseAddress;
		startinfo.StackImageLength = stackimg.Length;

		// Create the Process object, transferring the host and startup information
		return std::make_shared<Process>(std::move(host), std::move(startinfo));
	}

	// Terminate the host process on exception since it doesn't get killed by the Host destructor
	// TODO: should be LinuxException wrapping the underlying one?  Only case right now where
	// that wouldn't be true is interpreter's OpenExec() call which should already throw LinuxExceptions
	catch(...) { host->Terminate(E_FAIL); throw; }
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
