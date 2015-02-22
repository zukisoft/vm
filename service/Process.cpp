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
#include "Process.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Process Constructor
//
// Arguments:
//
//	vm				- Virtual machine instance
//	architecture	- Process architecture
//	pid				- Virtual process identifier
//	process			- Native process handle
//	processid		- Native process identifier
//	memory			- ProcessMemory virtual memory manager
//	ldt				- Address of allocated local descriptor table
//	programbreak	- Address of initial program break
//	mainthread		- Main/initial process thread instance
//	rootdir			- Initial process root directory
//	workingdir		- Initial process working directory

Process::Process(const std::shared_ptr<VirtualMachine>& vm, ::Architecture architecture, uapi::pid_t pid, const std::shared_ptr<NativeHandle>& process,
	DWORD processid, std::unique_ptr<ProcessMemory>&& memory, const void* ldt, const void* programbreak, const std::shared_ptr<::Thread>& mainthread,
	const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir) : Process(vm, architecture, pid, process, 
	processid, std::move(memory), ldt, programbreak, ProcessHandles::Create(), SignalActions::Create(), mainthread, rootdir, workingdir) {}

//-----------------------------------------------------------------------------
// Process Constructor
//
// Arguments:
//
//	vm				- Virtual machine instance
//	architecture	- Process architecture
//	pid				- Virtual process identifier
//	process			- Native process handle
//	processid		- Native process identifier
//	memory			- ProcessMemory virtual memory manager
//	ldt				- Address of allocated local descriptor table
//	programbreak	- Address of initial program break
//	handles			- Initial file system handles collection
//	sigactions		- Initial set of signal actions
//	mainthread		- Main/initial process thread instance
//	rootdir			- Initial process root directory
//	workingdir		- Initial process working directory

Process::Process(const std::shared_ptr<VirtualMachine>& vm, ::Architecture architecture, uapi::pid_t pid, const std::shared_ptr<NativeHandle>& process,
	DWORD processid, std::unique_ptr<ProcessMemory>&& memory, const void* ldt, const void* programbreak, const std::shared_ptr<ProcessHandles>& handles, 
	const std::shared_ptr<SignalActions>& sigactions, const std::shared_ptr<::Thread>& mainthread, const std::shared_ptr<FileSystem::Alias>& rootdir, 
	const std::shared_ptr<FileSystem::Alias>& workingdir) : m_vm(vm), m_architecture(architecture), m_pid(pid), m_process(process), m_processid(processid),
	m_memory(std::move(memory)), m_ldt(ldt), m_programbreak(programbreak), m_handles(handles), m_sigactions(sigactions), m_rootdir(rootdir), m_workingdir(workingdir)
{
	_ASSERTE(pid == mainthread->ThreadId);		// PID and main thread TID should match

	// Initialize the threads collection with just the main thread instance
	m_threads[mainthread->ThreadId] = std::move(mainthread);
}

//-----------------------------------------------------------------------------
// Process Destructor

Process::~Process()
{
	// TODO: clear out the threads first?
	m_vm->ReleasePID(m_pid);
}

//-----------------------------------------------------------------------------
// Process::AddHandle
//
// Adds a file descriptor to the process
//
// Arguments:
//
//	handle		- File system handle instance to be added

int Process::AddHandle(const std::shared_ptr<FileSystem::Handle>& handle)
{
	// Allow the collection to determine the file descriptor index
	return m_handles->Add(handle);
}

//-----------------------------------------------------------------------------
// Process::AddHandle
//
// Adds a file descriptor with a specific index to the process
//
// Arguments:
//
//	fd			- File descriptor index
//	handle		- File system handle instance to be added

int Process::AddHandle(int fd, const std::shared_ptr<FileSystem::Handle>& handle)
{
	// Attempt to add the handle with the specified file descriptor index
	return m_handles->Add(fd, handle);
}

//-----------------------------------------------------------------------------
// Process::getArchitecture
//
// Gets the process architecture type

::Architecture Process::getArchitecture(void) const
{
	return m_architecture;
}

//-----------------------------------------------------------------------------
// Process::CreateThreadStack (private, static)
//
// Creates the stack for a new thread
//
// Arguments:
//
//	vm			- Parent VirtualMachine instance
//	memory		- ProcessMemory instance to use for allocation

const void* Process::CreateThreadStack(const std::shared_ptr<VirtualMachine>& vm, const std::unique_ptr<ProcessMemory>& memory)
{
	// Get the default thread stack size from the VirtualMachine instance and convert it 
	size_t stacklen;
	try { stacklen = std::stoul(vm->GetProperty(VirtualMachine::Properties::ThreadStackSize)); }
	catch(...) { throw Exception(E_PROCESSINVALIDSTACKSIZE); }
		
	// Allocate the stack memory and calculate the stack pointer, which will be reduced by the length of a guard page
	const void* stack = memory->Allocate(stacklen, LINUX_PROT_READ | LINUX_PROT_WRITE);
	const void* stackpointer = reinterpret_cast<void*>(uintptr_t(stack) + stacklen - SystemInformation::PageSize);

	// Install read/write guard pages at both ends of the stack memory region
	memory->Guard(stack, SystemInformation::PageSize, LINUX_PROT_READ | LINUX_PROT_WRITE);
	memory->Guard(stackpointer, SystemInformation::PageSize, LINUX_PROT_READ | LINUX_PROT_WRITE);

	return stackpointer;			// Return the stack pointer address
}

//-----------------------------------------------------------------------------
// Process::getFileCreationModeMask
//
// Gets the process umask value

uapi::mode_t Process::getFileCreationModeMask(void) const
{
	return m_umask;
}

//-----------------------------------------------------------------------------
// Process::putFileCreationModeMask
//
// Sets the process umask value

void Process::putFileCreationModeMask(uapi::mode_t value)
{
	m_umask = (value & LINUX_S_IRWXUGO);
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
std::shared_ptr<Process> Process::FromExecutable(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t pid, const std::unique_ptr<Executable>& executable)
{
	// Create a host process for the specified architecture
	std::unique_ptr<NativeProcess> host = NativeProcess::Create<architecture>(vm);

	try {

		// Create a new virtual address space for the process
		std::unique_ptr<ProcessMemory> memory = ProcessMemory::Create(host->Process);

		// Allocate a new local descriptor table for the process
		const void* ldt = nullptr;
		try { ldt = memory->Allocate(LINUX_LDT_ENTRIES * sizeof(uapi::user_desc32), LINUX_PROT_READ | LINUX_PROT_WRITE); }
		catch(Exception& ex) { throw LinuxException(LINUX_ENOMEM, ex); }

		// Allocate the stack for the main process thread
		const void* stackpointer = nullptr;
		try { stackpointer = CreateThreadStack(vm, memory); }
		catch(Exception& ex) { throw LinuxException(LINUX_ENOMEM, ex); }

		// Load the executable image into the process address space and set up the thread stack
		Executable::LoadResult loaded = executable->Load(memory, stackpointer);

		// Create the task state for the main thread based on the load result
		std::unique_ptr<TaskState> ts = TaskState::Create(architecture, loaded.EntryPoint, loaded.StackPointer);
		
		// Wrap the main process thread in a Thread instance
		// needs entry point and stack pointer, aka the task
		std::shared_ptr<Thread> mainthread = Thread::FromNativeHandle<architecture>(pid, host->Process, host->Thread, host->ThreadId);

		// Construct and return the new Process instance
		return std::make_shared<Process>(vm, architecture, pid, host->Process, host->ProcessId, std::move(memory), ldt, loaded.ProgramBreak,
			mainthread, executable->RootDirectory, executable->WorkingDirectory);
	}

	catch(...) { /* TODO terminate native process; don't close handles */ throw; }
}
	
//-----------------------------------------------------------------------------
// Process::getHandle
//
// Gets a file system handle from its file descriptor index

std::shared_ptr<FileSystem::Handle> Process::getHandle(int fd) const
{
	return m_handles->operator[](fd);
}

//-----------------------------------------------------------------------------
// Process::getLocalDescriptorTable
//
// Gets the address of the process local descriptor table

const void* Process::getLocalDescriptorTable(void) const
{
	return m_ldt;
}

//-----------------------------------------------------------------------------
// Process::getNativeProcessId
//
// Gets the native operating system process identifier

DWORD Process::getNativeProcessId(void) const
{
	return m_processid;
}
	
//-----------------------------------------------------------------------------
// Process::getProcessId
//
// Gets the virtual process identifier

uapi::pid_t Process::getProcessId(void) const
{
	return m_pid;
}

//-----------------------------------------------------------------------------
// Process::getProgramBreak
//
// Gets the currently set program break address

const void* Process::getProgramBreak(void) const
{
	return m_programbreak;
}

//----------------------------------------------------------------------------
// Process::ProtectMemory
//
// Sets memory protection flags for a virtual address space region
//
// Arguments:
//
//	address		- Base address of the region to protect
//	length		- Length of the region to protect
//	prot		- Linux protection flags to apply to the region

void Process::ProtectMemory(const void* address, size_t length, int prot) const
{ 
	m_memory->Protect(address, length, prot);
}

//-----------------------------------------------------------------------------
// Process::ReadMemory
//
// Reads data from the process virtual address space
//
// Arguments:
//
//	address		- Virtual address to read the data from
//	buffer		- Local address to write the data into
//	length		- Number of bytes to read/write

size_t Process::ReadMemory(const void* address, void* buffer, size_t length) const
{
	// Use ProcessMemory to read from the virtualized address space
	return m_memory->Read(address, buffer, length);
}

//-----------------------------------------------------------------------------
// Process::RemoveHandle
//
// Removes a file system handle from the process
//
// Arguments:
//
//	fd			- File descriptor index

void Process::RemoveHandle(int fd)
{
	// Attempt to remove the specified file descriptor from the process
	m_handles->Remove(fd);
}

//-----------------------------------------------------------------------------
// Process::getRootDirectory
//
// Gets the process root directory alias instance

std::shared_ptr<FileSystem::Alias> Process::getRootDirectory(void) const
{
	return m_rootdir;
}

//-----------------------------------------------------------------------------
// Process::putRootDirectory
//
// Sets the process root directory alias instance

void Process::putRootDirectory(const std::shared_ptr<FileSystem::Alias>& value)
{
	m_rootdir = value;
}

//-----------------------------------------------------------------------------
// Process::SetProgramBreak
//
// Adjusts the program break address by increasing or decreasing the number
// of allocated pages immediately following the loaded binary image
//
// Arguments:
//
//	address		- Requested program break address

const void* Process::SetProgramBreak(const void* address)
{
	// NULL can be passed in as the address to retrieve the current program break
	if(address == nullptr) return m_programbreak;

	// The real break address must be aligned to a page boundary
	const void* oldbreak = align::up(m_programbreak, SystemInformation::PageSize);
	const void* newbreak = align::up(address, SystemInformation::PageSize);

	// If the aligned addresses are not the same, the actual break must be changed
	if(oldbreak != newbreak) {

		// Calculate the delta between the current and requested address
		intptr_t delta = intptr_t(newbreak) - intptr_t(oldbreak);

		try {

			// Allocate or release program break address space based on the calculated delta.
			if(delta > 0) m_memory->Allocate(oldbreak, delta, LINUX_PROT_READ | LINUX_PROT_WRITE);
			else m_memory->Release(newbreak, delta);
		}

		// Return the previously set program break address if it could not be adjusted,
		// this operation is not intended to return any error codes
		catch(...) { return m_programbreak; }
	}

	// Store and return the requested address, not the page-aligned address
	m_programbreak = address;
	return m_programbreak;
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

std::shared_ptr<Process> Process::Spawn(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t pid, const char_t* filename, 
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
// Process::getThread
//
// Gets a Thread instance from its virtual thread identifier

std::shared_ptr<Thread> Process::getThread(uapi::pid_t tid)
{
	thread_map_lock_t::scoped_lock_read reader(m_threadslock);

	// Attempt to locate the thread within the collection
	const auto found = m_threads.find(tid);
	return (found != m_threads.end()) ? found->second : nullptr;
}

//-----------------------------------------------------------------------------
// Process::getWorkingDirectory
//
// Gets the process working directory alias instance

std::shared_ptr<FileSystem::Alias> Process::getWorkingDirectory(void) const
{
	return m_workingdir;
}

//-----------------------------------------------------------------------------
// Process::putWorkingDirectory
//
// Sets the process working directory alias instance

void Process::putWorkingDirectory(const std::shared_ptr<FileSystem::Alias>& value)
{
	m_workingdir = value;
}

//-----------------------------------------------------------------------------
// Process::WriteMemory
//
// Writes data into the process virtual address space
//
// Arguments:
//
//	address		- Virtual address to write the data into
//	buffer		- Local address to read the data from
//	length		- Number of bytes to write/read

size_t Process::WriteMemory(const void* address, const void* buffer, size_t length) const
{
	// Use ProcessMemory to write into the virtualized address space
	return m_memory->Write(address, buffer, length);
}

//-----------------------------------------------------------------------------
// Process::getZombie
//
// Gets a flag indicating if this process is a zombie or not, which means the
// actual process has terminated but this class instance still exists

bool Process::getZombie(void) const
{
	// If the native process has terminated, this process is a zombie
	return (WaitForSingleObject(m_process->Handle, 0) == WAIT_OBJECT_0);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
