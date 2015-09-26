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

#include "Capability.h"
#include "Executable.h"
#include "LinuxException.h"
#include "NativeProcess.h"
#include "Pid.h"
#include "ProcessGroup.h"
#include "Session.h"
#include "Thread.h"
#include "VirtualMachine.h"

#pragma warning(push, 4)

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

	// Spawning a new process requires root level access
	Capability::Demand(Capability::SystemAdmin);

	// Create an Executable::PathResolver lambda (prevents needing to pass all those arguments around)
	Executable::PathResolver resolver = [&](char_t const* path) -> std::shared_ptr<FileSystem::Handle> { return FileSystem::OpenExecutable(ns, root, working, path); };

	// Construct an Executable instance from the provided file system file
	auto executable = Executable::FromFile(resolver, path, arguments, environment);

	// Create a new Host instance of the appropriate architecture
	auto host = session->VirtualMachine->CreateHost(executable->Architecture);
	_ASSERTE(host->Architecture == executable->Architecture);

	try {

		// Load the executable image into the constructed host process instance
		auto layout = executable->Load(host.get(), 2 MiB);		// <--- todo: get stack size from virtual machine properties

		// layout has entry point and stack pointer in it

		// new way I want to do this ... create the Thread instance around the main thread handle/id, let it call acquire_thread
		// like any other thread would.  Doing it at time of process creation is a smidge faster but complicates the code, and there
		// is absolutely no reason to assume that performance won't be so horrible it would make any difference regardless
		//auto task = Task::Create(layout->Architecture, layout->EntryPoint, layout->StackPointer);

		// Attempt to allocate a new Local Descriptor Table for the process, the size is architecture dependent
		size_t ldtsize = LINUX_LDT_ENTRIES * ((host->Architecture == Architecture::x86) ? sizeof(uapi::user_desc32) : sizeof(uapi::user_desc64));
		try { ldtaddr = host->AllocateMemory(ldtsize, ProcessMemory::Protection::Read | ProcessMemory::Protection::Write); }
		catch(...) { throw LinuxException{ LINUX_ENOMEM }; }

		// Create the Process instance, providing a blank local descriptor table allocation bitmap
		process = std::make_shared<Process>(std::move(host), std::move(pid), session, pgroup, std::move(ns), ldtaddr, Bitmap(LINUX_LDT_ENTRIES), 
			std::move(root), std::move(working));
	}

	// Terminate the host process if any exceptions occurred during process creation
	catch(...) { if(host) host->Terminate(ERROR_PROCESS_ABORTED, true); throw; }

	// Establish links to the parent process group and session containers
	AddProcessGroupProcess(pgroup, process);
	AddSessionProcess(session, process);

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
