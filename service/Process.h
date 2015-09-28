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

#include <memory>
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

	// Process::State
	//
	// Defines the state of the process
	enum class State
	{
		Pending		= 0,			// Process is pending
		Running,					// Process is running
		Suspended,					// Process is suspended
		Stopped,					// Process is stopped
	};

	//-------------------------------------------------------------------------
	// Friend Functions

	// AddProcessThread
	//
	// Adds a thread to the collection
	friend std::shared_ptr<Process> AddProcessThread(std::shared_ptr<Process> process, std::shared_ptr<Thread> thread);

	// RemoveProcessThread
	//
	// Removes a thread from the collection
	friend void RemoveProcessThread(std::shared_ptr<Process> process, Thread const* thread);

	//-------------------------------------------------------------------------
	// Member Functions

	// Attach (static)
	//
	// Attaches a native process to a pending virtual machine process
	static std::shared_ptr<Process> Attach(DWORD nativepid);

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

	// WaitForState
	//
	// Waits for the process to reach a specific state
	bool WaitForState(enum class State, uint32_t timeoutms) const;

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

	// State
	//
	// Gets the current process state
	__declspec(property(get=getState)) enum class State State;
	enum class State getState(void) const;

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

	// pendingmap_t
	//
	// Collection of pending process objects
	using pendingmap_t = std::unordered_map<DWORD, std::shared_ptr<Process>>;
	
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

	// thread_map_t
	//
	// Collection of thread instances
	using thread_map_t = std::unordered_map<Thread const*, std::weak_ptr<Thread>>;

	// Instance Constructor
	//
	Process(nativeproc_t nativeproc, pid_t pid, session_t session, pgroup_t pgroup, namespace_t ns, uintptr_t ldtaddr, Bitmap&& ldtslots, fspath_t root, fspath_t working);
	friend class std::_Ref_count_obj<Process>;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// SetState
	//
	// Sets the process state value and signals the condition variable
	void SetState(enum class State state);

	//-------------------------------------------------------------------------
	// Member Variables

	nativeproc_t const					m_nativeproc;		// NativeProcess instance
	pid_t const							m_pid;				// Process identifier
	pgroup_t							m_pgroup;			// Parent ProcessGroup
	session_t							m_session;			// Parent Session
	namespace_t const					m_ns;				// Process namespace

	// Process State
	//
	enum class State					m_state;			// Current process state
	mutable std::condition_variable		m_statecond;		// State change condition
	mutable std::mutex					m_statelock;		// Synchronization object

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

	mutable sync::critical_section		m_cs;				// Synchronization object

	// Pending Processes (static)
	//
	static pendingmap_t					s_pendingmap;		// Collection of pending processes
	static std::condition_variable		s_pendingcond;		// Process attach condition
	static std::mutex					s_pendinglock;		// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESS_H_
