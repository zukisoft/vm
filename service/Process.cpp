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
//	termsignal		- Signal to send to parent on termination
//	rootdir			- Initial process root directory
//	workingdir		- Initial process working directory

Process::Process(const std::shared_ptr<VirtualMachine>& vm, ::Architecture architecture, uapi::pid_t pid, const std::shared_ptr<Process>& parent, 
	const std::shared_ptr<::NativeHandle>& process, DWORD processid, std::unique_ptr<ProcessMemory>&& memory, const void* ldt, Bitmap&& ldtslots, 
	const void* programbreak, const std::shared_ptr<::Thread>& mainthread, int termsignal, const std::shared_ptr<FileSystem::Alias>& rootdir, 
	const std::shared_ptr<FileSystem::Alias>& workingdir) : Process(vm, architecture, pid, parent, process, processid, std::move(memory), ldt, 
	std::move(ldtslots), programbreak, ProcessHandles::Create(), SignalActions::Create(), mainthread, termsignal, rootdir, workingdir) {}

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
//	termsignal		- Signal to send to parent on termination
//	rootdir			- Initial process root directory
//	workingdir		- Initial process working directory

Process::Process(const std::shared_ptr<VirtualMachine>& vm, ::Architecture architecture, uapi::pid_t pid, const std::shared_ptr<Process>& parent, 
	const std::shared_ptr<::NativeHandle>& process, DWORD processid, std::unique_ptr<ProcessMemory>&& memory, const void* ldt, Bitmap&& ldtslots, 
	const void* programbreak, const std::shared_ptr<ProcessHandles>& handles, const std::shared_ptr<SignalActions>& sigactions, 
	const std::shared_ptr<::Thread>& mainthread, int termsignal, const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir) 
	: m_vm(vm), m_architecture(architecture), m_pid(pid), m_parent(parent), m_process(process), m_processid(processid), m_statuscode(0), m_memory(std::move(memory)), 
	m_ldt(ldt), m_ldtslots(std::move(ldtslots)), m_programbreak(programbreak), m_handles(handles), m_sigactions(sigactions), m_termsignal(termsignal), 
	m_rootdir(rootdir), m_workingdir(workingdir)
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
	// Release all held references to child Thread instances
	m_threads.clear();

	// Release the PID
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
	//CLONE_FS
	//CLONE_IO
	//CLONE_NEWIPC
	//CLONE_NEWNET
	//CLONE_NEWNS
	//CLONE_NEWPID
	//CLONE_NEWUSER
	//CLONE_NEWUTS
	//CLONE_PTRACE
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

		// Clone the current process memory into the new process
		std::unique_ptr<ProcessMemory> childmemory = m_memory->Clone(childhost->Process);

		// Create the file system handle collection for the child process, which may be shared or duplicated (CLONE_FILES)
		std::shared_ptr<ProcessHandles> childhandles = (flags & LINUX_CLONE_FILES) ? m_handles : ProcessHandles::Duplicate(m_handles);

		// Create the signal actions collection for the child process, which may be shared or duplicated (CLONE_SIGHAND)
		std::shared_ptr<SignalActions> childsigactions = (flags & LINUX_CLONE_SIGHAND) ? m_sigactions : SignalActions::Duplicate(m_sigactions);

		// Determine the parent process for the child, which is either this process or this process' parent (CLONE_PARENT)
		std::shared_ptr<Process> childparent = (flags & LINUX_CLONE_PARENT) ? this->Parent : shared_from_this();

		// Create the main thread instance for the child process
		std::shared_ptr<::Thread> childthread = ::Thread::FromNativeHandle<architecture>(pid, childhost->Process, childhost->Thread, childhost->ThreadId, std::move(task));

		// Determine the child termination signal from the low byte of the flags
		int childtermsig = (flags & 0xFF);

		// Construct and insert a new Process instance to the child collection
		process_map_lock_t::scoped_lock writer(m_childlock);
		auto emplaced = m_children.emplace(pid, std::make_shared<Process>(m_vm, m_architecture, pid, childparent, childhost->Process, childhost->ProcessId,
			std::move(childmemory), m_ldt, Bitmap(m_ldtslots), m_programbreak, childhandles, childsigactions, childthread, childtermsig, m_rootdir, m_workingdir));

		// Verify that the new Process instance was added to the collection and return the shared_ptr
		if(!emplaced.second) throw Exception(E_PROCESSDUPLICATEPID, pid, m_pid);
		return emplaced.first->second;
	}

	// Kill the native operating system process if any exceptions occurred during creation
	catch(...) { childhost->Terminate(LINUX_SIGKILL); throw; }
}

//-----------------------------------------------------------------------------
// Process::CreateStack (private, static)
//
// Allocates a new stack in the process
//
// Arguments:
//
//	vm			- Parent VirtualMachine instance
//	memory		- ProcessMemory instance to use for allocation

const void* Process::CreateStack(const std::shared_ptr<VirtualMachine>& vm, const std::unique_ptr<ProcessMemory>& memory)
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

	// CLONE_SIGHAND must have been specified in the flags for this operation
	if((flags & LINUX_CLONE_SIGHAND) == 0) throw LinuxException(LINUX_EINVAL);

	// CLONE_VM must have been specified in the flags for this operation
	if((flags & LINUX_CLONE_VM) == 0) throw LinuxException(LINUX_EINVAL);

	(task);
	// need a new stack or does pthreads create it, either way write the task there
	// but need to watch for overflow somehow

	// It would be easier to make a mask for all the valid/invalid flags, there aren't
	// many valid 'options' when creating a thread

	// FLAGS TO DEAL WITH:
	//
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
	//CLONE_PTRACE
	//CLONE_SYSVSEM
	//CLONE_UNTRACED
	//CLONE_VFORK

	// TODO: IMPLEMENT
	return nullptr;
}

//-----------------------------------------------------------------------------
// Process::Execute
//
// Replaces the process with a new executable image
//
// Arguments:
//
//	filename		- Name of the file system executable
//	argv			- Command-line arguments
//	envp			- Environment variables

void Process::Execute(const char_t* filename, const char_t* const* argv, const char_t* const* envp)
{
	// Parse the filename and command line arguments into an Executable instance
	auto executable = Executable::FromFile(m_vm, filename, argv, envp, m_rootdir, m_workingdir);

	// Architecture::x86 --> 32-bit executable
	if(executable->Architecture == ::Architecture::x86) Execute<Architecture::x86>(executable);

	// Architecture::x86_64 --> 64-bit executable
#ifdef _M_X64
	else if(executable->Architecture == ::Architecture::x86_64) Execute<Architecture::x86_64>(executable);
#endif
	
	// Unsupported architecture
	else throw LinuxException(LINUX_EPERM);
}

//-----------------------------------------------------------------------------
// Process::Execute<Architecture> (private)
//
// Replaces the process with a new executable image
//
// Arguments:
//
//	filename		- Name of the file system executable
//	argv			- Command-line arguments
//	envp			- Environment variables

template<::Architecture architecture>
void Process::Execute(const std::unique_ptr<Executable>& executable)
{
	DWORD				threadid;				// New thread identifier

	// Suspend the process without signaling the parent
	SuspendInternal();

	// Create a new thread in the process using the address provided at process registration.  Use the minimum
	// allowed thread stack size, it will be overcome by the stack allocated below
	auto thread = ::NativeHandle::FromHandle(CreateRemoteThread(m_process->Handle, nullptr, SystemInformation::AllocationGranularity, 
		reinterpret_cast<LPTHREAD_START_ROUTINE>(m_threadproc), nullptr, CREATE_SUSPENDED, &threadid));

	try {

		// Take an exclusive lock against the threads collection
		thread_map_lock_t::scoped_lock writer(m_threadslock);

		// Terminate all existing threads and clear out the local collection
		for(auto iterator : m_threads) iterator.second->Terminate(0);
		m_threads.clear();

		try {
		
			// TODO (+ more, see execve(2))
			// Architecture
			// Termination Signal -> SIGCHLD

			// Unshare the process file handles by creating a duplicate collection and then
			// close all file descriptors that were set for close-on-exec
			m_handles = ProcessHandles::Duplicate(m_handles);
			m_handles->RemoveCloseOnExecute();

			// Unshare the signal actions by creating a duplicate collection and then resetting
			// all non-ignored signal actions back to their defaults
			m_sigactions = SignalActions::Duplicate(m_sigactions);
			m_sigactions->Reset();

			// Remove all existing memory sections from the current process
			m_memory->Clear();

			// Reallocate the local descriptor table for the process at the original address
			try { m_memory->Allocate(m_ldt, LINUX_LDT_ENTRIES * sizeof(uapi::user_desc32), LINUX_PROT_READ | LINUX_PROT_WRITE); }
			catch(Exception& ex) { throw LinuxException(LINUX_ENOMEM, ex); }

			// The local descriptor table is new, remove all set slot bits
			m_ldtslots.Clear();

			// Allocate the stack for the new main process thread
			const void* stackpointer = nullptr;
			try { stackpointer = CreateStack(m_vm, m_memory); }
			catch(Exception& ex) { throw LinuxException(LINUX_ENOMEM, ex); }

			// Load the executable image into the process address space and set up the thread stack
			Executable::LoadResult loaded = executable->Load(m_memory, stackpointer);

			// Reset the program break address based on the loaded executable 
			m_programbreak = loaded.ProgramBreak;

			// Make the new thread inherit the process pid and become the thread group leader
			m_threads.emplace(m_pid, Thread::FromNativeHandle<architecture>(m_pid, m_process, thread, threadid, 
				TaskState::Create<architecture>(loaded.EntryPoint, loaded.StackPointer)));

			ResumeInternal();		// Resume the process; this will start the thread as well
		}

		// Kill the entire process on exception, use SIGKILL as the exit code
		catch(...) { Terminate(uapi::MakeExitCode(0, LINUX_SIGKILL)); throw; }
	}

	// Kill just the newly created remote thread on exception
	catch(...) { TerminateThread(thread->Handle, uapi::MakeExitCode(0, LINUX_SIGKILL)); throw; }
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
// Process::getNativeHandle
//
// Gets the native operating system handle for this process

HANDLE Process::getNativeHandle(void) const
{
	return m_process->Handle;
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
	return nullptr;
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
	// Remove the specified file descriptor from the process
	m_handles->Remove(fd);
}

//-----------------------------------------------------------------------------
// Process::RemoveThread
//
// Removes a thread from the process
//
// Arguments:
//
//	tid			- Identifier for the thread to be removed
//	exitcode	- Exit code reported by the thread

// TODO: Rename me to ExitThread() or OnThreadExit() or something?
// RemoveThread doesn't describe how its supposed to be used
void Process::RemoveThread(uapi::pid_t tid, int exitcode)
{
	std::lock_guard<std::mutex>		crtisec(m_statuslock);
	thread_map_lock_t::scoped_lock	writer(m_threadslock);

	try {
		
		// TODO: this needs to be done to signal the StateChanged event of the thread
		// but I'm not fond of how this works out.  sys_exit/release_thread/rundown_context
		// could call this, but then the process wouldn't know to remove it
		// unless RemoveThread() was still called, so for now just put this here

		auto thread = m_threads.at(tid);			// can throw std::out_of_range if not there
		thread->Exit(exitcode);						// TODO: rename to OnExit()??

		// Remove the thread from the collection
		m_threads.erase(tid);			
	}

	catch(std::out_of_range&) { return; }
	// TODO: catch all else here?

	// If this was the last thread in the process, the process is exiting normally.
	// Signal the parent and set as terminated, but don't wait for the process itself,
	// as the calling thread is technically still running
	if(m_threads.size() == 0) {

		// The termination signal is atomic, access it only once
		int signal = m_termsignal;
		if(signal) {

			// Attempt to acquire a parent process reference and signal it
			auto parent = m_parent.lock();
			if((parent) && (parent->ProcessId != this->ProcessId)) parent->Signal(signal);
		}

		// Signal that the process has terminated
		m_statuscode = exitcode;
		SetWaitableState(State::Terminated);
	}
}

//-----------------------------------------------------------------------------
// Process::Resume
//
// Resumes the process
//
// Arguments:
//
//	NONE

void Process::Resume(void)
{
	std::lock_guard<std::mutex> critsec(m_statuslock);

	// Resume all threads in the process
	ResumeInternal();
	
	// Signal that the process is running again
	m_statuscode = uapi::RUNNING;
	SetWaitableState(State::Resumed);
}

//-----------------------------------------------------------------------------
// Process::ResumeInternal (private)
//
// Resumes the native operating system process from a suspended state
//
// Arguments:
//
//	NONE

void Process::ResumeInternal(void) const
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
// Process::Signal
//
// Signals the process
//
// Arguments:
//
//	signal		- Signal to send to the process

bool Process::Signal(int signal)
{
	//
	// TODO: There needs to be an overriding synchronization object
	// to serialize all signal activity at the PROCESS level, this means
	// that all actions (sigprocmask, etc) need to come through PROCESS
	// and never directly through THREAD.  Without doing this, there will
	// be a race condition that allows things to change while figuring
	// out what to do, and can cause a pending signal release event (such as
	// changing a sigprocmask to unblock it) to go unnoticed
	//

	// Get the action specified for this signal
	uapi::sigaction action = m_sigactions->Get(signal);

	// Signals specifically set as ignored do not get delivered to a thread
	if(action.sa_handler == LINUX_SIG_IGN) return true;

	// Signals that default to ignored do not get delivered either, this
	// currently includes SIGCHLD (17), SIGURG (23) and SIGWINCH (28)
	if((action.sa_handler == LINUX_SIG_DFL) && ((signal == LINUX_SIGCHLD) || 
		(signal == LINUX_SIGURG) || (signal == LINUX_SIGWINCH))) return true;

	// Acquire a read lock against the thread collection
	thread_map_lock_t::scoped_lock_read reader(m_threadslock);

	// Iterate over all the threads in the process
	for(const auto iterator : m_threads) {

		// Ask the current thread to process the signal
		switch(iterator.second->Signal(signal, action)) {

			// SignalResult::Blocked
			//
			// Continue searching for a thread that can handle this signal
			case Thread::SignalResult::Blocked: continue;

			// SignalResult::Delivered
			//
			// The thread accepted and delivered the signal
			case Thread::SignalResult::Delivered: return true;

			// SignalResult::Ignored
			//
			// The thread accepted and ignored the signal
			case Thread::SignalResult::Ignored: return true;

			// SignalResult::CoreDump (COREDUMP)
			//
			// The process should be core-dumped and terminated
			case Thread::SignalResult::CoreDump:
				Terminate(uapi::MakeExitCode(0, signal, true));
				return true;

			// SignalResult::Terminate (TERM)
			//
			// The process should be terminated
			case Thread::SignalResult::Terminate:
				Terminate(uapi::MakeExitCode(0, signal));
				return true;

			// SignalResult::Resume (CONT)
			//
			// The process should be resumed
			case Thread::SignalResult::Resume:
				Resume();
				return true;

			// SignalResult::Suspend (STOP)
			//
			// The process should be suspended
			case Thread::SignalResult::Suspend:
				Suspend();
				return true;

			// Unknown signal result
			//
			default:
				// do throw exception
				break;
		}
	}

	//
	// TODO: SIGNAL QUEUE MUST BE A PRIORITY QUEUE
	// QUEUE WILL ALSO NEED TO KNOW THE TARGET THREAD ID IF IT WAS A DIRECTED SIGNAL
	//

	// No thread in the process will currently accept this signal, queue it as pending
	//
	// TODO: SIGNAL IS NOW PENDING, NOT 'UNHANDLED' - still return false?
	//
	return false;
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
//	parent		- Optional parent process to assign to the process
//	argv		- Command-line arguments
//	envp		- Environment variables
//	rootdir		- Process root directory
//	workingdir	- Process working directory

std::shared_ptr<Process> Process::Spawn(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t pid, const std::shared_ptr<Process>& parent, 
	const char_t* filename, const char_t* const* argv, const char_t* const* envp, const std::shared_ptr<FileSystem::Alias>& rootdir, 
	const std::shared_ptr<FileSystem::Alias>& workingdir)
{
	// Parse the filename and command line arguments into an Executable instance
	auto executable = Executable::FromFile(vm, filename, argv, envp, rootdir, workingdir);

	// Architecture::x86 --> 32-bit executable
	if(executable->Architecture == Architecture::x86) return Spawn<Architecture::x86>(vm, pid, parent, executable);

#ifdef _M_X64
	// Architecture::x86_64 --> 64-bit executable
	else if(executable->Architecture == Architecture::x86_64) return Spawn<Architecture::x86_64>(vm, pid, parent, executable);
#endif
	
	throw LinuxException(LINUX_ENOEXEC);
}

//-----------------------------------------------------------------------------
// Process::Spawn<Architecture> (private, static)
//
// Creates a new process based on an Executable instance
//
// Arguments:
//
//	vm				- Parent virtual machine instance
//	pid				- Virtual process identifier to assign
//	parent			- Optional parent process to assign to the process
//	executable		- Executable instance

template<::Architecture architecture>
std::shared_ptr<Process> Process::Spawn(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t pid, const std::shared_ptr<Process>& parent, 
	const std::unique_ptr<Executable>& executable)
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
		try { stackpointer = CreateStack(vm, memory); }
		catch(Exception& ex) { throw LinuxException(LINUX_ENOMEM, ex); }

		// Load the executable image into the process address space and set up the thread stack
		Executable::LoadResult loaded = executable->Load(memory, stackpointer);

		// Create a Thread instance for the main thread of the new process, using a new TaskState
		std::shared_ptr<::Thread> mainthread = ::Thread::FromNativeHandle<architecture>(pid, host->Process, host->Thread, host->ThreadId,
			TaskState::Create<architecture>(loaded.EntryPoint, loaded.StackPointer));

		// Construct and return the new Process instance
		return std::make_shared<Process>(vm, architecture, pid, parent, host->Process, host->ProcessId, std::move(memory), ldt, Bitmap(LINUX_LDT_ENTRIES), 
			loaded.ProgramBreak, mainthread, LINUX_SIGCHLD, executable->RootDirectory, executable->WorkingDirectory);
	}

	// Terminate the created host process on exception
	catch(...) { host->Terminate(LINUX_SIGKILL); throw; }
}
	
//-----------------------------------------------------------------------------
// Process::Start
//
// Starts a newly created process instance
//
// Arguments:
//
//	NONE

void Process::Start(void)
{
	std::lock_guard<std::mutex> critsec(m_statuslock);

	// Start all threads in the process and update status to RUNNING
	ResumeInternal();
	m_statuscode = uapi::RUNNING;
}

//-----------------------------------------------------------------------------
// Process::getStatusCode
//
// Gets the current status/exit code for the process

int Process::getStatusCode(void)
{
	std::lock_guard<std::mutex> critsec(m_statuslock);
	return m_statuscode;
}

//-----------------------------------------------------------------------------
// Process::Suspend
//
// Suspends the process
//
// Arguments:
//
//	NONE

void Process::Suspend(void)
{
	std::lock_guard<std::mutex> critsec(m_statuslock);

	// Suspend all threads in the process
	SuspendInternal();
	
	// Signal that the process has been stopped
	m_statuscode = uapi::STOPPED;
	SetWaitableState(State::Suspended);
}

//-----------------------------------------------------------------------------
// Process::SuspendInternal (private)
//
// Suspends the native operating system process
//
// Arguments:
//
//	NONE

void Process::SuspendInternal(void) const
{
	NTSTATUS result = NtApi::NtSuspendProcess(m_process->Handle);
	if(result != 0) throw StructuredException(result);
}

//-----------------------------------------------------------------------------
// Process::Terminate
//
// Terminates the process
//
// Arguments:
//
//	exitcode	- Exit code for the process

void Process::Terminate(int exitcode)
{
	std::lock_guard<std::mutex> critsec(m_statuslock);

	// Terminate the process and wait for it to actually exit
	TerminateProcess(m_process->Handle, exitcode);
	WaitForSingleObject(m_process->Handle, INFINITE);

	// When a process is terminated, the parent optionally receives a signal
	// m_termsignal is std::atomic<>, access it only once
	int termsignal = m_termsignal;
	if(termsignal) {

		auto parent = m_parent.lock();
		if((parent) && (parent->ProcessId != this->ProcessId)) parent->Signal(termsignal);
	}

	// Signal that the process has terminated
	m_statuscode = exitcode;
	SetWaitableState(State::Terminated);
}

//-----------------------------------------------------------------------------
// Process::getTerminationSignal
//
// Gets the termination signal to send to the parent on termination

int Process::getTerminationSignal(void) const
{
	return m_termsignal;
}

//-----------------------------------------------------------------------------
// Process::putTerminationSignal
//
// Sets the termination signal to send to the parent on termination

void Process::putTerminationSignal(int value)
{
	m_termsignal = value;
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
// Process::WaitChild
//
// Waits for a child process/thread to terminate
//
// Arguments:
//
//	pid			- Specific PID to wait for, or flag for a multiple wait
//	status		- Optionaly receives the exit status
//	options		- Wait operation operations

uapi::pid_t Process::WaitChild(uapi::pid_t pid, int* status, int options)
{
	// Cannot wait for your own process or main thread to terminate
	if(pid == m_pid) throw LinuxException(LINUX_ECHILD);

	// Create a collection of waitable objects to wait against, this also
	// prevents the objects from dying off prematurely during a wait op
	std::vector<std::shared_ptr<Waitable>> objects;

	// NOT _WCLONE
	//
	// Wait for child processes that send SIGCHLD only
	if((options & LINUX__WCLONE) == 0) {
	
		process_map_lock_t::scoped_lock_read reader(m_childlock);
		for(auto iterator : m_children) {

			// Processes that do not send ECHILD on termination are skipped
			if(iterator.second->TerminationSignal != LINUX_ECHILD) continue;

			// TODO (pid < -1) -- specified process group (use absolute value)
			if(false) {

				int pgroup = -pid;
				(pgroup);
			}

			// TODO (pid == 0) -- this process group
			else if(false) {
			}

			// (pid == -1)  --> any child process
			else if(pid == -1) objects.push_back(iterator.second);

			// (pid == pid) --> specific child process
			else if(iterator.second->ProcessId == pid) objects.push_back(iterator.second);
		}
	}

	// _WALL OR _WCLONE
	//
	// Wait for threads and non-SIGCHLD processes as well
	if((options & LINUX__WALL) || (options && LINUX__WCLONE)) {

		// Lock both the child process and thread collection for iteration
		process_map_lock_t::scoped_lock_read proc_reader(m_childlock);
		thread_map_lock_t::scoped_lock_read thread_reader(m_threadslock);

		for(auto iterator : m_children) {

			// TODO: PID filtering (like above) todo: just redo this, have some function that
			// takes the collections and flags and comes up with this instead

			// Child processes that don't send SIGCHLD count for WALL or _WCLONE
			if(iterator.second->TerminationSignal != LINUX_SIGCHLD) objects.push_back(iterator.second);
		}

		for(auto iterator: m_threads) {

			// Do not include the process main thread (tid == pid)
			if(iterator.second->ThreadId != m_pid) objects.push_back(iterator.second);
		}
	}

	// If no PIDs to wait against were located based on the options, throw ECHILD
	if(objects.size() == 0) throw LinuxException(LINUX_ECHILD);

	// TODO: move this out?
	std::vector<HANDLE> handles;
	for(auto iterator : objects) handles.push_back(iterator->WaitableStateChanged);

	// TODO: need while loop here to spin on stopped/continued processes when the flags
	// didn't indicate to wait for them; repeatedly call WaitForMultipleObjects()

	// Wait for any or all of the collected handles to become signaled
	DWORD result = WaitForMultipleObjects(handles.size(), handles.data(), (options & LINUX__WALL) ? TRUE : FALSE, (options & LINUX_WNOHANG) ? 0 : INFINITE);

	// TODO: need to check result is actually valid
	std::shared_ptr<Waitable> selected = objects[result - WAIT_OBJECT_0];

	// TODO: if the object has terminated, wait for it to actually terminate, threads
	// and processes that exit normally have rundown code to execute
	if(selected->PopWaitableState() == State::Terminated) {

		// TODO - need to add getter for native handle to Schedulable?  Could also
		// dynamically cast to Process/Thread
		//WaitForSingleObject(selected->
	}
		
	// Report the exit code for the process/thread if the caller wants to know
	if(status) *status = selected->StatusCode;

	// TODO: TEMPORARY -- MOVE UP (or get rid of it)
	std::shared_ptr<Process> tempproc = std::dynamic_pointer_cast<Process>(selected);

	// todo: if nothing was signaled and WNOHANG was set, return zero instead
	if(result == WAIT_FAILED) throw LinuxException(LINUX_ECHILD);

	// WAIT_TIMEOUT will only be sent if nothing was signaled after a zero timeout
	if(result == WAIT_TIMEOUT) return 0;

	else {

		// Remove the specified PID from the process collection
		process_map_lock_t::scoped_lock procwriter(m_childlock);

		if(options & LINUX__WALL) m_children.clear();
		else m_children.erase(tempproc->ProcessId);

		//// Remove the specified PID from the thread collection
		//// todo: if _WALL, clear all but main thread (pid == tid)
		//process_map_lock_t::scoped_lock threadwriter(m_threadslock);
		//m_threads.erase(tempthread->ProcessId);
	}

	// this needs to happen to release the Process instance
	m_vm->RemoveProcess(tempproc->ProcessId);
	// todo: vm->RemoveProcess(waitedon);

	// Return the PID of the object that was signaled
	return tempproc->ProcessId;

	// FLAGS TO DEAL WITH IN HERE
	//#define LINUX_WNOHANG				0x00000001
	//#define LINUX_WUNTRACED				0x00000002
	//#define LINUX_WSTOPPED				WUNTRACED
	//#define LINUX_WEXITED				0x00000004
	//#define LINUX_WCONTINUED			0x00000008
	//#define LINUX_WNOWAIT				0x01000000		/* Don't reap, just poll status.  */
	//
	//#define LINUX__WNOTHREAD			0x20000000		/* Don't wait on children of other threads in this group */
	//#define LINUX__WALL					0x40000000		/* Wait on all children, regardless of type */
	//#define LINUX__WCLONE				0x80000000		/* Wait only on non-SIGCHLD children */
	//#define LINUX_P_ALL				0
	//#define LINUX_P_PID				1
	//#define LINUX_P_PGID			2
}

// version for waitid()
void Process::WaitChild(int which, uapi::pid_t pid, uapi::siginfo* info, int options, uapi::rusage* rusage)
{
	// which argument values to determine collection contents
	LINUX_P_ALL;	// 0
	LINUX_P_PID;	// 1
	LINUX_P_PGID;	// 2

	(which);
	(pid);
	(info);
	(options);
	(rusage);

	// BUILD COLLECTION BASED ON FLAGS
	// CALL WAITCHILD() INTERNAL VERSION

	//
	// NOTE: NEED TO MAKE EVENT IN SCHEDULABLE MANUAL RESET AGAIN
	//

	throw LinuxException(LINUX_ENOSYS);
}

//-----------------------------------------------------------------------------
// Process::WaitChild (private)
//
// Internal version of WaitChild implementation
//
// Arguments:
//
//	objects		- Collection of child objects to wait against
//	options		- Wait operation options (waitid(2) set)
//	waitall		- Flag to wait for all objects rather than just one
//	siginfo		- Receives information about the selected child
//	rusage		- Receives resource utilization information about the selected child

void Process::WaitChild(std::vector<std::shared_ptr<Waitable>>& objects, int options, bool waitall, uapi::siginfo* siginfo, uapi::rusage* rusage)
{
	DWORD						waitresult;				// Result from wait operation

	// Convert the collection of Waitable objects into waitable HANDLEs.  The objects collection
	// still remains and will keep the shared_ptr<> instances around until we are finished here
	std::vector<HANDLE> handles(objects.size());
	for(auto iterator : objects) handles.push_back(iterator->WaitableStateChanged);

	while(true) {

		// Wait for one or all of the specified object handles to become signaled
		waitresult = WaitForMultipleObjects(handles.size(), handles.data(), (waitall) ? TRUE : FALSE, (options & LINUX_WNOHANG) ? 0 : INFINITE);

		// WAIT_FAILED
		//
		// An error occurred during the wait operation; throw an exception
		if(waitresult == WAIT_FAILED) throw Win32Exception();	// todo: exception

		// WAIT_TIMEOUT
		//
		// None of the specified objects became signaled; leave siginfo and rusage unspecified
		else if(waitresult == WAIT_TIMEOUT) return;

		// WAIT_OBJECT_0 [WAITALL]
		//
		// All of the specified objects have been signaled
		else if((waitall) && (waitresult == WAIT_OBJECT_0)) {
		}

		// WAIT_OBJECT_0 + n [WAITANY]
		//
		// One of the specified objects has been signaled


		// check result against options
		// if suspended and not WSTOPPED -> loop
		// if resumed and not WCONTINUED -> loop
		// if terminated and not WEXITED -> loop

		// if not WNOWAIT, reset the event
		
		// get the exit code
		// fill in siginfo and rusage

		siginfo->si_signo = LINUX_SIGCHLD;
		siginfo->si_errno = 0;
		siginfo->si_code = 0;
		siginfo->linux_si_pid = 0;
		siginfo->linux_si_uid = 0;
		siginfo->linux_si_status = 0;
		// code

		// return
	}

	(options);
	(waitall);
	(siginfo);
	(rusage);

	// need to know:
	// what to wait for (suspend, resume, terminate)
	// wait for one or wait for all
	// reset the event or not
	// block or don't block (nohang)

	// need to return:
	// siginfo, rusage

//#define LINUX_WNOHANG			0x00000001
//#define LINUX_WUNTRACED			0x00000002
//#define LINUX_WSTOPPED			LINUX_WUNTRACED
//#define LINUX_WEXITED			0x00000004
//#define LINUX_WCONTINUED		0x00000008
//#define LINUX_WNOWAIT			0x01000000		/* Don't reap, just poll status.  */

//
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

#pragma warning(pop)
