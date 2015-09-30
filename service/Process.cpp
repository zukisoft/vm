//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"}; to deal
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

#include <tuple>
#include "Capability.h"
#include "Executable.h"
#include "LinuxException.h"
#include "NativeProcess.h"
#include "NativeThread.h"
#include "Pid.h"
#include "ProcessGroup.h"
#include "Session.h"
#include "Thread.h"
#include "VirtualMachine.h"

#pragma warning(push, 4)

// Process::s_pendingmap (static)
//
// Collection of processes pending attach
Process::pendingmap_t Process::s_pendingmap;

// Process::s_pendingcond (static)
//
// Condition variable used to signal pending process attach
std::condition_variable	Process::s_pendingcond;

// Process::s_pendinglock (static)
//
// Synchronization object for the pending condition variable
std::mutex Process::s_pendinglock;

//-----------------------------------------------------------------------------
// AddProcessThread
//
// Adds a thread to a Process instance
//
// Arguments:
//
//	process		- Process instance to operate against
//	thread		- Thread instance to be added

std::shared_ptr<Process> AddProcessThread(std::shared_ptr<Process> process, std::shared_ptr<Thread> thread)
{
	sync::critical_section::scoped_lock cs{ process->m_cs };
	if(!process->m_threads.emplace(thread.get(), thread).second) throw LinuxException{ LINUX_ENOMEM };

	return process;
}

//-----------------------------------------------------------------------------
// RemoveProcessThread
//
// Removes a thread from a Process instance
//
// Arguments:
//
//	process		- Process instance to operate against
//	thread		- Thread instance to be removed

void RemoveProcessThread(std::shared_ptr<Process> process, Thread const* thread)
{
	sync::critical_section::scoped_lock cs{ process->m_cs };
	process->m_threads.erase(thread);
}

//-----------------------------------------------------------------------------
// Process Constructor (private)
//
// Arguments:
//
//	nativeproc	- NativeProcess instance to take ownership of
//	pid			- Process identifier to assign to the process
//	session		- Session in which the process will be a member
//	pgroup		- ProcessGroup in which the process will be a member
//	namespace	- Namespace to associate with this process
//	ldtaddr		- Address of process local descriptor table
//	ldtslots	- Local descriptor table allocation bitmap
//	root		- Initial root path for this process
//	working		- Initial working path for this process

Process::Process(nativeproc_t nativeproc, pid_t pid, session_t session, pgroup_t pgroup, namespace_t ns, uintptr_t ldtaddr, Bitmap&& ldtslots, fspath_t root, fspath_t working) : 
	m_nativeproc(std::move(nativeproc)), m_pid(std::move(pid)), m_session(std::move(session)), m_pgroup(std::move(pgroup)), m_ns(std::move(ns)), 
	m_ldtaddr(ldtaddr), m_ldtslots(std::move(ldtslots)), m_root(std::move(root)), m_working(std::move(working))
{
	m_state = State::Pending;			// Process is pending by default
}

//-----------------------------------------------------------------------------
// Process Destructor

Process::~Process()
{
	RemoveProcessGroupProcess(m_pgroup, this);
	RemoveSessionProcess(m_session, this);
}

//-----------------------------------------------------------------------------
// Process::getArchitecture
//
// Gets the architecture flag for this process

enum class Architecture Process::getArchitecture(void) const
{
	return m_nativeproc->Architecture;
}

//-----------------------------------------------------------------------------
// Process::Attach (static)
//
// Attaches a native process to this Process instance
//
// Arguments:
//
//	nativepid		- Native process identifier

std::shared_ptr<Process> Process::Attach(DWORD nativepid)
{
	std::unique_lock<std::mutex> critsec{ s_pendinglock };

	// Attempt to locate the process in the pending process collection
	auto found = s_pendingmap.find(nativepid);
	if(found == s_pendingmap.end()) return nullptr;

	// Pull out the shared_ptr for the process and remove the pending element
	auto process = found->second;
	s_pendingmap.erase(found);

	// Notify all waiting threads to reevaluate the condition
	s_pendingcond.notify_all();
	critsec.unlock();

	// Wait up to the configured timeout for the process to reach a Running state
	// todo: this value needs to be configurable, can get to VirtualMachine from Process
	if(!process->WaitForState(State::Running, 10000)) return nullptr;

	return process;
}

//-----------------------------------------------------------------------------
// Process::Create (static)
//
// Creates a new process instance
//
// Arguments:
//
//	pid				- Process identifier to assign to the process
//	session			- Session that the process will become a member of
//	group			- Process group that the process will become a member of
//	ns				- Namespace to assign to the process and use for path lookups
//	root			- Root path to assign to the process and use for path lookups
//	working			- Working path to assign to the process and use for path lookups
//	path			- Path to the executable
//	arguments		- Array of process command line arguments
//	environment		- Array of process environment variables

std::shared_ptr<Process> Process::Create(std::shared_ptr<Pid> pid, std::shared_ptr<class Session> session, std::shared_ptr<class ProcessGroup> pgroup,
		std::shared_ptr<class Namespace> ns, std::shared_ptr<FileSystem::Path> root, std::shared_ptr<FileSystem::Path> working, char_t const* path,
		char_t const* const* arguments, char_t const* const* environment)
{
	uintptr_t							ldtaddr(0);				// Local descriptor table address
	std::shared_ptr<Process>			process;				// The constructed Process instance

	Capability::Demand(Capability::SystemAdmin);				// Only root can spawn a process directly

	// Create an Executable::PathResolver lambda (prevents needing to pass all those arguments around)
	Executable::PathResolver resolver = [&](char_t const* path) -> std::shared_ptr<FileSystem::Handle> { return FileSystem::OpenExecutable(ns, root, working, path); };

	// Construct an Executable instance from the provided file system file
	auto executable = Executable::FromFile(resolver, path, arguments, environment);

	// Create a new hosting process/thread of the appropriate architecture
	std::unique_ptr<NativeProcess> nativeprocess;
	std::unique_ptr<NativeThread> nativethread;
	std::tie(nativeprocess, nativethread) = session->VirtualMachine->CreateHost(executable->Architecture);

	try {

		// Load the executable image into the constructed host process instance
		auto layout = executable->Load(nativeprocess.get(), 2 MiB);		// <--- todo: get stack size from virtual machine properties

		// Attempt to allocate a new Local Descriptor Table for the process, the size is architecture dependent
		size_t ldtsize = LINUX_LDT_ENTRIES * ((nativeprocess->Architecture == Architecture::x86) ? sizeof(uapi::user_desc32) : sizeof(uapi::user_desc64));
		try { ldtaddr = nativeprocess->AllocateMemory(ldtsize, ProcessMemory::Protection::Read | ProcessMemory::Protection::Write); }
		catch(...) { throw LinuxException{ LINUX_ENOMEM }; }

		// Create the Process instance, providing a blank local descriptor table allocation bitmap
		process = std::make_shared<Process>(std::move(nativeprocess), std::move(pid), session, pgroup, std::move(ns), ldtaddr, Bitmap(LINUX_LDT_ENTRIES), 
			std::move(root), std::move(working));
	}

	// Kill the process with ERROR_PROCESS_ABORTED if there was a problem before it becomes a Process instance
	catch(...) { nativeprocess->Terminate(ERROR_PROCESS_ABORTED, true); throw; }

	//
	// todo: this is pretty ugly down here, I think there needs to be a helper function, perhaps make "Start"
	// which can be called publicly and stop this Create() function after the process is created (kinda makes
	// sense when you think about it, huh?)
	//

	// The Process instance was successfully created and has taken ownership of everything.  Start the
	// native process and wait for it to attach to the virtual machine
	try {

		// todo: ensure state is Pending here if moving into a new function

		DWORD nativepid = process->m_nativeproc->ProcessId;

		// Insert an entry into the pending process collection and start the native process
		std::unique_lock<std::mutex> critsec{ s_pendinglock };
		s_pendingmap.emplace(nativepid, process);
		process->m_nativeproc->Resume();

		// Wait for the process to attach and remove the shared_ptr placed into the collection
		// todo: get timeout value from virtual machine properties
		if(!s_pendingcond.wait_until(critsec, std::chrono::system_clock::now() + std::chrono::milliseconds(30000),
			[=]() -> bool { return s_pendingmap.count(nativepid) == 0; })) {

			// The process failed to attach, remove the pending entry and throw
			// todo: custom exception -- this is important to know what happened
			throw LinuxException{ LINUX_ENOEXEC, Win32Exception{ ERROR_PROCESS_ABORTED } };	// todo: custom exception
		}

		// The process has attached successfully, add links to the session and group
		AddProcessGroupProcess(pgroup, process);
		AddSessionProcess(session, process);

		process->SetState(State::Running);				// Process is now running
	}

	catch(...) { /* TODO: KILL THE PROCESS */ throw; }

	return process;
}

//-----------------------------------------------------------------------------
// Process::getLocalDescriptorTableAddress
//
// Gets the address of the local descriptor table for this process

uintptr_t Process::getLocalDescriptorTableAddress(void) const
{
	return m_ldtaddr;
}

//-----------------------------------------------------------------------------
// Process::getNamespace
//
// Gets the namespace associated with this process

std::shared_ptr<class Namespace> Process::getNamespace(void) const
{
	return m_ns;
}

//-----------------------------------------------------------------------------
// Process::getProcessGroup
//
// Gets the process group in which this process is a member

std::shared_ptr<class ProcessGroup> Process::getProcessGroup(void) const
{
	// The process group is non-const for a process and requires synchronization
	sync::critical_section::scoped_lock cs{ m_cs };
	return m_pgroup;
}

//-----------------------------------------------------------------------------
// Process::getProcessId
//
// Gets the process identifier

std::shared_ptr<Pid> Process::getProcessId(void) const
{
	return m_pid;
}

//-----------------------------------------------------------------------------
// Process::getRootPath
//
// Gets the root path for the process

std::shared_ptr<FileSystem::Path> Process::getRootPath(void) const
{
	// The root path is non-const for a process and requires synchronization
	sync::critical_section::scoped_lock cs{ m_cs };
	return m_root;
}

//-----------------------------------------------------------------------------
// Process::getSession
//
// Gets the session in which this process is a member

std::shared_ptr<class Session> Process::getSession(void) const
{
	// The session is non-const for a process and requires synchronization
	sync::critical_section::scoped_lock cs{ m_cs };
	return m_session;
}

//-----------------------------------------------------------------------------
// Process::SetProcessGroup
//
// Changes the process group that this process is a member of
//
// Arguments:
//
//	pgroup		- New process group instance

void Process::SetProcessGroup(std::shared_ptr<class ProcessGroup> pgroup)
{
	sync::critical_section::scoped_lock cs{ m_cs };
	m_pgroup = SwapProcessGroupProcess(m_pgroup, pgroup, this);
}

//-----------------------------------------------------------------------------
// Process::SetSession
//
// Changes the session that this process is a member of
//
// Arguments:
//
//	session		- New session instance
//	pgroup		- New process group instance

void Process::SetSession(std::shared_ptr<class Session> session, std::shared_ptr<class ProcessGroup> pgroup)
{
	sync::critical_section::scoped_lock cs{ m_cs };
	m_session = SwapSessionProcess(m_session, session, this);
	m_pgroup = SwapProcessGroupProcess(m_pgroup, pgroup, this);
}

//-----------------------------------------------------------------------------
// Process::getState
//
// Gets the state of the process

enum class Process::State Process::getState(void) const
{
	// Allow a dirty read of the state variable, locking the mutex here wouldn't do
	// anything useful -- value could still change between the time this function
	// ends (releasing the mutex) and the time the caller examines the result
	return m_state;
}

//-----------------------------------------------------------------------------
// Process::SetState (private)
//
// Sets the state of the process and signals the condition variable
//
// Arguments:
//
//	state		- New state of the process

void Process::SetState(enum class State state)
{
	std::unique_lock<std::mutex> critsec{ m_statelock };
	m_state = state;
	m_statecond.notify_all();
}

//-----------------------------------------------------------------------------
// Process::WaitForState
//
// Waits for the process to reach a specific state
//
// Arguments:
//
//	state		- Process state to wait for
//	timeoutms	- Timeout for the wait operation, in milliseconds

bool Process::WaitForState(enum class State state, uint32_t timeoutms) const
{
	std::unique_lock<std::mutex> critsec{ m_statelock };
	return m_statecond.wait_until(critsec, std::chrono::system_clock::now() + std::chrono::milliseconds(timeoutms), 
		[&]() -> bool { return m_state == state; });
}

//-----------------------------------------------------------------------------
// Process::getWorkingPath
//
// Gets the working path for the process

std::shared_ptr<FileSystem::Path> Process::getWorkingPath(void) const
{
	// The working path is non-const for a process and requires synchronization
	sync::critical_section::scoped_lock cs{ m_cs };
	return m_working;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
