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

#ifndef __PROCESS_H_
#define __PROCESS_H_
#pragma once

#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "Architecture.h"
#include "Bitmap.h"
#include "FileSystem.h"

#pragma warning(push, 4)

// Forward Declarations
//
class Namespace;
class NativeProcess;
class Pid;
class ProcessGroup;
class Session;
class Thread;

//-----------------------------------------------------------------------------
// Process
//
// Implements a virtual machine process/thread group instance

class Process
{
public:

	// Destructor
	//
	~Process();

	//-------------------------------------------------------------------------
	// Friend Functions

	// AddProcessThread
	//
	// Adds a thread to the collection
	friend std::shared_ptr<Process> AddProcessThread(std::shared_ptr<Process> process, std::shared_ptr<Thread> thread);

	// AttachProcess
	//
	// Attaches a native process to a pending Process instance
	friend std::shared_ptr<Process> AttachProcess(DWORD nativepid);

	// RemoveProcessThread
	//
	// Removes a thread from the collection
	friend void RemoveProcessThread(std::shared_ptr<Process> process, Thread const* thread);

	// StartProcess
	//
	// Starts a Process and waits for it to attach from the native process
	friend void StartProcess(std::shared_ptr<Process> process, uint32_t timeoutms);

	//-------------------------------------------------------------------------
	// Member Functions

	// Create (static)
	//
	// Creates a new Process instance
	static std::shared_ptr<Process> Create(std::shared_ptr<Pid> pid, std::shared_ptr<class Session> session, std::shared_ptr<class ProcessGroup> pgroup,
		std::shared_ptr<class Namespace> ns, std::shared_ptr<FileSystem::Path> root, std::shared_ptr<FileSystem::Path> working, char_t const* path,
		char_t const* const* arguments, char_t const* const* environment);

	// SetProcessGroup
	//
	// Changes the process group that this process is a member of
	void SetProcessGroup(std::shared_ptr<class ProcessGroup> pgroup);

	// SetSession
	//
	// Changes the session that this process is a member of
	void SetSession(std::shared_ptr<class Session> session, std::shared_ptr<class ProcessGroup> pgroup);

	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the architecture flag for this process
	__declspec(property(get=getArchitecture)) enum class Architecture Architecture;
	enum class Architecture getArchitecture(void) const;

	// LocalDescriptorTableAddress
	//
	// Gets the address of the local descriptor table for this process
	__declspec(property(get=getLocalDescriptorTableAddress)) uintptr_t LocalDescriptorTableAddress;
	uintptr_t getLocalDescriptorTableAddress(void) const;

	// Namespace
	//
	// Gets the namespace associated with this process
	__declspec(property(get=getNamespace)) std::shared_ptr<class Namespace> Namespace;
	std::shared_ptr<class Namespace> getNamespace(void) const;

	// ProcessGroup
	//
	// Gets the process group to which this process belongs
	__declspec(property(get=getProcessGroup)) std::shared_ptr<class ProcessGroup> ProcessGroup;
	std::shared_ptr<class ProcessGroup> getProcessGroup(void) const;

	// ProcessId
	//
	// Gets the process identifier
	__declspec(property(get=getProcessId)) std::shared_ptr<Pid> ProcessId;
	std::shared_ptr<Pid> getProcessId(void) const;

	// RootPath
	//
	// Gets the process root path
	__declspec(property(get=getRootPath)) std::shared_ptr<FileSystem::Path> RootPath;
	std::shared_ptr<FileSystem::Path> getRootPath(void) const;

	// Session
	//
	// Gets the session to which this process belongs
	__declspec(property(get=getSession)) std::shared_ptr<class Session> Session;
	std::shared_ptr<class Session> getSession(void) const;

	// WorkingPath
	//
	// Gets the process working path
	__declspec(property(get=getWorkingPath)) std::shared_ptr<FileSystem::Path> WorkingPath;
	std::shared_ptr<FileSystem::Path> getWorkingPath(void) const;

private:

	Process(Process const&)=delete;
	Process& operator=(Process const&)=delete;

	// fspath_t
	//
	// FileSystem::Path shared pointer
	using fspath_t = std::shared_ptr<FileSystem::Path>;

	// namespace_t
	//
	// Namespace shared pointer
	using namespace_t = std::shared_ptr<class Namespace>;

	// nativeproc_t
	//
	// NativeProcess unique pointer
	using nativeproc_t = std::unique_ptr<NativeProcess>;

	// pgroup_t
	//
	// ProcessGroup shared pointer
	using pgroup_t = std::shared_ptr<class ProcessGroup>;

	// pid_t
	//
	// Pid shared pointer
	using pid_t = std::shared_ptr<Pid>;

	// session_t
	//
	// Session shared pointer
	using session_t = std::shared_ptr<class Session>;

	// statechange_t
	//
	// Indicates the type of state change that has occurred
	enum class statechange_t 
	{
		exited			= LINUX_CLD_EXITED,			// Process has exited normally
		killed			= LINUX_CLD_KILLED,			// Process was killed by a signal
		dumped			= LINUX_CLD_DUMPED,			// Process was killed and dumped
		trapped			= LINUX_CLD_TRAPPED,		// Traced process was trapped
		stopped			= LINUX_CLD_STOPPED,		// Process was stopped by a signal
		continued		= LINUX_CLD_CONTINUED,		// Process was continued by SIGCONT
	};

	// thread_map_t
	//
	// Collection of thread instances
	using thread_map_t = std::unordered_map<Thread const*, std::weak_ptr<Thread>>;

	// waiter_t
	//
	// Structure used to register a wait operation
	struct waiter_t 
	{
		std::condition_variable&			signal;			// Condition variable to signal
		std::mutex&							lock;			// Condition variable lock object
		int const							options;		// Wait operation mask and flags
		std::shared_ptr<Process> const&		process;		// Wait operation context object
		uapi::siginfo*						siginfo;		// Resultant signal information
		std::shared_ptr<Process>&			result;			// Resultant signaled object
	};

	// Instance Constructor
	//
	Process(nativeproc_t nativeproc, pid_t pid, session_t session, pgroup_t pgroup, namespace_t ns, uintptr_t ldtaddr, Bitmap&& ldtslots, fspath_t root, fspath_t working);
	friend class std::_Ref_count_obj<Process>;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// NotifyStateChange
	//
	// Signals that a change in the process state has occurred
	void NotifyStateChange(statechange_t newstate, int32_t status); 

	// Wait (static)
	//
	// Waits for a process instance to become signaled
	static std::shared_ptr<Process> Wait(std::vector<std::shared_ptr<Process>> const& processes, int options, uapi::siginfo* siginfo);

	// WaitOperationAcceptsStateChange (static)
	//
	// Determines if a wait options mask accepts a specific state change code
	static bool WaitOperationAcceptsStateChange(int mask, statechange_t newstate);

	//-------------------------------------------------------------------------
	// Member Variables

	nativeproc_t const					m_nativeproc;		// NativeProcess instance
	pid_t const							m_pid;				// Process identifier
	pgroup_t							m_pgroup;			// Parent ProcessGroup
	session_t							m_session;			// Parent Session
	namespace_t const					m_ns;				// Process namespace

	// Local Descriptor Table
	//
	uintptr_t const						m_ldtaddr;			// Address of local descriptor table
	Bitmap								m_ldtslots;			// LDT allocation map
	mutable sync::reader_writer_lock	m_ldtlock;			// Synchronization object

	// File System
	//
	fspath_t							m_root;				// Process root path
	fspath_t							m_working;			// Process working path

	// Threads
	//
	thread_map_t						m_threads;			// Collection of threads

	// Wait Operations
	//
	std::list<waiter_t>					m_waiters;			// Objects waiting for a state change
	uapi::siginfo						m_statepending;		// Unprocessed state change signal
	std::mutex							m_statelock;		// Synchronization object


	mutable sync::critical_section		m_cs;				// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESS_H_
