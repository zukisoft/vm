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

// INTERPRETER_SCRIPT_MAGIC
//
// Magic number present at the head of an interpreter script
static uint8_t INTERPRETER_SCRIPT_MAGIC[] = { 0x23, 0x21 };		// "#!"

// Process::Create<Architecture::x86>
//
// Explicit Instantiation of template function
template std::shared_ptr<Process> Process::Create<Architecture::x86>(const std::shared_ptr<VirtualMachine>&, uapi::pid_t pid, const FileSystem::AliasPtr&,
	const FileSystem::AliasPtr&, const FileSystem::HandlePtr&, const uapi::char_t**, const uapi::char_t**, const tchar_t*, const tchar_t*);

#ifdef _M_X64
// Process::Create<Architecture::x86_64>
//
// Explicit Instantiation of template function
template std::shared_ptr<Process> Process::Create<Architecture::x86_64>(const std::shared_ptr<VirtualMachine>&, uapi::pid_t pid, const FileSystem::AliasPtr&,
	const FileSystem::AliasPtr&, const FileSystem::HandlePtr&, const uapi::char_t**, const uapi::char_t**, const tchar_t*, const tchar_t*);
#endif

//-----------------------------------------------------------------------------
// Process Constructor
//
// Arguments:
//
//	TODO: DOCUMENT THEM

Process::Process(::Architecture architecture, std::unique_ptr<Host>&& host, std::shared_ptr<Thread>&& thread, uapi::pid_t pid, const FileSystem::AliasPtr& rootdir, const FileSystem::AliasPtr& workingdir, 
	std::unique_ptr<TaskState>&& taskstate, const void* ldt, Bitmap&& ldtslots, const std::shared_ptr<ProcessHandles>& handles, const std::shared_ptr<SignalActions>& sigactions, const void* programbreak) : 
	m_architecture(architecture), m_host(std::move(host)), m_pid(pid), m_rootdir(rootdir), m_workingdir(workingdir), m_taskstate(std::move(taskstate)), 
	m_ldt(ldt), m_handles(handles), m_sigactions(sigactions), m_programbreak(programbreak), m_ldtslots(std::move(ldtslots))
{
	m_nativethreadproc = nullptr;

	// Add the initial process thread to the collection
	m_threads[thread->ThreadId] = std::move(thread);
}

//-----------------------------------------------------------------------------
// Process::CheckHostArchitecture<x86> (static, private)
//
// Verifies that the created host process is 32-bit
//
// Arguments:
//
//	process		- Handle to the created host process

template <> inline void Process::CheckHostArchitecture<Architecture::x86>(HANDLE process)
{
	BOOL			result;				// Result from IsWow64Process

	// 32-bit systems can only create 32-bit processes; nothing to worry about
	if(SystemInformation::ProcessorArchitecture == SystemInformation::Architecture::Intel) return;

	// 64-bit system; verify that the process is running under WOW64
	if(!IsWow64Process(process, &result)) throw Win32Exception();
	if(!result) throw Exception(E_PROCESSINVALIDX86HOST);
}

//-----------------------------------------------------------------------------
// Process::CheckHostArchitecture<x86_64> (static, private)
//
// Verifies that the created host process is 64-bit
//
// Arguments:
//
//	process		- Handle to the created host process

#ifdef _M_X64
template <> inline void Process::CheckHostArchitecture<Architecture::x86_64>(HANDLE process)
{
	BOOL				result;				// Result from IsWow64Process

	// 64-bit system; verify that the process is not running under WOW64
	if(!IsWow64Process(process, &result)) throw Win32Exception();
	if(result) throw Exception(E_PROCESSINVALIDX64HOST);
}
#endif

//-----------------------------------------------------------------------------
// Process::Clone
//
// Clones the running process into a new child process
//
// Arguments:
//
// TODO
std::shared_ptr<Process> Process::Clone(const std::shared_ptr<VirtualMachine>& vm, uint32_t flags, void* taskstate, size_t taskstatelen)
{
	// need parent process to set that up, see CLONE_PARENT flag as well


	std::unique_ptr<TaskState> ts = TaskState::Create(m_architecture, taskstate, taskstatelen);
	std::shared_ptr<Process> child;
	uapi::pid_t pid = 0;

	m_host->Suspend();										// Suspend the parent process

	try {
		
		// Create the new host process
		std::unique_ptr<Host> host = Host::Create(vm->GetProperty(VirtualMachine::Properties::HostProcessBinary32).c_str(), 
			vm->GetProperty(VirtualMachine::Properties::HostProcessArguments).c_str(), nullptr, 0);

		try {

			host->CloneMemory(m_host);					// Clone the address space of the existing process

			// TODO: If CLONE_VM then the memory gets shared, not cloned

			pid = vm->AllocatePID();					// Allocate a new process identifier for the child

			try {

				// Create the file system handle collection for the child process, which may be shared or duplicated (CLONE_FILES)
				std::shared_ptr<ProcessHandles> childhandles = (flags & LINUX_CLONE_FILES) ? m_handles : ProcessHandles::Duplicate(m_handles);

				// Create the signal actions collection for the child process, which may be shared or duplicated (CLONE_SIGHAND)
				std::shared_ptr<SignalActions> childactions = (flags & LINUX_CLONE_SIGHAND) ? m_sigactions : SignalActions::Duplicate(m_sigactions);
						
				Bitmap ldtslots(m_ldtslots);
				// todo: Architecture is hard-coded
				
				/// removed temporarily
				///std::shared_ptr<Thread> thread = Thread::FromHandle<Architecture::x86>(host->ProcessHandle, pid, host->ThreadHandle, host->ThreadId);
				//child = std::make_shared<Process>(m_architecture, std::move(host), std::move(thread), pid, m_rootdir, m_workingdir, std::move(ts), m_ldt, std::move(ldtslots), childhandles, childactions, m_programbreak);
				child = std::make_shared<Process>(m_architecture, std::move(host), nullptr, pid, m_rootdir, m_workingdir, std::move(ts), m_ldt, std::move(ldtslots), childhandles, childactions, m_programbreak);
			}

			catch(...) { vm->ReleasePID(pid); throw; }
		}

		// todo: Terminate expects a LINUX error code, not a Windows one!
		catch(std::exception& ex) { host->Terminate(E_FAIL); throw; (ex); /* TODO */ }
	}

	// Ensure that the parent process is resumed in the event of an exception
	catch(...) { m_host->Resume(); throw; }

	// TESTING CHILDREN
	m_children.insert(std::make_pair(pid, child));

	m_host->Resume();						// Resume execution of the parent process
	child->Start();							// Start execution of the child process

	return child;
}

//-----------------------------------------------------------------------------
// Process::Create (static)
//
// Constructs a new process instance from an ELF binary
//
// Arguments:
//
//	vm			- Reference to the VirtualMachine instance
//	rootdir		- Initial process root directory alias
//	workingdir	- Initial process working directory alias
//	handle		- Open FileSystem::Handle against the ELF binary to load
//	argv		- ELF command line arguments from caller
//	envp		- ELF environment variables from caller
//	hostpath	- Path to the external host to load
//	hostargs	- Command line arguments to pass to the host

template <Architecture architecture>
std::shared_ptr<Process> Process::Create(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t pid, const FileSystem::AliasPtr& rootdir, 
	const FileSystem::AliasPtr& workingdir, const FileSystem::HandlePtr& handle, const uapi::char_t** argv, const uapi::char_t** envp, 
	const tchar_t* hostpath, const tchar_t* hostargs)
{
	using elf = elf_traits<architecture>;

	std::unique_ptr<ElfImage>		executable;				// The main ELF binary image to be loaded
	std::unique_ptr<ElfImage>		interpreter;			// Optional interpreter image specified by executable
	uint8_t							random[16];				// 16-bytes of random data for AT_RANDOM auxvec

	// Create the external host process (suspended by default) and verify the class/architecture
	// as this will all go south very quickly if it's not the expected architecture
	std::unique_ptr<Host> host = Host::Create(hostpath, hostargs, nullptr, 0);
	CheckHostArchitecture<architecture>(host->ProcessHandle);

	try {

		// The ELF loader requires the file handle to be at position zero
		handle->Seek(0, LINUX_SEEK_SET);

		// Generate the AT_RANDOM data to be associated with this process
		Random::Generate(random, 16);

		// Attempt to load the binary image into the process, then check for an interpreter
		executable = ElfImage::Load<architecture>(handle, host);
		if(executable->Interpreter) {

			// Acquire a handle to the interpreter binary and attempt to load that into the process
			bool absolute = (*executable->Interpreter == '/');
			FileSystem::HandlePtr interphandle = vm->OpenExecutable(rootdir, (absolute) ? rootdir : workingdir, executable->Interpreter);
			interpreter = ElfImage::Load<architecture>(interphandle, host);
		}

		// Construct the ELF arguments stack image for the hosted process
		ElfArguments args(argv, envp);

		(LINUX_AT_EXECFD);																		//  2 - TODO
		if(executable->ProgramHeaders) {

			args.AppendAuxiliaryVector(LINUX_AT_PHDR, executable->ProgramHeaders);				//  3
			args.AppendAuxiliaryVector(LINUX_AT_PHENT, sizeof(typename elf::progheader_t));		//  4
			args.AppendAuxiliaryVector(LINUX_AT_PHNUM, executable->NumProgramHeaders);			//  5
		}

		args.AppendAuxiliaryVector(LINUX_AT_PAGESZ, SystemInformation::PageSize);				//  6
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
		(LINUX_AT_EXECFN);																		// 31 - TODO -- WHATEVER WAS PASSED INTO EXECVE() GOES HERE
		(LINUX_AT_SYSINFO);																		// 32 - TODO MAY NOT IMPLEMENT?
		//args.AppendAuxiliaryVector(LINUX_AT_SYSINFO_EHDR, vdso->BaseAddress);					// 33 - TODO NEEDS VDSO

		// Allocate the stack for the process, using the currently set initial stack size for the Virtual Machine
		//// TODO: NEED TO GET STACK LENGTH FROM RESOURCE LIMITS SET FOR VIRTUAL MACHINE
		size_t stacklen = 2 MiB;
		const void* stack = host->AllocateMemory(stacklen, PAGE_READWRITE);

		// Place guard pages at the beginning and the end of the stack
		host->ProtectMemory(stack, SystemInformation::PageSize, PAGE_READONLY | PAGE_GUARD);
		host->ProtectMemory(reinterpret_cast<void*>(uintptr_t(stack) + stacklen - SystemInformation::PageSize), SystemInformation::PageSize, PAGE_READONLY | PAGE_GUARD);

		// Write the ELF arguments into the read/write portion of the process stack section and get the resultant pointer
		const void* stack_pointer = args.GenerateProcessStack<architecture>(host->ProcessHandle, reinterpret_cast<void*>(uintptr_t(stack) + SystemInformation::PageSize), 
			stacklen - (SystemInformation::PageSize * 2));

		// Load the remainder of the StartupInfo structure with the necessary information to get the ELF binary running
		const void* entry_point = (interpreter) ? interpreter->EntryPoint : executable->EntryPoint;
		const void* program_break = executable->ProgramBreak;

		// TODO LDT TEST
		// SIZE IS BASED ON CLASS (32 v 64)
		const void* ldt = host->AllocateMemory(LINUX_LDT_ENTRIES * sizeof(uapi::user_desc32), PAGE_READWRITE);

		// Create the main thread for the process, the initial thread's TID will always match the process PID

		/// removed temporarily
		//std::shared_ptr<Thread> thread = Thread::FromHandle<architecture>(host->ProcessHandle, pid, host->ThreadHandle, host->ThreadId);

		// this should abort on bad handle for now

		// TODO TESTING NEW STARTUP INFORMATION
		std::unique_ptr<TaskState> ts = TaskState::Create(architecture, entry_point, stack_pointer);
		std::shared_ptr<ProcessHandles> handles = ProcessHandles::Create();
		std::shared_ptr<SignalActions> actions = SignalActions::Create();

		// Create the Process object, transferring the host, startup information and allocated memory sections
		//return std::make_shared<Process>(architecture, std::move(host), std::move(thread), pid, rootdir, workingdir, std::move(ts), ldt, Bitmap(LINUX_LDT_ENTRIES), handles, actions, program_break);
		return std::make_shared<Process>(architecture, std::move(host), nullptr, pid, rootdir, workingdir, std::move(ts), ldt, Bitmap(LINUX_LDT_ENTRIES), handles, actions, program_break);
	}

	// Terminate the host process on exception since it doesn't get killed by the Host destructor
	// TODO: should be LinuxException wrapping the underlying one?  Only case right now where
	// that wouldn't be true is interpreter's OpenExec() call which should already throw LinuxExceptions
	// TODO: exit code should be a LINUX exit code, not a Windows one
	catch(...) { host->Terminate(E_FAIL); throw; }
}

//-----------------------------------------------------------------------------
// Process::Execute
//
// Executes a new binary by replacing the contents of this process
//

void Process::Execute(const std::shared_ptr<VirtualMachine>& vm, const char_t* filename, const char_t* const& argv, const char_t* const& envp)
{
	(argv);
	(envp);
	(filename);
	(vm);

	m_host->Suspend();						// Suspend the native process

	try {

		// If the architecture is the same, can re-use the host process otherwise
		// a new host process will need to be constructed

		// Kill all threads
		// ? do they need to be signaled

		// Remove all file handles that are set for close-on-exec
		m_handles->RemoveCloseOnExecute();
		
		// Release all allocated memory
		m_host->ClearMemory();

		// TODO: Actually implement this
	}
	
	catch(...) { m_host->Resume(); throw; }
}

//-----------------------------------------------------------------------------
// Process::FindNativeThread
//
// Locates a thread within this process based on its native thread identifier
//
// Arguments:
//
//	nativetid		- Native thread identifier

std::shared_ptr<Thread> Process::FindNativeThread(DWORD nativetid)
{
	thread_map_lock_t::scoped_lock_read reader(m_threadslock);

	_ASSERTE(false);
	// Iterate over the collection to find the native thread
	//for(const auto iterator : m_threads)
	//	if(iterator.second->NativeThreadId == nativetid) return iterator.second;

	// Specified thread is not a thread within this process
	return nullptr;
}

//-----------------------------------------------------------------------------
// Process::FindThread
//
// Locates a thread within this process based on its thread identifier
//
// Arguments:
//
//	tid			- Thread identifier

std::shared_ptr<Thread> Process::FindThread(uapi::pid_t tid)
{
	thread_map_lock_t::scoped_lock_read reader(m_threadslock);

	// Locate the thread within the collection
	const auto found = m_threads.find(tid);
	return (found != m_threads.end()) ? found->second : nullptr;
}

//-----------------------------------------------------------------------------
// Process::GetInitialTaskState
//
// Gets a copy of the original task state information for the process, the
// contents of which varies based on if the process is 32 or 64-bit
//
// Arguments:
//
//	taskstate	- Pointer to receive the task state information
//	length		- Length of the startup information buffer

void Process::GetInitialTaskState(void* startinfo, size_t length)
{
	return m_taskstate->CopyTo(startinfo, length);
}

//-----------------------------------------------------------------------------
// Process::getLocalDescriptorTable
//
// Gets the address of the local descriptor table in the process

const void* Process::getLocalDescriptorTable(void) const
{
	return m_ldt;
}

//-----------------------------------------------------------------------------
// Process::MapMemory
//
// Creates a memory mapping for the process
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
	FileSystem::HandlePtr		handle = nullptr;		// Handle to the file object

	_ASSERTE(m_host);
	_ASSERTE(m_host->ProcessHandle);

	// MAP_HUGETLB is not currently supported, but may be possible in the future
	if(flags & LINUX_MAP_HUGETLB) throw LinuxException(LINUX_EINVAL);

	// MAP_GROWSDOWN is not supported
	if(flags & LINUX_MAP_GROWSDOWN) throw LinuxException(LINUX_EINVAL);

	// Suggested base addresses are not supported, switch the address to NULL if MAP_FIXED is not set
	if((flags & LINUX_MAP_FIXED) == 0) address = nullptr;

	// Non-anonymous mappings require a valid file descriptor to be specified
	if(((flags & LINUX_MAP_ANONYMOUS) == 0) && (fd <= 0)) throw LinuxException(LINUX_EBADF);
	if(fd > 0) handle = GetHandle(fd)->Duplicate(LINUX_O_RDONLY);

	// Attempt to allocate the process memory and adjust address to that base if it was NULL
	const void* base = m_host->AllocateMemory(address, length, uapi::LinuxProtToWindowsPageFlags(prot));
	if(address == nullptr) address = base;

	// If a file handle was specified, copy data from the file into the allocated region,
	// private mappings need not be concerned with writing this data back to the file
	if(handle != nullptr) {

		uintptr_t	dest = uintptr_t(address);			// Easier pointer math as uintptr_t
		size_t		read;								// Bytes read from the source file
		SIZE_T		written;							// Result from NtWriteVirtualMemory

		// TODO: Both tempfs and hostfs should be able to handle memory mappings now with
		// the introduction of sections; this would be much more efficient that way.  Such
		// a call would need to fail for FS that can't support it, and fall back to this
		// type of implementation.  Also evaluate the same in ElfImage, mapping the source
		// file would be better than having to read it from a stream

		HeapBuffer<uint8_t> buffer(SystemInformation::AllocationGranularity);	// 64K buffer

		// Seek the file handle to the specified offset
		if(handle->Seek(offset, LINUX_SEEK_SET) != offset) throw LinuxException(LINUX_EINVAL);

		do {
			
			// Copy the next chunk of bytes from the source file into the process' memory space
			read = handle->Read(buffer, min(length, buffer.Size));
			NTSTATUS result = NtApi::NtWriteVirtualMemory(m_host->ProcessHandle, reinterpret_cast<void*>(dest), buffer, read, &written);
			if(result != NtApi::STATUS_SUCCESS) throw LinuxException(LINUX_EACCES, StructuredException(result));

			dest += written;						// Increment destination pointer
			length -= read;							// Decrement remaining bytes to be copied

		} while((length > 0) && (read > 0));
	};

	// MAP_LOCKED - Attempt to lock the memory into the process working set
	if(flags & LINUX_MAP_LOCKED) {

		// Make an attempt to satisfy this request, but don't throw an exception if it fails
		void* lockaddress = const_cast<void*>(address);
		SIZE_T locklength = length;
		NtApi::NtLockVirtualMemory(m_host->ProcessHandle, &lockaddress, &locklength, NtApi::MAP_PROCESS);
	}

	return address;
}

//-----------------------------------------------------------------------------
// Process::getNativeHandle
//
// Gets the native operating system handle for the process

HANDLE Process::getNativeHandle(void) const
{
	return m_host->ProcessHandle;
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
// Gets the address of the native thread entry point for the process

void* Process::getNativeThreadProc(void) const
{
	return m_nativethreadproc;
}

//-----------------------------------------------------------------------------
// Process::getNativeThreadProc
//
// Gets the address of the native thread entry point for the process

void Process::putNativeThreadProc(void* value)
{
#ifdef _M_X64
	if((m_architecture == Architecture::x86) && (uintptr_t(value) > UINT32_MAX)) throw Exception(E_PROCESSINVALIDTHREADPROC);
#endif

	m_nativethreadproc = value;
}

//-----------------------------------------------------------------------------
// Process::getParentProcessId

uapi::pid_t Process::getParentProcessId(void) const
{
	// If the parent process is still alive, return that identifier, otherwise
	// the parent process becomes the init process (id 1)
	std::shared_ptr<Process> parent = m_parent.lock();
	return (parent) ? parent->ProcessId : VirtualMachine::PROCESSID_INIT;
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
	m_host->WriteMemory(reinterpret_cast<void*>(destination), u_info, sizeof(uapi::user_desc32));

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
			if(delta > 0) m_host->AllocateMemory(oldbreak, delta, PAGE_READWRITE);
			else m_host->ReleaseMemory(newbreak, delta);
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
	// SignalActions has a convenience function for exactly this purpose
	m_sigactions->Set(signal, action, oldaction);
}

//-----------------------------------------------------------------------------
// Process::Signal
//
// Sends a signal to the process
//
// Arguments:
//
//	signal		- The signal to send to the process

void Process::Signal(int signal)
{
	// most of this is moving to Thread

	/// README:
	// http://www.linuxprogrammingblog.com/all-about-linux-signals?page=show
	///

	if(signal > LINUX__NSIG) throw LinuxException(LINUX_EINVAL);

	// Retrieve the currently action for this signal
	uapi::sigaction action = m_sigactions->Get(signal);
	m_threads.at(m_pid)->BeginSignal(signal, action);

	//// SIGKILL cannot be masked
	//if(signal == LINUX_SIGKILL) {
	//}

	//// SIGSTOP cannot be masked
	//else if(signal == LINUX_SIGSTOP) {
	//}

	//// SIGCONT ?

	//// SIGCHLD also special

	//// Real-time signals range from SIGRTMIN to SIGRTMAX, behavior is different

	//// If the signal has been set to IGNORE don't do anything
	//if(action.sa_handler == LINUX_SIG_IGN) return;

	//// If the signal has been set to DEFAULT, use default behaviors
	//if(action.sa_handler == LINUX_SIG_DFL) return; //{
}

//-----------------------------------------------------------------------------
// Process::Start
//
// Starts the process
//
// Arguments:
//
//	NONE

void Process::Start(void)
{
	return m_host->Resume();
}

//-----------------------------------------------------------------------------
// Process::UnmapMemory
//
// Releases a memory region allocated with MapMemory
//
// Arguments:
//
//	address		- Base address of the region to be released
//	length		- Length of the region to be released

void Process::UnmapMemory(void* address, size_t length)
{
	m_host->ReleaseMemory(address, length);
}

uapi::pid_t Process::RegisterThread_TEST(DWORD nativeid)
{
	// JUST A TEST - THIS IS GARBAGE
	m_threadid_TEST = nativeid;
	return 400;
}

uapi::pid_t Process::WaitChild_TEST(uapi::pid_t pid, int* status)
{
	// JUST A TEST - THIS IS GARBAGE

	(pid);
	uapi::pid_t waitpid = 0;
	std::vector<HANDLE> handles;
	for(auto iterator : m_children) {

		std::shared_ptr<Process> child = iterator.second.lock();
		if(child) waitpid = child->ProcessId;
		if(child) handles.push_back(child->m_host->ProcessHandle);
	}

	if(handles.size() == 0) { 
		return -LINUX_ECHILD; 
	}

	// this will also need to clean up zombie processes/threads

	WaitForMultipleObjects(handles.size(), handles.data(), FALSE, INFINITE);
	int x = 123; (x);

	// reproduce status for WIFxxx macros here
	// 0x0000 = terminated normally

	if(status) *status = 0;

	return waitpid;
}

//-----------------------------------------------------------------------------
// Process::getZombie

bool Process::getZombie(void) const
{
	// If the host process has terminated, this process is a zombie
	return (WaitForSingleObject(m_host->ProcessHandle, 0) == WAIT_OBJECT_0);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
