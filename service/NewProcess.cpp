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
// Process Constructor
//
// Arguments:
//
// TODO: They will change

NewProcess::NewProcess(const std::shared_ptr<VirtualMachine>& vm, ::Architecture architecture, const std::shared_ptr<NativeHandle>& process, 
	uapi::pid_t pid, const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir,
	std::unique_ptr<ProcessMemory>&& memory) :
	m_vm(vm), m_architecture(architecture), m_process(process), m_pid(pid), m_rootdir(rootdir), m_workingdir(workingdir), m_memory(std::move(memory))
{
}

//-----------------------------------------------------------------------------
// Process Destructor

NewProcess::~NewProcess()
{
	m_vm->ReleasePID(m_pid);
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
// Process::CreateHostProcess<Architecture::x86> (private, static)
//
// Creates a 32-bit x86 native operating system host process
//
// Arguments:
//
//	vm			- Reference to the parent VirtualMachine instance

template<>
PROCESS_INFORMATION NewProcess::CreateHostProcess<Architecture::x86>(const std::shared_ptr<VirtualMachine>& vm)
{
	PROCESS_INFORMATION				procinfo;			// Process information

	// Get the configured path to the host binary as well as the standard command-line arguments
	const std::tstring path = vm->GetProperty(VirtualMachine::Properties::HostProcessBinary32);
	const std::tstring arguments = vm->GetProperty(VirtualMachine::Properties::HostProcessArguments);

	// Generate the command line for the child process, using the specifed path as argument zero
	tchar_t commandline[MAX_PATH];
	_sntprintf_s(commandline, MAX_PATH, MAX_PATH, _T("\"%s\"%s"), path.c_str(), arguments.c_str());

	// Generate the STARTUPINFO for the new native process
	zero_init<STARTUPINFO> startinfo;
	startinfo.cb = sizeof(STARTUPINFO);

	// Attempt to launch the process using the CREATE_SUSPENDED flag
	if(!CreateProcess(path.c_str(), commandline, nullptr, nullptr, TRUE, CREATE_SUSPENDED, nullptr, nullptr, &startinfo, &procinfo))
		throw LinuxException(LINUX_EPERM, Win32Exception());

	return procinfo;					// Return the process information
}

//-----------------------------------------------------------------------------
// Process::CreateHostProcess<Architecture::x86_64> (private, static)
//
// Creates a 32-bit x86 native operating system host process
//
// Arguments:
//
//	vm			- Reference to the parent VirtualMachine instance
#ifdef _M_X64
template<>
PROCESS_INFORMATION NewProcess::CreateHostProcess<Architecture::x86_64>(const std::shared_ptr<VirtualMachine>& vm)
{
	PROCESS_INFORMATION				procinfo;			// Process information

	// Get the configured path to the host binary as well as the standard command-line arguments
	const std::tstring path = vm->GetProperty(VirtualMachine::Properties::HostProcessBinary64);
	const std::tstring arguments = vm->GetProperty(VirtualMachine::Properties::HostProcessArguments);

	// Generate the command line for the child process, using the specifed path as argument zero
	tchar_t commandline[MAX_PATH];
	_sntprintf_s(commandline, MAX_PATH, MAX_PATH, _T("\"%s\"%s"), path.c_str(), arguments.c_str());

	// Generate the STARTUPINFO for the new native process
	zero_init<STARTUPINFO> startinfo;
	startinfo.cb = sizeof(STARTUPINFO);

	// Attempt to launch the process using the CREATE_SUSPENDED flag
	if(!CreateProcess(path.c_str(), commandline, nullptr, nullptr, TRUE, CREATE_SUSPENDED, nullptr, nullptr, &startinfo, &procinfo))
		throw LinuxException(LINUX_EPERM, Win32Exception());

	return procinfo;					// Return the process information
}
#endif
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

	PROCESS_INFORMATION				hostprocinfo;		// Native host process information
	std::unique_ptr<ElfImage>		binary;				// The main ELF binary image to be loaded
	std::unique_ptr<ElfImage>		interpreter;		// Optional interpreter image to be loaded

	// Create the native operating system host process for the specified architecture and wrap
	// the handles into shared pointers for passing around
	hostprocinfo = CreateHostProcess<architecture>(vm);
	std::shared_ptr<NativeHandle> hostprocess = NativeHandle::FromHandle(hostprocinfo.hProcess);
	std::shared_ptr<NativeHandle> hostthread = NativeHandle::FromHandle(hostprocinfo.hThread);

	try {

		// Create a virtual address space for the process; ProcessMemory does not take ownership of the handle
		std::unique_ptr<ProcessMemory> memory = ProcessMemory::Create(hostprocess);

		// Wrap the main process thread in a Thread instance
		std::shared_ptr<Thread> thread = Thread::FromHandle<architecture>(hostprocess->Handle, pid, hostthread->Handle, hostprocinfo.dwThreadId);

		// I MAY WANT TO ROLL ELFIMAGE BACK INTO THIS CLASS AND DO SOMETHING LIKE THIS:
		// LoadBinary<architecture>(executable->Handle, memory);
		// LoadBinary<architecture>(interpreter, memory);
		// ALSO ELFARGUMENTS --> PROCESSARGUMENTS (perhaps shareable, perhaps not)

		// Load the binary and any top-level interpreter image that it specifies into the native process
		// TODO: should these take ProcessMemory instead? I think yes
		binary = ElfImage::Load<architecture>(executable->Handle, /* host */ nullptr);
		if(binary->Interpreter) interpreter = ElfImage::Load<architecture>(vm->OpenExecutable(executable->RootDirectory, executable->WorkingDirectory, binary->Interpreter), /* host */ nullptr);

		//
		// NEW PROCESS INITIALIZATION HERE
		//

		return std::make_shared<NewProcess>(vm, architecture, hostprocess, pid, executable->RootDirectory, executable->WorkingDirectory, std::move(memory));
	}

	catch(...) { /* terminate native process and close handles */ throw; }
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
