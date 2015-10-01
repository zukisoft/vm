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

// g_pendingmap (local)
//
// Collection of processes pending attach
static std::unordered_map<DWORD, std::shared_ptr<Process>> g_pendingmap;

// g_pendingcond (local)
//
// Condition variable used to signal pending process attach
static std::condition_variable g_pendingcond;

// g_pendinglock (local)
//
// Synchronization object for the pending condition variable
static std::mutex g_pendinglock;

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
// AttachProcess
//
// Attaches a native process to this Process instance
//
// Arguments:
//
//	nativepid		- Native process identifier

std::shared_ptr<Process> AttachProcess(DWORD nativepid)
{
	uapi::siginfo		siginfo;			// Process wait operation data

	std::unique_lock<std::mutex> critsec{ g_pendinglock };

	// Attempt to locate the process in the pending process collection
	auto found = g_pendingmap.find(nativepid);
	if(found == g_pendingmap.end()) return nullptr;

	// Pull out the shared_ptr for the process and remove the pending element
	auto process = found->second;
	g_pendingmap.erase(found);

	// Notify all waiting threads to reevaluate the condition
	g_pendingcond.notify_all();
	critsec.unlock();

	// Wait for the process to signal that it is running (StartProcess simulates a SIGCONT)
	Process::Wait({ process }, LINUX_WEXITED | LINUX_WSTOPPED | LINUX_WCONTINUED, &siginfo);
	if(siginfo.si_code != LINUX_CLD_CONTINUED) throw Exception{ E_FAIL };		// <-- todo: exception

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
// StartProcess
//
// Starts a process and waits for it to attach to the Process instance
//
// Arguments:
//
//	process		- Process instance to be started
//	timeoutms	- Timeout value for the operation in milliseconds

void StartProcess(std::shared_ptr<Process> process, uint32_t timeoutms)
{
	DWORD nativepid = process->m_nativeproc->ProcessId;

	std::unique_lock<std::mutex> critsec{ g_pendinglock };

	// Insert an entry into the pending process collection and start the native process
	g_pendingmap.emplace(nativepid, process);
	process->m_nativeproc->Resume();

	// Wait for the process to attach and remove the shared_ptr placed into the collection
	if(!g_pendingcond.wait_until(critsec, std::chrono::system_clock::now() + std::chrono::milliseconds(timeoutms),
		[=]() -> bool { return g_pendingmap.count(nativepid) == 0; })) {

		// The process failed to attach in the time specified, remove the entry and throw
		g_pendingmap.erase(nativepid);
		throw LinuxException{ LINUX_ENOEXEC, Win32Exception{ ERROR_PROCESS_ABORTED } };
	}
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
	// Initialize the pending state change signal information
	memset(&m_statepending, 0, sizeof(uapi::siginfo));
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

	// Start the Process instance and wait for the native process to attach to it
	try { StartProcess(process, 30000); }	// <-- todo: get timeout from virtual machine properties
	catch(...) { /* TODO: TERMINATE PROCESS USING PROCESS->KILL()/TERMINATE() */ throw; }

	AddProcessGroupProcess(pgroup, process);		// Link to the process group
	AddSessionProcess(session, process);			// Link to the session

	// Indicate that the process is now running by simulating a SIGCONT
	process->NotifyStateChange(statechange_t::continued, 0);

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
// Process::NotifyStateChange (protected)
//
// Signals that a change in process state has occurred
//
// Arguments:
//
//	newstate	- New state to be reported for the object
//	status		- Current object status/exit code

void Process::NotifyStateChange(statechange_t newstate, int32_t status)
{
	uapi::siginfo		siginfo;			// Signal information (SIGCHLD)

	// Convert the input arguments into a siginfo (SIGCHLD) structure
	siginfo.si_signo = LINUX_SIGCHLD;
	siginfo.si_errno = 0;
	siginfo.si_code = static_cast<int32_t>(newstate);
	siginfo.linux_si_pid = m_pid->getValue(m_ns);
	siginfo.linux_si_uid = 0;
	siginfo.linux_si_status = status;

	// Lock the waiters collection and pending siginfo member variables
	std::lock_guard<std::mutex> cscollection{ m_statelock };

	// Revoke any pending signal that was not processed by a waiter
	memset(&m_statepending, 0, sizeof(uapi::siginfo));

	for(auto& iterator : m_waiters) {

		// Take the lock for this waiter and check that it hasn't already been spent,
		// this is necessary to prevent a race condition wherein a second call to this
		// function before the waiter could be removed would resignal it
		std::unique_lock<std::mutex> cswaiter{ iterator.lock };
		if(iterator.siginfo->linux_si_pid != 0) continue;

		// If this waiter is not interested in this state change, move on to the next one
		if(!WaitOperationAcceptsStateChange(iterator.options, newstate)) continue;

		// Write the signal information out through the provided pointer and assign
		// the context object of this waiter to the shared result object reference
		*iterator.siginfo = siginfo;
		iterator.result = iterator.process;

		// Signal the waiter's condition variable that a result is available
		iterator.signal.notify_one();

		// If WNOWAIT has not been specified by this waiter, the signal is consumed
		if((iterator.options & LINUX_WNOWAIT) == 0) return;
	}
	
	m_statepending = siginfo;		// Save the signal for another waiter (WNOWAIT)
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
// Process::Wait (private, static)
//
// Waits for a Process instance to become signaled
//
// Arguments:
//
//	processes	- Vector of Process instances to be waited upon
//	options		- Wait operation flags and options
//	siginfo		- On success, contains resultant signal information

std::shared_ptr<Process> Process::Wait(std::vector<std::shared_ptr<Process>> const& processes, int options, uapi::siginfo* siginfo)
{
	std::condition_variable			signal;				// Signaled on a successful wait
	std::mutex						lock;				// Condition variable synchronization object
	std::shared_ptr<Process>		signaled;			// Process that was signaled

	// The caller has to be willing to wait for something otherwise this would never return
	_ASSERTE(options & (LINUX_WEXITED | LINUX_WSTOPPED | LINUX_WCONTINUED));
	if(options & ~(LINUX_WEXITED | LINUX_WSTOPPED | LINUX_WCONTINUED)) throw LinuxException{ LINUX_EINVAL };

	// The si_pid field of the signal information is used to detect spurious condition variable
	// wakes as well as preventing it from being signaled multiple times; initialize it to zero
	_ASSERTE(siginfo);
	siginfo->linux_si_pid = 0;

	// At least one Process instance must have been provided
	_ASSERTE(processes.size());
	if(processes.size() == 0) return nullptr;

	// Take the condition variable lock before adding any waiters
	std::unique_lock<std::mutex> cscondvar{ lock };

	// Iterate over all of the Process instances to check for a pending signal that can be
	// consumed immediately, or to register a wait operation against it
	for(auto const& iterator : processes) {

		std::lock_guard<std::mutex> cswaitable{ iterator->m_statelock };

		// If there is already a pending state for this Process instance that matches the
		// requested wait operation mask, pull it out and stop registering waits
		if((iterator->m_statepending.linux_si_pid != 0) && (WaitOperationAcceptsStateChange(options, static_cast<statechange_t>(iterator->m_statepending.si_code)))) {

			// Pull out the pending signal information and assign the result object
			*siginfo = iterator->m_statepending;
			signaled = iterator;

			// If WNOWAIT has not been specified, reset the pending signal information
			if((options & LINUX_WNOWAIT) == 0) memset(&iterator->m_statepending, 0, sizeof(uapi::siginfo));

			signal.notify_one();			// Condition will signal immediately if waited upon
			break;							// No need to register any more waiters
		}

		// Unless WNOHANG has been specified, register a wait operation for this Process instance
		if((options & LINUX_WNOHANG) == 0) iterator->m_waiters.push_back({ signal, lock, options, iterator, siginfo, signaled });
	}

	// If WNOHANG was specified, nothing will have been registered to wait against; only a
	// consumed pending signal from a Process instance is considered as a result
	if(options & LINUX_WNOHANG) return signaled;

	// Wait indefinitely for the condition variable to become signaled and retake the lock
	signal.wait(cscondvar, [&]() -> bool { return siginfo->linux_si_pid != 0; });

	// Remove this wait from the provided Process instances
	for(auto const& iterator : processes) {

		std::lock_guard<std::mutex> critsec{ iterator->m_statelock };
		iterator->m_waiters.remove_if([&](waiter_t const& item) -> bool { return &item.signal == &signal; });
	}

	return signaled;			// Return Process instance that was signaled
}

//-----------------------------------------------------------------------------
// Process::WaitOperationAcceptsStateChange (private, static)
//
// Determines if a wait options mask accepts a specific state change code
//
// Arguments:
//
//	mask		- Wait operation flags and options mask
//	newstate	- StateChange code to check against the provided mask

inline bool Process::WaitOperationAcceptsStateChange(int mask, statechange_t newstate)
{
	switch(newstate) {

		// WEXITED -- Accepts exited, killed, dumped
		//
		case statechange_t::exited:
		case statechange_t::killed:
		case statechange_t::dumped:
			return ((mask & LINUX_WEXITED) == LINUX_WEXITED);

		// WSTOPPED -- Accepts trapped, stopped
		//
		case statechange_t::trapped:
		case statechange_t::stopped:
			return ((mask & LINUX_WSTOPPED) == LINUX_WSTOPPED);

		// WCONTINUED -- Accepts continued
		//
		case statechange_t::continued:
			return ((mask & LINUX_WCONTINUED) == LINUX_WCONTINUED);
	}

	return false;
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
