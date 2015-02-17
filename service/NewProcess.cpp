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
#include "NewProcess.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Process Destructor

NewProcess::~NewProcess()
{
	m_vm->ReleasePID(m_pid);
	CloseHandle(m_nativehandle);
}

//-----------------------------------------------------------------------------
// Process::getArchitecture
//
// Gets the process architecture type

::Architecture NewProcess::getArchitecture(void) const
{
	return m_architecture;
}

//-----------------------------------------------------------------------------
// Process::FromExecutable<Architecture> (private, static)
//
// Creates a new process based on an Executable instance
//
// Arguments:
//
//	vm				- Parent virtual machine instance
//	pid				- Virtual process identifier to assign
//	executable		- Executable instance

template<::Architecture architecture>
std::shared_ptr<NewProcess> NewProcess::FromExecutable(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t pid, const std::unique_ptr<Executable>& executable)
{
	using elf = elf_traits<architecture>;

	std::unique_ptr<ElfImage>		binary;				// The main ELF binary image to be loaded
	std::unique_ptr<ElfImage>		interpreter;		// Optional interpreter image to be loaded

	//
	// CREATE NATIVE PROCESS HERE
	// PROCESS_INFORMATION processinfo = CreateNativeProcess<architecture>(vm);
	//

	// Create a virtual address space for the process; ProcessMemory does not take ownership of the handle
	std::shared_ptr<ProcessMemory> memory = ProcessMemory::Create(/* host */ nullptr);

	// I MAY WANT TO ROLL ELFIMAGE BACK INTO THIS CLASS AND DO SOMETHING LIKE THIS:
	// LoadBinary<architecture>(executable->Handle, memory);
	// LoadBinary<architecture>(interpreter, memory);
	// ALSO ELFARGUMENTS --> PROCESSARGUMENTS (perhaps shareable, perhaps not)

	// Load the binary and any top-level interpreter image that it specifies into the native process
	binary = ElfImage::Load<architecture>(executable->Handle, /* host */ nullptr);
	if(binary->Interpreter) interpreter = ElfImage::Load<architecture>(vm->OpenExecutable(executable->RootDirectory, executable->WorkingDirectory, binary->Interpreter), /* host */ nullptr);

	//
	// NEW PROCESS INITIALIZATION HERE
	//

	return std::make_shared<NewProcess>(vm, architecture, nullptr, 0, pid, executable->RootDirectory, executable->WorkingDirectory);
}
	
//-----------------------------------------------------------------------------
// Process::getProcessId
//
// Gets the virtual process identifier

uapi::pid_t NewProcess::getProcessId(void) const
{
	return m_pid;
}

//-----------------------------------------------------------------------------
// Process::getRootDirectory
//
// Gets the process root directory alias instance

std::shared_ptr<FileSystem::Alias> NewProcess::getRootDirectory(void) const
{
	return m_rootdir;
}

//-----------------------------------------------------------------------------
// Process::putRootDirectory
//
// Sets the process root directory alias instance

void NewProcess::putRootDirectory(const std::shared_ptr<FileSystem::Alias>& value)
{
	m_rootdir = value;
}

//-----------------------------------------------------------------------------
// Process::Spawn (static)
//
// Creates a new process instance from an executable in the file system
//
// Arguments:
//
//	vm			- Parent virtual machine instance
//	pid			- Virtual process identifier to assign
//	filename	- Name of the file system executable
//	argv		- Command-line arguments
//	envp		- Environment variables
//	rootdir		- Process root directory
//	workingdir	- Process working directory

std::shared_ptr<NewProcess> NewProcess::Spawn(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t pid, const char_t* filename, 
	const char_t* const* argv, const char_t* const* envp, const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir)
{
	// Parse the filename and command line arguments into an Executable instance
	auto executable = Executable::FromFile(vm, filename, argv, envp, rootdir, workingdir);

	// Use FromExecutable<> to spawn the process against the resolved binary file system object
	if(executable->Architecture == Architecture::x86) return FromExecutable<Architecture::x86>(vm, pid, executable);
#ifdef _M_X64
	else if(executable->Architecture == Architecture::x86_64) return FromExecutable<Architecture::x86_64>(vm, pid, executable);
#endif
	
	throw LinuxException(LINUX_ENOEXEC);
}

//-----------------------------------------------------------------------------
// Process::getWorkingDirectory
//
// Gets the process working directory alias instance

std::shared_ptr<FileSystem::Alias> NewProcess::getWorkingDirectory(void) const
{
	return m_workingdir;
}

//-----------------------------------------------------------------------------
// Process::putWorkingDirectory
//
// Sets the process working directory alias instance

void NewProcess::putWorkingDirectory(const std::shared_ptr<FileSystem::Alias>& value)
{
	m_workingdir = value;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
