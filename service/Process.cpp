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
//	parent			- Parent Process instance
//	process			- Native process handle
//	processid		- Native process identifier
//	memory			- ProcessMemory virtual memory manager
//	ldt				- Address of allocated local descriptor table
//	ldtslots		- Local descriptor table allocation bitmap
//	programbreak	- Address of initial program break
//	mainthread		- Main/initial process thread instance
//	rootdir			- Initial process root directory
//	workingdir		- Initial process working directory

Process::Process(const std::shared_ptr<VirtualMachine>& vm, ::Architecture architecture, uapi::pid_t pid, const std::shared_ptr<Process>& parent, 
	const std::shared_ptr<NativeHandle>& process, DWORD processid, std::unique_ptr<ProcessMemory>&& memory, const void* ldt, Bitmap&& ldtslots, 
	const void* programbreak, const std::shared_ptr<::Thread>& mainthread, const std::shared_ptr<FileSystem::Alias>& rootdir, 
	const std::shared_ptr<FileSystem::Alias>& workingdir) : Process(vm, architecture, pid, parent, process, processid, std::move(memory), ldt, 
	std::move(ldtslots), programbreak, ProcessHandles::Create(), SignalActions::Create(), mainthread, rootdir, workingdir) {}

//-----------------------------------------------------------------------------
// Process Constructor
//
// Arguments:
//
//	vm				- Virtual machine instance
//	architecture	- Process architecture
//	pid				- Virtual process identifier
//	parent			- Parent Process instance
//	process			- Native process handle
//	processid		- Native process identifier
//	memory			- ProcessMemory virtual memory manager
//	ldt				- Address of allocated local descriptor table
//	ldtslots		- Local descriptor table allocation bitmap
//	programbreak	- Address of initial program break
//	handles			- Initial file system handles collection
//	sigactions		- Initial set of signal actions
//	mainthread		- Main/initial process thread instance
//	rootdir			- Initial process root directory
//	workingdir		- Initial process working directory

Process::Process(const std::shared_ptr<VirtualMachine>& vm, ::Architecture architecture, uapi::pid_t pid, const std::shared_ptr<Process>& parent, 
	const std::shared_ptr<NativeHandle>& process, DWORD processid, std::unique_ptr<ProcessMemory>&& memory, const void* ldt, Bitmap&& ldtslots, 
	const void* programbreak, const std::shared_ptr<ProcessHandles>& handles, const std::shared_ptr<SignalActions>& sigactions, 
	const std::shared_ptr<::Thread>& mainthread, const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir) 
	: m_vm(vm), m_architecture(architecture), m_pid(pid), m_parent(parent), m_process(process), m_processid(processid), m_memory(std::move(memory)), m_ldt(ldt), 
	m_ldtslots(std::move(ldtslots)), m_programbreak(programbreak), m_handles(handles), m_sigactions(sigactions), m_rootdir(rootdir), m_workingdir(workingdir)
{
	_ASSERTE(pid == mainthread->ThreadId);		// PID and main thread TID should match

	// The address of the thread entry point must be provided by the host process
	// when it acquires the RPC process/thread context handle
	m_threadproc = nullptr;

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
// Process::Clone
//
// Clones this process into a new child process
//
// Arguments:
//
//	flags		- Flags indicating the desired operations
//	task		- Task state for the new process

std::shared_ptr<Process> Process::Clone(int flags, std::unique_ptr<TaskState>&& task)
{
	// Allocate the PID for the cloned process
	uapi::pid_t pid = m_vm->AllocatePID();

	try {

		// Architecture::x86 --> 32 bit clone operation
		if(m_architecture == Architecture::x86) return Clone<Architecture::x86>(pid, flags, std::move(task));

#ifdef _M_X64
		// Architecture::x86_64 --> 64 bit clone operation
		else if(m_architecture == Architecture::x86_64) return Clone<Architecture::x86_64>(pid, flags, std::move(task));
#endif

		// Unsupported architecture
		throw LinuxException(LINUX_EPERM);
	}

	// Release the allocated PID upon exception
	catch(...) { m_vm->ReleasePID(pid); throw; }
}

//-----------------------------------------------------------------------------
// Process::Clone<Architecture> (private)
//
// Forks this process into a new child process
//
// Arguments:
//
//	pid			- PID to assign to the new child process
//	flags		- Flags indicating the desired operations
//	task		- Task state for the new process

template<::Architecture architecture>
std::shared_ptr<Process> Process::Clone(uapi::pid_t pid, int flags, std::unique_ptr<TaskState>&& task)
{
	// FLAGS TO DEAL WITH:
	//
	//CLONE_CHILD_CLEARTID
	//CLONE_CHILD_SETTID
	//CLONE_FS
	//CLONE_IO
	//CLONE_NEWIPC
	//CLONE_NEWNET
	//CLONE_NEWNS
	//CLONE_NEWPID
	//CLONE_NEWUSER
	//CLONE_NEWUTS
	//CLONE_PARENT_SETTID
	//CLONE_PTRACE
	//CLONE_SETTLS
	//CLONE_STOPPED
	//CLONE_SYSVSEM
	//CLONE_UNTRACED
	//CLONE_VFORK

	// CLONE_THREAD should not be specified when creating a new child process
	if(flags & LINUX_CLONE_THREAD) throw LinuxException(LINUX_EINVAL);

	// CLONE_VM is not currently supported when creating a new child process, it requires the two
	// processes to literally share memory, including allocations and releases. The section objects
	// can be shared, but each process would still control it's own ability to map/unmap them.
	if(flags & LINUX_CLONE_VM) throw LinuxException(LINUX_EINVAL);

	// Create a new host process for the current process architecture
	std::unique_ptr<NativeProcess> childhost = NativeProcess::Create<architecture>(m_vm);

	try {

		// Clone the current process memory as copy-on-write into the new process
		// todo: rename that to Duplicate() to match the rest
		std::unique_ptr<ProcessMemory> childmemory = ProcessMemory::FromProcessMemory(childhost->Process, m_memory, ProcessMemory::DuplicationMode::Clone);

		// Create the file system handle collection for the child process, which may be shared or duplicated (CLONE_FILES)
		std::shared_ptr<ProcessHandles> childhandles = (flags & LINUX_CLONE_FILES) ? m_handles : ProcessHandles::Duplicate(m_handles);

		// Create the signal actions collection for the child process, which may be shared or duplicated (CLONE_SIGHAND)
		std::shared_ptr<SignalActions> childsigactions = (flags & LINUX_CLONE_SIGHAND) ? m_sigactions : SignalActions::Duplicate(m_sigactions);

		// Determine the parent process for the child, which is either this process or this process' parent (CLONE_PARENT)
		std::shared_ptr<Process> childparent = (flags & LINUX_CLONE_PARENT) ? this->Parent : shared_from_this();

		// Create the main thread instance for the child process
		std::shared_ptr<::Thread> childthread = ::Thread::FromNativeHandle<architecture>(pid, childhost->Process, childhost->Thread, childhost->ThreadId, std::move(task));

		// Create and return the child Process instance
		return std::make_shared<Process>(m_vm, m_architecture, pid, childparent, childhost->Process, childhost->ProcessId, std::move(childmemory),
			m_ldt, Bitmap(m_ldtslots), m_programbreak, childhandles, childsigactions, childthread, m_rootdir, m_workingdir);
	}

	// Kill the native operating system process if any exceptions occurred during creation
	catch(...) { TerminateProcess(childhost->Process->Handle, static_cast<UINT>(E_UNEXPECTED)); throw; }
}

//-----------------------------------------------------------------------------
// Process::CreateThread
//
// Creates a new thread within the process instance
//
// Arguments:
//
//	flags		- Flags indicating the desired operations
//	task		- Task state for the new thread

std::shared_ptr<Thread> Process::CreateThread(int flags, const std::unique_ptr<TaskState>& task) const
{
	// CLONE_THREAD must have been specified in the flags for this operation
	if((flags & LINUX_CLONE_THREAD) == 0) throw LinuxException(LINUX_EINVAL);

	// CLONE_VM must have been specified in the flags for this operation
	if((flags & LINUX_CLONE_VM) == 0) throw LinuxException(LINUX_EINVAL);

	(task);
	// need a new stack or does pthreads create it, either way write the task there
	// but need to watch for overflow somehow

	// It would be easier to make a mask for all the valid/invalid flags, there aren't
	// many valid 'options' when creating a thread

	// FLAGS TO DEAL WITH:
	//
	//CLONE_CHILD_CLEARTID
	//CLONE_CHILD_SETTID
	//CLONE_FILES
	//CLONE_FS
	//CLONE_IO
	//CLONE_NEWIPC
	//CLONE_NEWNET
	//CLONE_NEWNS
	//CLONE_NEWPID
	//CLONE_NEWUSER
	//CLONE_NEWUTS
	//CLONE_PARENT
	//CLONE_PARENT_SETTID
	//CLONE_PTRACE
	//CLONE_SETTLS
	//CLONE_SIGHAND
	//CLONE_STOPPED
	//CLONE_SYSVSEM
	//CLONE_UNTRACED
	//CLONE_VFORK

	// TODO: IMPLEMENT
	return nullptr;
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
	//
	// TODO: THIS MAY BE BETTER SERVED AS A STATIC METHOD ON THREAD CLASS, THE PROCESS
	// DOESN'T REALLY HAVE A STACK, NOT ITS PROBLEM TO CREATE ONE
	//

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
// Process::Execute
//
// Replaces the process with a new executable image
//
// Arguments:
//
//	filename	- Name of the file system executable
//	argv		- Command-line arguments
//	envp		- Environment variables

void Process::Execute(const char_t* filename, const char_t* const* argv, const char_t* const* envp)
{
	// Parse the filename and command line arguments into an Executable instance
	auto executable = Executable::FromFile(m_vm, filename, argv, envp, m_rootdir, m_workingdir);

	Suspend();							// Suspend the entire native process

	try {

		// If the architecture of the executable is the same as this process, it can
		// be cleared out and replaced without spawning a new native process
		if(executable->Architecture == m_architecture) {

			// TODO
		}

		// Otherwise, the native process has to be replaced with a new one that
		// supports the target architecture
		else {

			// TODO
		}

		// (RE)START THE PROCESS WHEN DONE
	} 
	
	// Resume the unmodified process on exception so the system call will
	// return the appropriate error code back to the caller
	catch(...) { Resume(); throw; }
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

		// Allocate a new local descriptor table for the process and its slot tracking bitmap

		// TODO: this can be done in the constructor, but could throw, which *should* be ok.  Clone 
		// constructor will still need to accept the pointer and bitmap instance of course

		const void* ldt = nullptr;
		Bitmap ldtslots(LINUX_LDT_ENTRIES);
		try { ldt = memory->Allocate(LINUX_LDT_ENTRIES * sizeof(uapi::user_desc32), LINUX_PROT_READ | LINUX_PROT_WRITE); }
		catch(Exception& ex) { throw LinuxException(LINUX_ENOMEM, ex); }

		// Allocate the stack for the main process thread
		const void* stackpointer = nullptr;
		try { stackpointer = CreateThreadStack(vm, memory); }
		catch(Exception& ex) { throw LinuxException(LINUX_ENOMEM, ex); }

		// Load the executable image into the process address space and set up the thread stack
		Executable::LoadResult loaded = executable->Load(memory, stackpointer);

		// Create a Thread instance for the main thread of the new process, using a new TaskState
		std::shared_ptr<::Thread> mainthread = ::Thread::FromNativeHandle<architecture>(pid, host->Process, host->Thread, host->ThreadId,
			TaskState::Create<architecture>(loaded.EntryPoint, loaded.StackPointer));

		// Construct and return the new Process instance
		// TODO: PARENT?
		return std::make_shared<Process>(vm, architecture, pid, nullptr, host->Process, host->ProcessId, std::move(memory), ldt, std::move(ldtslots), 
			loaded.ProgramBreak, mainthread, executable->RootDirectory, executable->WorkingDirectory);
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
// Process::MapMemory
//
// Allocates/maps process virtual address space
//
// Arguments:
//
//	address		- Optional fixed address to use for the mapping
//	length		- Length of the requested mapping
//	prot		- Memory protection flags
//	flags		- Memory mapping flags, must include MAP_PRIVATE
//	fd			- Optional file descriptor from which to map
//	offset		- Optional offset into fd from which to map

const void* Process::MapMemory(const void* address, size_t length, int prot, int flags, int fd, uapi::loff_t offset)
{
	std::shared_ptr<FileSystem::Handle>	handle = nullptr;		// Handle to the file object

	// MAP_HUGETLB is not currently supported, but may be possible in the future
	if(flags & LINUX_MAP_HUGETLB) throw LinuxException(LINUX_EINVAL);

	// MAP_GROWSDOWN is not supported
	if(flags & LINUX_MAP_GROWSDOWN) throw LinuxException(LINUX_EINVAL);

	// Suggested base addresses are not supported, switch the address to NULL if MAP_FIXED is not set
	if((flags & LINUX_MAP_FIXED) == 0) address = nullptr;

	// Non-anonymous mappings require a valid file descriptor to be specified
	if(((flags & LINUX_MAP_ANONYMOUS) == 0) && (fd <= 0)) throw LinuxException(LINUX_EBADF);
	if(fd > 0) handle = m_handles->Get(fd)->Duplicate(LINUX_O_RDONLY);

	// Attempt to allocate the process memory and adjust address to that base if it was NULL
	const void* base = m_memory->Allocate(address, length, prot);
	if(address == nullptr) address = base;

	// If a file handle was specified, copy data from the file into the allocated region,
	// private mappings need not be concerned with writing this data back to the file
	if(handle != nullptr) {

		uintptr_t	dest = uintptr_t(address);			// Easier pointer math as uintptr_t
		size_t		read = 0;							// Data read from the file object

		// TODO: Both tempfs and hostfs should be able to handle memory mappings now with
		// the introduction of sections; this would be much more efficient that way.  Such
		// a call would need to fail for FS that can't support it, and fall back to this
		// type of implementation.  Also evaluate the same in ElfImage, mapping the source
		// file would be much better than having to read it from a stream

		HeapBuffer<uint8_t> buffer(SystemInformation::AllocationGranularity);	// 64K buffer

		// Seek the file handle to the specified offset
		if(handle->Seek(offset, LINUX_SEEK_SET) != offset) throw LinuxException(LINUX_EINVAL);

		do {
			
			// Copy the next chunk of bytes from the source file into the process address space
			read = handle->Read(buffer, min(length, buffer.Size));
			size_t written = m_memory->Write(reinterpret_cast<void*>(dest), buffer, read);

			dest += written;						// Increment destination pointer
			length -= read;							// Decrement remaining bytes to be copied

		} while((length > 0) && (read > 0));
	};

	// MAP_LOCKED - Attempt to lock the memory into the process working set.  This operation
	// will not throw an exception if it fails, its considered a hint in this implementation
	if(flags & LINUX_MAP_LOCKED) m_memory->Lock(address, length);

	return address;
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
// Process::getThread
//
// Gets a Thread instance from its native thread identifier

std::shared_ptr<Thread> Process::getNativeThread(DWORD tid)
{
	thread_map_lock_t::scoped_lock_read reader(m_threadslock);

	for(auto iterator : m_threads) if(iterator.second->NativeThreadId == tid) return iterator.second;
	throw Exception(E_FAIL);	// todo: Exception
}

//-----------------------------------------------------------------------------
// Process::getNativeThreadProc
//
// Gets the address of the thread entry point in the native process

const void* Process::getNativeThreadProc(void) const
{
	return m_threadproc;
}

//-----------------------------------------------------------------------------
// Process::putNativeThreadProc
//
// Sets the address of the thread entry point in the native process

void Process::putNativeThreadProc(const void* value)
{
	m_threadproc = value;
}

//-----------------------------------------------------------------------------
// Process::getParent
//
// Gets the parent of this process, or the root process if an orphan

std::shared_ptr<Process> Process::getParent(void) const
{
	// Try to get the parent instance, which may have disappeared orphaning
	// this process.  In that case, use the root/init process as the parent
	std::shared_ptr<Process> parent = m_parent.lock();
	return (parent) ? parent : m_vm->RootProcess;
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
// Process::Resume (private)
//
// Resumes the native operating system process from a suspended state
//
// Arguments:
//
//	NONE

void Process::Resume(void) const
{
	NTSTATUS result = NtApi::NtResumeProcess(m_process->Handle);
	if(result != 0) throw StructuredException(result);
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
// Process::SetLocalDescriptor
//
// Creates or updates an entry in the process local descriptor table
//
// Arguments:
//
//	u_info		- user_desc structure describing the LDT to be updated

void Process::SetLocalDescriptor(uapi::user_desc32* u_info)
{
	// Grab the requested slot number from the user_desc structure
	uint32_t slot = u_info->entry_number;

	ldt_lock_t::scoped_lock writer(m_ldtlock);

	if(slot == -1) {

		// Attempt to locate a free slot in the LDT allocation bitmap
		slot = m_ldtslots.FindClear();
		if(slot == Bitmap::NotFound) throw LinuxException(LINUX_ESRCH);

		// From the caller's perspective, the slot will have bit 13 (0x1000) set
		// to help ensure that a real LDT won't get accessed
		slot |= LINUX_LDT_ENTRIES;
	}

	// The slot number must fall between LINUX_LDT_ENTRIES and (LINUX_LDT_ENTRIES << 1)
	if((slot < LINUX_LDT_ENTRIES) || (slot >= (LINUX_LDT_ENTRIES << 1))) throw LinuxException(LINUX_EINVAL);

	// Attempt to update the entry in the process local descriptor table memory region
	uintptr_t destination = uintptr_t(m_ldt) + ((slot & ~LINUX_LDT_ENTRIES) * sizeof(uapi::user_desc32));
	m_memory->Write(reinterpret_cast<void*>(destination), u_info, sizeof(uapi::user_desc32));

	// Provide the allocated/updated slot number back to the caller in the user_desc structure
	u_info->entry_number = slot;

	// Track the slot allocation in the zero-based LDT allocation bitmap
	m_ldtslots.Set(slot & ~LINUX_LDT_ENTRIES);
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
// Process::SetSignalAction
//
// Adds or updates a signal action entry for the process
//
// Arguments:
//
//	signal		- Signal to be added or updated
//	action		- Specifies the new signal action structure
//	oldaction	- Optionally receives the previously set signal action structure

void Process::SetSignalAction(int signal, const uapi::sigaction* action, uapi::sigaction* oldaction)
{
	// SignalActions has a matching method for this operation
	m_sigactions->Set(signal, action, oldaction);
}

//-----------------------------------------------------------------------------
// Process::getSignalAction
//
// Gets the action set for a signal to the process

uapi::sigaction Process::getSignalAction(int signal) const
{
	// SignalActions has a matching method for this operation
	return m_sigactions->Get(signal);
}

//-----------------------------------------------------------------------------
// Process::putSignalAction
//
// Sets the action to be taken for a signal to the process

void Process::putSignalAction(int signal, uapi::sigaction action)
{
	// SignalActions has a matching method for this operation
	m_sigactions->Set(signal, &action, nullptr);
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
// Process::Start
//
// Starts a newly created process instance, provided to allow the creator to
// add it to any collections or tracking mechanisms before it begins execution
//
// Arguments:
//
//	NONE

void Process::Start(void) const
{
	// Just resume the process, it would have been created suspended
	Resume();
}

//-----------------------------------------------------------------------------
// Process::Suspend (private)
//
// Suspends the native operating system process
//
// Arguments:
//
//	NONE

void Process::Suspend(void) const
{
	NTSTATUS result = NtApi::NtSuspendProcess(m_process->Handle);
	if(result != 0) throw StructuredException(result);
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
// Process::UnmapMemory
//
// Releases process virtual address space
//
// Arguments:
//
//	address		- Base address of the memory to be released
//	length		- Length of the memory region to be released

void Process::UnmapMemory(const void* address, size_t length)
{
	// Releasing memory is taken care of by ProcessMemory
	m_memory->Release(address, length);
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
