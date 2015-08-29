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

#include "ProcessGroup.h"
#include "Session.h"
#include "StructuredException.h"

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
//	host			- NativeProcess instance
//	task			- TaskState for the initial process thread
//	memory			- ProcessMemory virtual memory manager
//	ldt				- Address of allocated local descriptor table
//	ldtslots		- Local descriptor table allocation bitmap
//	programbreak	- Address of initial program break
//	termsignal		- Signal to send to parent on termination
//	rootdir			- Initial process root directory
//	workingdir		- Initial process working directory

Process::Process(const std::shared_ptr<_VmOld>& vm, ::Architecture architecture, uapi::pid_t pid, const std::shared_ptr<Process>& parent, 
	std::unique_ptr<NativeProcess>&& host, std::unique_ptr<TaskState>&& task, std::unique_ptr<ProcessMemory>&& memory, const void* ldt, Bitmap&& ldtslots, 
	const void* programbreak, int termsignal, const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir) : 
	Process(vm, architecture, pid, parent, std::move(host), std::move(task), std::move(memory), ldt, std::move(ldtslots), programbreak, 
	ProcessHandles::Create(), SignalActions::Create(), termsignal, rootdir, workingdir) {}

//-----------------------------------------------------------------------------
// Process Constructor
//
// Arguments:
//
//	vm				- Virtual machine instance
//	architecture	- Process architecture
//	pid				- Virtual process identifier
//	parent			- Parent Process instance
//	host			- NativeProcess instance
//	task			- TaskState for the initial process thread
//	memory			- ProcessMemory virtual memory manager
//	ldt				- Address of allocated local descriptor table
//	ldtslots		- Local descriptor table allocation bitmap
//	programbreak	- Address of initial program break
//	handles			- Initial file system handles collection
//	sigactions		- Initial set of signal actions
//	termsignal		- Signal to send to parent on termination
//	rootdir			- Initial process root directory
//	workingdir		- Initial process working directory

Process::Process(const std::shared_ptr<_VmOld>& vm, ::Architecture architecture, uapi::pid_t pid, const std::shared_ptr<Process>& parent, 
	std::unique_ptr<NativeProcess>&& host, std::unique_ptr<TaskState>&& task, std::unique_ptr<ProcessMemory>&& memory, const void* ldt, Bitmap&& ldtslots, 
	const void* programbreak, const std::shared_ptr<ProcessHandles>& handles, const std::shared_ptr<SignalActions>& sigactions, int termsignal, 
	const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir) : m_vm(vm), m_architecture(architecture), 
	m_pid(pid), m_parent(parent), m_host(std::move(host)), m_memory(std::move(memory)), m_ldt(ldt), m_ldtslots(std::move(ldtslots)), 
	m_programbreak(programbreak), m_handles(handles), m_sigactions(sigactions), m_termsignal(termsignal), m_rootdir(rootdir), m_workingdir(workingdir), 
	m_threadproc(nullptr), m_nochildren(true) 
{
	m_pendingthreads.emplace(m_host->ThreadId, std::move(task));
}

//-----------------------------------------------------------------------------
// Process Destructor

Process::~Process()
{
	auto vm = m_vm.lock();			// Parent virtual machine instance

	// There should not be any threads left in this process
	_ASSERTE(m_threads.size() == 0);

	// Release the TIDs for any threads that still exist in the collection
	for(const auto& iterator : m_threads)
		if((iterator.first != m_pid) && vm) vm->ReleasePID(iterator.first);
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
// Process::AttachThread
//
// Attaches a newly created thread to the process instance
//
// Arguments:
//
//	nativetid		- Native OS thread identifier

std::shared_ptr<::Thread> Process::AttachThread(DWORD nativetid)
{
	std::shared_ptr<::Thread>		thread;				// Attached Thread instance

	// Lock the parent virtual machine instance in order to acquire the thread identifier
	auto vm = m_vm.lock();
	if(!vm) throw LinuxException(LINUX_ESRCH);

	// Attempt to open a new NativeHandle with THREAD_ALL_ACCESS against the provided thread identifier
	std::shared_ptr<NativeHandle> handle = NativeHandle::FromHandle(OpenThread(THREAD_ALL_ACCESS, FALSE, nativetid));
	if(handle->Handle == nullptr) throw LinuxException(LINUX_ESRCH, Win32Exception());

	// Acquire exclusive access to the pending threads collection and condition variable
	std::unique_lock<std::mutex> critsec(m_attachlock);

	// Locate the pending thread in the collection
	auto iterator = m_pendingthreads.find(nativetid);
	if(iterator == m_pendingthreads.end()) throw LinuxException(LINUX_ESRCH);

	// Acquire exclusive access to the active threads collection
	thread_map_lock_t::scoped_lock_write writer(m_threadslock);

	// Allocate a thread identifier for the new Thread instance; the first thread in the process
	// (or a replacement main thread from Execute) inherits the process PID instead
	uapi::pid_t tid = (m_threads.size() == 0) ? m_pid : vm->AllocatePID();

	// Attempt to create the Thread instance, releasing the allocated thread identifier on exception
	try { thread = Thread::Create(shared_from_this(), tid, handle, nativetid, std::move(iterator->second)); }
	catch(...) { if(tid != m_pid) vm->ReleasePID(tid); throw; }

	m_threads.emplace(tid, thread);			// Insert the new Thread
	m_pendingthreads.erase(iterator);		// Remove pending thread entry
	m_attached.notify_all();				// Notify pending collection changed

	return thread;
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
// Process::ClearThreads (private)
//
// Removes all threads from the process
//
// Arguments:
//
//	NONE

void Process::ClearThreads(void)
{
	// Take an exclusive lock against the threads collection
	thread_map_lock_t::scoped_lock_write writer(m_threadslock);

	// Iterate over the collection of threads and remove all of them
	for(const auto& iterator : m_threads) iterator.second->Terminate(LINUX_SIGTERM);

	m_threads.clear();			// Remove all threads
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
	auto vm = m_vm.lock();
	if(!vm) throw LinuxException(LINUX_ESRCH);

	// Allocate the PID for the cloned process
	uapi::pid_t pid = vm->AllocatePID();

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
	catch(...) { vm->ReleasePID(pid); throw; }
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
	auto vm = m_vm.lock();
	if(!vm) throw LinuxException(LINUX_ESRCH);

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
	std::unique_ptr<NativeProcess> childhost = NativeProcess::Create<architecture>(vm);

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
		//std::shared_ptr<::Thread> childthread = ::Thread::FromNativeHandle<architecture>(pid, childhost->Process, childhost->Thread, childhost->ThreadId, std::move(task));

		// Determine the child termination signal from the low byte of the flags
		int childtermsig = (flags & 0xFF);

		// Construct and insert a new Process instance to the child collection
		process_map_lock_t::scoped_lock_write writer(m_childlock);
		auto emplaced = m_children.emplace(pid, std::make_shared<Process>(vm, m_architecture, pid, childparent, std::move(childhost), std::move(task),
			std::move(childmemory), m_ldt, Bitmap(m_ldtslots), m_programbreak, childhandles, childsigactions, childtermsig, m_rootdir, m_workingdir));

		// todo: dirty hack
		auto pgroup = m_pgroup.lock();
		pgroup->AttachProcess(emplaced.first->second);
		emplaced.first->second->m_pgroup = pgroup;

		// Reset the condition variable indicating that there are no children as there now is one
		m_nochildren = false;

		// Verify that the new Process instance was added to the collection and return the shared_ptr
		if(!emplaced.second) throw Exception(E_PROCESSDUPLICATEPID, pid, m_pid);
		return emplaced.first->second;
	}

	// Kill the native operating system process if any exceptions occurred during creation
	catch(...) { childhost->Terminate(LINUX_SIGKILL); throw; }
}

//-----------------------------------------------------------------------------
// Process::CollectWaitables (private)
//
// todo: this will be renamed or absorbed into something else

std::vector<std::shared_ptr<Waitable>> Process::CollectWaitables(uapi::idtype_t type, uapi::pid_t id, int options)
{
	// Create a collection of waitable objects to operate against
	std::vector<std::shared_ptr<Waitable>> objects;

	// Lock the contents of the process collection
	process_map_lock_t::scoped_lock_read proc_reader(m_childlock);

	// Iterate over all of the child processes in the collection
	for(const auto& iterator : m_children) {

		// P_PID - Wait for a specific process id
		if(type == LINUX_P_PID) { if(iterator.second->ProcessId != id) continue; }

		// P_PGID - Wait for children in a specific process group
		if(type == LINUX_P_PGID) { /* todo: continue if iterator pgid is not id */ }

		// "clone" processes are ones that don't send SIGCHLD on termination
		bool isclone = (iterator.second->TerminationSignal != LINUX_SIGCHLD);

		// clone processes require __WCLONE or __WALL to be set
		if(isclone && ((options & (LINUX__WCLONE | LINUX__WALL)) == 0)) continue;

		// non-clone processes require __WCLONE to not be set
		if(!isclone && (options & LINUX__WCLONE)) continue;

		// TODO: __WNOTHREAD

		/*	 Normally wait* calls look for any ready child (or ptrace attachee)
		 > of any thread in the process (thread group). The POSIX concept of
		 > parents and children is only about processes, but in Linux the
		 > actual links are between individual threads and the POSIX behavior
		 > comes about in ways like wait looking at each sibling's children
		 > list. Unlike things with POSIX processes, ptrace is actually always
		 > thread to thread. Only the one thread that did PTRACE_ATTACH (or
		 > fork'd the child) can use ptrace on it, but normally wait* calls
		 > made by any other thread in the process can instead be the one to
		 > report its stops.*/	
			/*
		> The __WNOTHREAD option tells wait* to check only the calling
		 > thread's own children (and ptrace attachees). When the ptracer
		 > isn't part of a multithreaded process, it has no effect. When it
		 > is, you might as well use __WNOTHREAD if you are only interested in
		 > ptrace-attached threads and are calling wait* in the ptracer thread.
		 > It saves some loops around the thread list. If there are other
		 > threads in the process that might call wait*, then you need to make
		 > sure they do use __WNOTHREAD if you don't want them to swallow the
		 > reports from your ptrace targets.*/

		objects.push_back(iterator.second);
	}

	return objects;
}

//-----------------------------------------------------------------------------
// Process::CreateStack (private, static)
//
// Allocates a new stack in the process
//
// Arguments:
//
//	vm			- Parent _VmOld instance
//	memory		- ProcessMemory instance to use for allocation

const void* Process::CreateStack(const std::shared_ptr<_VmOld>& vm, const std::unique_ptr<ProcessMemory>& memory)
{
	// Get the default thread stack size from the _VmOld instance and convert it 
	size_t stacklen;
	try { stacklen = std::stoul(vm->GetProperty(_VmOld::Properties::ThreadStackSize)); }
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
	auto executable = Executable::FromFile(filename, argv, envp, m_rootdir, m_workingdir);

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
	DWORD					nativetid;			// Native thread identifier
	unsigned long			timeoutms;			// Thread attach timeout value

	auto vm = m_vm.lock();
	if(!vm) throw LinuxException(LINUX_ESRCH);

	// Get the thread attach timeout value from the virtual machine properties
	try { timeoutms = std::stoul(vm->GetProperty(_VmOld::Properties::ThreadAttachTimeout)); }
	catch(...) { throw Exception(E_PROCESSINVALIDTHREADTIMEOUT); }

	// Suspend the process; all existing threads will be terminated
	SuspendInternal();

	// Create a new thread in the process using the address provided at process registration, using 
	// the minimum allowed thread stack size, it will be overcome by the custom stack allocated below
	auto thread = ::NativeHandle::FromHandle(CreateRemoteThread(m_host->Process->Handle, nullptr, SystemInformation::AllocationGranularity, 
		reinterpret_cast<LPTHREAD_START_ROUTINE>(m_threadproc), nullptr, CREATE_SUSPENDED, &nativetid));
	if(thread->Handle == nullptr) throw Win32Exception();

	//
	// TODO: WHAT HAPPENS TO CHILD PROCESSES DURING EXECVE()?
	//

	// Once the new thread has been created, any exceptions that occur are considered fatal
	// and will result in the termination of the entire process
	try {

		ClearThreads();					// Terminate and remove all existing threads

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
		m_ldtslots.ClearAll();
		try { m_memory->Allocate(m_ldt, LINUX_LDT_ENTRIES * sizeof(uapi::user_desc32), LINUX_PROT_READ | LINUX_PROT_WRITE); }
		catch(Exception& ex) { throw LinuxException(LINUX_ENOMEM, ex); }

		// Load the executable image into the process address space with a new stack allocation
		Executable::LoadResult loaded = executable->Load(m_memory, CreateStack(vm, m_memory));

		// Reset the program break address based on the loaded executable 
		m_programbreak = loaded.ProgramBreak;

		// Lock the pending threads collection and insert a new TaskState for the thread to acquire
		std::unique_lock<std::mutex> critsec(m_attachlock);
		m_pendingthreads.emplace(nativetid, TaskState::Create<architecture>(loaded.EntryPoint, loaded.StackPointer));

		// Resume the process within the context of the unique_lock<> to prevent a race condition
		ResumeInternal();

		// Wait up to defined timeout in milliseconds for the thread to attach and remove the element
		if(m_attached.wait_until(critsec, std::chrono::system_clock::now() + std::chrono::milliseconds(timeoutms), 
			[&]() -> bool { return m_pendingthreads.count(nativetid) == 0; })) return;

		// Throw ESRCH / E_PROCESSTHREADTIMEOUT if the wait operation expired
		throw LinuxException(LINUX_ESRCH, Exception(E_PROCESSTHREADTIMEOUT, m_pid));
	}

	// Kill the entire process on exception, using SIGKILL as the exit code
	catch(...) { Terminate(uapi::MakeExitCode(0, LINUX_SIGKILL)); throw; }
}

//-----------------------------------------------------------------------------
// Process::ExitThread
//
// Indicates that a thread has exited normally
//
// Arguments:
//
//	tid			- Identifier for the thread to be removed
//	exitcode	- Exit code reported by the thread

void Process::DetachThread(const std::shared_ptr<::Thread>& thread, int exitcode)
{
	(thread);
	(exitcode);
}

void Process::ExitThread(uapi::pid_t tid, int exitcode)
{
	auto vm = m_vm.lock();
	if(!vm) throw LinuxException(LINUX_ESRCH);

	thread_map_lock_t::scoped_lock_write writer(m_threadslock);

	// Locate the Thread instance associated with this thread id
	const auto& iterator = m_threads.find(tid);
	if(iterator == m_threads.end()) throw LinuxException(LINUX_ESRCH);

	// Remove the Thread instance from the collection
	if(tid != m_pid) vm->ReleasePID(tid);
	m_threads.erase(iterator);

	// If this was the last thread in the process, the process is exiting normally
	if(m_threads.size() == 0) NotifyParent(Waitable::State::Exited, exitcode);

	// If the parent of this process is set to SA_NOCLDWAIT or ignores SIGCHLD, this
	// process doesn't need to become a zombie and can remove itself from the process group
	auto parent = m_parent.lock();
	if(parent) {

		auto sigchild = parent->SignalAction[LINUX_SIGCHLD];
		if((sigchild.sa_flags & LINUX_SA_NOCLDWAIT) || (sigchild.sa_handler = LINUX_SIG_IGN)) {
		}

	}

	// testing only - no parent init process; remove to prevent zombie for now
	if(!parent) {

		auto pgroup = m_pgroup.lock();
		if(pgroup) pgroup->ReleaseProcess(m_pid);
	}
	// TODO:
	// process becomes a zombie unless parent is set to SA_NOCLDWAIT or ignores SIGCHLD
}

//-----------------------------------------------------------------------------
// Process::FromExecutable (static)
//
// Creates a new process instance from an executable
//
// Arguments:
//
//	pgroup		- Parent process group instance
//	pid			- Process identifier
//	ns			- Namespace instance
//	executable	- Executable to construct into a process

std::shared_ptr<Process> Process::FromExecutable(const std::shared_ptr<_VmOld>& vm, const std::shared_ptr<::ProcessGroup>& pgroup, uapi::pid_t pid,
	const std::unique_ptr<Executable>& executable)
{
	// Architecture::x86 --> 32-bit executable
	if(executable->Architecture == Architecture::x86) return FromExecutable<Architecture::x86>(vm, pgroup, pid, nullptr, executable);

#ifdef _M_X64
	// Architecture::x86_64 --> 64-bit executable
	else if(executable->Architecture == Architecture::x86_64) return FromExecutable<Architecture::x86_64>(vm, pgroup, pid, nullptr, executable);
#endif
	
	throw LinuxException(LINUX_ENOEXEC);
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
//	parent			- Optional parent process to assign to the process
//	executable		- Executable instance

template<::Architecture architecture>
std::shared_ptr<Process> Process::FromExecutable(const std::shared_ptr<_VmOld>& vm, const std::shared_ptr<::ProcessGroup>& pgroup, 
	uapi::pid_t pid, const std::shared_ptr<Process>& parent, const std::unique_ptr<Executable>& executable)
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
		//std::shared_ptr<::Thread> mainthread = ::Thread::FromNativeHandle<architecture>(pid, host->Process, host->Thread, host->ThreadId,
		//	TaskState::Create<architecture>(loaded.EntryPoint, loaded.StackPointer));

		// Construct and return the new Process instance
		auto result = std::make_shared<Process>(vm, architecture, pid, parent, std::move(host), TaskState::Create<architecture>(loaded.EntryPoint, loaded.StackPointer), 
			std::move(memory), ldt, Bitmap(LINUX_LDT_ENTRIES), loaded.ProgramBreak, LINUX_SIGCHLD, executable->RootDirectory, executable->WorkingDirectory);
		result->m_pgroup = pgroup;	// todo: dirty hack

		return result;
	}

	// Terminate the created host process on exception
	catch(...) { host->Terminate(LINUX_SIGKILL); throw; }
}
	
//-----------------------------------------------------------------------------
// Process::GetResourceUsage
//
// Gets resource usage information for the process
//
// Arguments:
//
//	who		- Flag indicating what usage information to collect
//	rusage	- Resultant resource usage information structure

void Process::GetResourceUsage(int who, uapi::rusage* rusage)
{
	FILETIME					creation, exit;			// Creation and exit times
	FILETIME					kernel, user;			// Kernel and user times
	PROCESS_MEMORY_COUNTERS		memory;					// Memory usage information
	IO_COUNTERS					io;						// I/O statistics

	uint64_t					totalkernel = 0;		// Accumulated kernel time
	uint64_t					totaluser = 0;			// Accumulated user time
	size_t						maxworkingset = 0;		// Maximum seen working set

	// RUSAGE_THREAD can only be used with a Thread instance
	_ASSERTE(who != LINUX_RUSAGE_THREAD);
	if(who == LINUX_RUSAGE_THREAD) throw LinuxException(LINUX_EINVAL);

	// Initialize the entire structure to zero, most of the fields won't be populated
	_ASSERTE(rusage);
	memset(rusage, 0, sizeof(uapi::rusage));

	//
	// TODO: This could go into a helper function, send in the const shared_ptr<Process>& and rusage*, much cleaner
	//

	// RUSAGE_SELF - Information about the current process
	//
	if((who == LINUX_RUSAGE_BOTH) || (who == LINUX_RUSAGE_SELF)) {

		// Attempt to acquire the necessary information for this process
		if(!GetProcessTimes(m_host->Process->Handle, &creation, &exit, &kernel, &user)) throw Win32Exception();
		if(!GetProcessMemoryInfo(m_host->Process->Handle, &memory, sizeof(PROCESS_MEMORY_COUNTERS))) throw Win32Exception();
		if(!GetProcessIoCounters(m_host->Process->Handle, &io)) throw Win32Exception();

		// Tally the acquired information, directly into the structure when possible
		totalkernel += (static_cast<uint64_t>(kernel.dwHighDateTime) << 32) + kernel.dwLowDateTime;
		totaluser += (static_cast<uint64_t>(user.dwHighDateTime) << 32) + user.dwLowDateTime;
		rusage->ru_majflt += memory.PageFaultCount;
		rusage->ru_inblock += io.ReadOperationCount;
		rusage->ru_oublock += io.WriteOperationCount;
		if(memory.WorkingSetSize > maxworkingset) maxworkingset = memory.WorkingSetSize;
	}

	// RUSAGE_CHILDREN - Information about all child processes
	//
	if((who == LINUX_RUSAGE_BOTH) || (who == LINUX_RUSAGE_CHILDREN)) {

		process_map_lock_t::scoped_lock_read reader(m_childlock);
		for(const auto& iterator : m_children) {

			// Attempt to acquire the necessary information about the child process
			if(!GetProcessTimes(iterator.second->m_host->Process->Handle, &creation, &exit, &kernel, &user)) throw Win32Exception();
			if(!GetProcessMemoryInfo(iterator.second->m_host->Process->Handle, &memory, sizeof(PROCESS_MEMORY_COUNTERS))) throw Win32Exception();
			if(!GetProcessIoCounters(iterator.second->m_host->Process->Handle, &io)) throw Win32Exception();

			// Tally the acquired information, directly into the structure when possible
			totalkernel += (static_cast<uint64_t>(kernel.dwHighDateTime) << 32) + kernel.dwLowDateTime;
			totaluser += (static_cast<uint64_t>(user.dwHighDateTime) << 32) + user.dwLowDateTime;
			rusage->ru_majflt += memory.PageFaultCount;
			rusage->ru_inblock += io.ReadOperationCount;
			rusage->ru_oublock += io.WriteOperationCount;
			if(memory.WorkingSetSize > maxworkingset) maxworkingset = memory.WorkingSetSize;
		}
	}

	// Convert the total kernel time into uapi::timeval
	totalkernel /= 10;
	rusage->ru_systime.tv_sec = totalkernel / 1000000;		// Seconds
	rusage->ru_systime.tv_usec = totalkernel % 1000000;		// Microseconds

	// Convert the total user time into uapi::timeval
	totaluser /= 10;										
	rusage->ru_utime.tv_sec = totaluser / 1000000;			// Seconds
	rusage->ru_utime.tv_usec = totaluser % 1000000;			// Microseconds

	// Convert the maximum seen working set size to kilobytes
	rusage->ru_maxrss = (maxworkingset >> 10);				// Kilobytes
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
	if(fd > 0) handle = m_handles->Get(fd)->Duplicate();

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
			read = handle->Read(buffer, std::min(length, buffer.Size));
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
	return m_host->ProcessId;
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
// Process::NotifyParent
//
// Notifies the parent process that a state change has occurred
//
// Arguments:
//
//	state		- New process state
//	status		- Status code associated with the changed state

void Process::NotifyParent(Waitable::State state, int32_t status)
{
	bool autoreap = false;				// Process auto-reap flag

	// Attempt to acquire a reference to the parent process and ensure that it hasn't
	// been set up as a self-referential parent instance
	std::shared_ptr<Process> parent = m_parent.lock();
	if((parent) && (parent->m_pid != m_pid)) {

		int signal = 0;			// Default to not sending a signal to the parent

		// Get the parent's disposition for the SIGCHLD signal
		uapi::sigaction sigaction = parent->getSignalAction(LINUX_SIGCHLD);
		if(sigaction.sa_handler != LINUX_SIG_IGN) {

			// State::Exited / State::Killed / State::Dumped
			//
			// Send termination signal to parent if not disabled via SA_NOCLDWAIT
			if((state == State::Exited) || (state == State::Killed) || (state == State::Dumped)) {

				// If the SA_NOCLDWAIT flag is specified for SIGCHLD, the process can automatically
				// reap itself, otherwise the parent has to do it after a wait operation
				autoreap = ((sigaction.sa_flags & LINUX_SA_NOCLDWAIT) == LINUX_SA_NOCLDWAIT);
				if(!autoreap) signal = m_termsignal;
			}

			// State::Trapped / State::Stopped / State::Continued
			//
			// Send SIGCHLD to the parent if not disabled via SA_NOCLDSTOP
			else if((state == State::Trapped) || (state == State::Stopped) || (state == State::Continued)) {

				if((sigaction.sa_flags & LINUX_SA_NOCLDSTOP) == 0) signal = LINUX_SIGCHLD;
			}

			// If a signal code was determined above, signal the parent process
			if(signal) parent->Signal(signal);
		}
	}

	// Notify any threads waiting for this process that a state change has occurred;
	// this is done whether or not SIGCHLD was sent to the parent above
	Waitable::NotifyStateChange(m_pid, state, status);

	// If the process can be automatically reaped without a wait, do it here
	if(autoreap) {
		
		// todo: this needs some work, but ok for now
		// will need to check for a 'subreaper' eventually too
		process_map_lock_t::scoped_lock_write writer(parent->m_childlock);
		//parent->m_vm->RemoveProcess(m_pid);

		// this is ugly
		auto pgroup = m_pgroup.lock();
		if(pgroup) pgroup->ReleaseProcess(m_pid);

		// this is ugly
		parent->m_children.erase(m_pid);
		if(parent->m_children.size() == 0) parent->m_nochildren = true;
	}
}

//-----------------------------------------------------------------------------
// Process::getParent
//
// Gets the parent of this process, or the root process if an orphan

std::shared_ptr<Process> Process::getParent(void) const
{
	// TODO: this actually needs to return the reaper

	auto vm = m_vm.lock();
	if(!vm) throw LinuxException(LINUX_ESRCH);

	// Try to get the parent instance, which may have disappeared orphaning
	// this process.  In that case, use the root/init process as the parent
	std::shared_ptr<Process> parent = m_parent.lock();
	return (parent) ? parent : vm->RootProcess;
}
	
//-----------------------------------------------------------------------------
// Process::getProcessGroup
//
// Gets a reference to the containing process group instance

std::shared_ptr<::ProcessGroup> Process::getProcessGroup(void) const
{
	return m_pgroup.lock();
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
	m_handles->Remove(fd);
}

//-----------------------------------------------------------------------------
// Process::RundownThread
//
// Runs down a thread instance that terminated abnormally and as a result was
// unable to call ExitThread()
//
// Arguments:
//
//	thread		- Thread instance to be run down

void Process::RundownThread(const std::shared_ptr<::Thread>& thread)
{
	thread_map_lock_t::scoped_lock_write writer(m_threadslock);

	// If there are no threads associated with this process, nothing to do
	if(m_threads.size() == 0) return;

	// A thread should only appear in the collection once, but iterate over all
	auto iterator = m_threads.begin();
	while(iterator != m_threads.end()) {

		// Remove the thread from the collection
		if(thread == iterator->second) iterator = m_threads.erase(iterator);
		else ++iterator;
	}

	// If this was the last thread, the process has terminated abnormally.  Indicate
	// this condition by using a process exit code of SIGKILL
	if(m_threads.size() == 0) NotifyParent(Waitable::State::Exited, LINUX_SIGKILL);
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
	ResumeInternal();
	NotifyParent(Waitable::State::Continued, uapi::RUNNING);
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
	NTSTATUS result = NtApi::NtResumeProcess(m_host->Process->Handle);
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

	ldt_lock_t::scoped_lock_write writer(m_ldtlock);

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
	return m_sigactions->Get(signal);
}

//-----------------------------------------------------------------------------
// Process::putSignalAction
//
// Sets the action to be taken for a signal to the process

void Process::putSignalAction(int signal, uapi::sigaction action)
{
	m_sigactions->Set(signal, &action, nullptr);
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
	unsigned long			timeoutms;			// Thread attach timeout value

	auto vm = m_vm.lock();
	if(!vm) throw LinuxException(LINUX_ESRCH);

	// Any exeception that occurs during process start is considered fatal and results
	// in the process instance being terminated
	try {

		// Get the thread attach timeout value from the virtual machine properties
		try { timeoutms = std::stoul(vm->GetProperty(_VmOld::Properties::ThreadAttachTimeout)); }
		catch(...) { throw Exception(E_PROCESSINVALIDTHREADTIMEOUT); }

		// Start the process; there is no race condition to consider with the pending threads
		// collection here as the main thread is added during object construction above
		ResumeInternal();
	
		// Start is a special case in that the initial thread task state is added to the pending
		// threads collection during construction; there is no race condition with Resume to consider
		std::unique_lock<std::mutex> critsec(m_attachlock);
		if(m_pendingthreads.count(m_host->ThreadId) == 0) return;

		// Wait up to defined timeout in milliseconds for the main thread to attach and remove the element
		if(m_attached.wait_until(critsec, std::chrono::system_clock::now() + std::chrono::milliseconds(timeoutms), 
			[&]() -> bool { return m_pendingthreads.count(m_host->ThreadId) == 0; })) return;

		// Throw ESRCH / E_PROCESSTHREADTIMEOUT if the wait operation expired
		throw LinuxException(LINUX_ESRCH, Exception(E_PROCESSTHREADTIMEOUT, m_pid));
	}

	// Kill the entire process on exception, using SIGKILL as the exit code
	catch(...) { Terminate(uapi::MakeExitCode(0, LINUX_SIGKILL)); throw; }
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
	SuspendInternal();
	NotifyParent(Waitable::State::Stopped, uapi::STOPPED);
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
	NTSTATUS result = NtApi::NtSuspendProcess(m_host->Process->Handle);
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
	// Terminate all threads
	// TODO

	// Terminate the process and wait for it to actually exit
	TerminateProcess(m_host->Process->Handle, exitcode);
	WaitForSingleObject(m_host->Process->Handle, INFINITE);

	// Signal that the process has been terminated
	NotifyParent((exitcode & 0x80) ? Waitable::State::Dumped : Waitable::State::Killed, exitcode);
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

	// Attempt to locate the Thread instance associated with this TID
	const auto& iterator = m_threads.find(tid);
	if(iterator == m_threads.end()) throw LinuxException(LINUX_ESRCH);

	return iterator->second;
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
	m_memory->Release(address, length);
}

//-----------------------------------------------------------------------------
// Process::WaitChild
//
// Waits for a child process to signal a state change; this amalgamates all
// of the various wait() system calls into one master function
//
// Arguments:
//
//	type		- Type of PID indicated in the id argument
//	id			- Specific PID to wait for, depends on type argument
//	status		- Optionally receives the exit status for the process
//	options		- Wait operation options
//	siginfo		- Optionally receives the resultant signal information
//	rusage		- Optionally receives resource usage information for the child

uapi::pid_t Process::WaitChild(uapi::idtype_t type, uapi::pid_t id, int* status, int options, uapi::siginfo* siginfo, uapi::rusage* rusage)
{
	// If this process has specifically set SIGCHLD to ignored or has set SA_NOCLDWAIT
	// in the flags, WaitChild() only returns once there are no children left in this process
	uapi::sigaction sigchld = m_sigactions->Get(LINUX_SIGCHLD);
	if((sigchld.sa_handler == LINUX_SIG_IGN) || (sigchld.sa_flags & LINUX_SA_NOCLDWAIT)) {

		// Wait until there are no more child processes and throw ECHILD
		m_nochildren.WaitUntil(true);
		throw LinuxException(LINUX_ECHILD);
	}

	// Create a collection of waitable objects to operate against, ECHILD if none
	std::vector<std::shared_ptr<Waitable>> objects = CollectWaitables(type, id, options);
	if(objects.size() == 0) throw LinuxException(LINUX_ECHILD);

	// todo: Waitable can be pulled into Process now
	uapi::siginfo l_siginfo;
	std::shared_ptr<Process> result = std::static_pointer_cast<Process>(Waitable::Wait(objects, options, &l_siginfo));

	// Check if a child object was successfully waited on and returned
	if(result == nullptr) {

		if(options & LINUX_WNOHANG) return 0;			// WNOHANG - ok to return zero
		else throw LinuxException(LINUX_ECHILD);		// Not WNOHANG - ECHILD
	}

	// If the process has terminated, remove it after the successful wait
	Waitable::State state = static_cast<Waitable::State>(l_siginfo.si_code);
	if((state == Waitable::State::Dumped) || (state == Waitable::State::Exited) || (state == Waitable::State::Killed)) {

		// todo: this needs some work, but ok for now
		// will need to check for a 'subreaper' eventually too
		process_map_lock_t::scoped_lock_write writer(m_childlock);

		//m_vm->RemoveProcess(l_siginfo.linux_si_pid);

		// TODO: NEED TO RELEASE FROM PROCESS GROUP
		// this can't be the way I want to do this; this is awful
		auto pgroup = result->ProcessGroup;
		if(pgroup) pgroup->ReleaseProcess(l_siginfo.linux_si_pid);

		// this is ugly; need a better way
		m_children.erase(l_siginfo.linux_si_pid);
		if(m_children.size() == 0) m_nochildren = true;
	}

	// Optionally return the child process exit code and the signal information
	if(status) *status = l_siginfo.linux_si_status;
	if(siginfo) *siginfo = l_siginfo;

	// Optionally acquire RUSAGE_BOTH resource information for the child process,
	if(rusage) result->GetResourceUsage(LINUX_RUSAGE_BOTH, rusage);

	// Return the PID of the process that was successfully waited on
	return l_siginfo.linux_si_pid;
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
	return m_memory->Write(address, buffer, length);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
