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

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <concrt.h>
#include <linux/ldt.h>
#include <linux/resource.h>
#include <linux/sched.h>
#include <linux/siginfo.h>
#include <linux/stat.h>
#include <linux/wait.h>
#include "Architecture.h"
#include "Bitmap.h"
#include "Executable.h"
#include "FileSystem.h"
#include "LinuxException.h"
#include "NativeHandle.h"
#include "NativeProcess.h"
#include "NtApi.h"
#include "ProcessHandles.h"
#include "ProcessMemory.h"
#include "ScalarCondition.h"
#include "SignalActions.h"
#include "TaskState.h"
#include "Thread.h"
#include "VirtualMachine.h"
#include "Waitable.h"

#pragma warning(push, 4)

// Forward Declarations
//
class ProcessGroup;
class Session;

//-----------------------------------------------------------------------------
// Process
//
// Process represents a virtual machine process/thread group instance.
//
// TODO: This class absolutely needs documentation, it's become very large
// and complicated in places.

class Process : public Waitable, public std::enable_shared_from_this<Process>
{
public:

	// Destructor
	//
	virtual ~Process();

	//-------------------------------------------------------------------------
	// Member Functions

	// AddHandle
	//
	// Adds a file system handle to the process
	int AddHandle(const std::shared_ptr<FileSystem::Handle>& handle);
	int AddHandle(int fd, const std::shared_ptr<FileSystem::Handle>& handle);

	// AttachThread
	//
	// Attaches a native thread to this process instance
	std::shared_ptr<::Thread> AttachThread(DWORD nativetid);

	// Clone
	//
	// Clones this process into a new child process
	std::shared_ptr<Process> Clone(int flags, std::unique_ptr<TaskState>&& task);

	// CreateThread
	//
	// Creates a new Thread in this process
	std::shared_ptr<Thread> CreateThread(int flags, const std::unique_ptr<TaskState>& task) const;

	// DetachThread
	//
	// Detaches a thread from this process instance
	void DetachThread(const std::shared_ptr<::Thread>& thread, int exitcode);

	// Execute
	//
	// Replaces the process with a new executable image
	void Execute(const char_t* filename, const char_t* const* argv, const char_t* const* envp);

	// ExitThread
	//
	// Indicates that a thread has exited normally
	void ExitThread(uapi::pid_t tid, int exitcode);

	// FromExecutable (static)
	//
	// Creates a new Process from an Executable instance
	static std::shared_ptr<Process> FromExecutable(const std::shared_ptr<VirtualMachine>& vm, const std::shared_ptr<::ProcessGroup>& pgroup, 
		uapi::pid_t pid, const std::unique_ptr<Executable>& executable);

	// GetResourceUsage
	//
	// Gets resource usage information for the process
	void GetResourceUsage(int who, uapi::rusage* rusage);

	// MapMemory
	//
	// Allocates/maps process virtual address space
	const void* MapMemory(size_t length, int prot, int flags);
	const void* MapMemory(const void* address, size_t length, int prot, int flags);
	const void* MapMemory(const void* address, size_t length, int prot, int flags, int fd, uapi::loff_t offset);

	// ProtectMemory
	//
	// Sets the memory protection flags for an address space region
	void ProtectMemory(const void* address, size_t length, int prot) const;

	// ReadMemory
	//
	// Reads data from the process virtual address space
	size_t ReadMemory(const void* address, void* buffer, size_t length) const;

	// RemoveHandle
	//
	// Removes a file system handle from the process
	void RemoveHandle(int fd);

	// Resume
	//
	// Resumes the process from a suspended state
	void Resume(void);

	// RundownThread
	//
	// Removes a thread from the process that didn't terminate normally
	void RundownThread(const std::shared_ptr<::Thread>& thread);

	// SetProgramBreak
	//
	// Sets the program break address to increase or decrease data segment length
	const void* SetProgramBreak(const void* address);

	// SetSignalAction
	//
	// Assigns an action to be taken for a process signal
	void SetSignalAction(int signal, const uapi::sigaction* action, uapi::sigaction* oldaction);

	// Signal
	//
	// Signals the process
	bool Signal(int signal);

	// Start
	//
	// Starts the process
	void Start(void);

	// Suspend
	//
	// Suspends the process
	void Suspend(void);

	// Terminate
	//
	// Terminates the process
	void Terminate(int exitcode);

	// UnmapMemory
	//
	// Releases process virtual address space
	void UnmapMemory(const void* address, size_t length);

	// WaitChild
	//
	// Waits for one or more child processes to terminate
	uapi::pid_t WaitChild(uapi::idtype_t type, uapi::pid_t id, int* status, int options, uapi::siginfo* siginfo, uapi::rusage* rusage);

	// WriteMemory
	//
	// Writes data into the process virtual address space
	size_t WriteMemory(const void* address, const void* buffer, size_t length) const;

	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the process architecture code
	__declspec(property(get=getArchitecture)) ::Architecture Architecture;
	::Architecture getArchitecture(void) const;

	// FileCreationModeMask
	//
	// Gets/sets the process umask for default file system permissions
	__declspec(property(get=getFileCreationModeMask, put=putFileCreationModeMask)) uapi::mode_t FileCreationModeMask;
	uapi::mode_t getFileCreationModeMask(void) const;
	void putFileCreationModeMask(uapi::mode_t value);

	// Handle
	//
	// Gets a file system handle by file descriptor index
	__declspec(property(get=getHandle)) std::shared_ptr<FileSystem::Handle> Handle[];
	std::shared_ptr<FileSystem::Handle> getHandle(int fd) const;

	// LocalDescriptorTable
	//
	// Gets the address of the process local descriptor table
	__declspec(property(get=getLocalDescriptorTable)) const void* LocalDescriptorTable;
	const void* getLocalDescriptorTable(void) const;

	// NativeProcessId
	//
	// Gets the native operating system process identifier
	__declspec(property(get=getNativeProcessId)) DWORD NativeProcessId;
	DWORD getNativeProcessId(void) const;

	// NativeThreadProc
	//
	// Gets/sets the address of the native process thread entry point
	__declspec(property(get=getNativeThreadProc, put=putNativeThreadProc)) const void* NativeThreadProc;
	const void* getNativeThreadProc(void) const;
	void putNativeThreadProc(const void* value);

	// Parent
	//
	// Gets the parent process for this process
	__declspec(property(get=getParent)) std::shared_ptr<Process> Parent;
	std::shared_ptr<Process> getParent(void) const;

	// ProcessGroup
	//
	// Gets a reference to the containing process group instance
	__declspec(property(get=getProcessGroup)) std::shared_ptr<::ProcessGroup> ProcessGroup;
	std::shared_ptr<::ProcessGroup> getProcessGroup(void) const;

	// ProcessId
	//
	// Gets the virtual process identifier
	__declspec(property(get=getProcessId)) uapi::pid_t ProcessId;
	uapi::pid_t getProcessId(void) const;

	// ProgramBreak
	//
	// Gets the current program break address
	__declspec(property(get=getProgramBreak)) const void* ProgramBreak;
	const void* getProgramBreak(void) const;

	// RootDirectory
	//
	// Gets/sets the process root directory alias
	__declspec(property(get=getRootDirectory, put=putRootDirectory)) std::shared_ptr<FileSystem::Alias> RootDirectory;
	std::shared_ptr<FileSystem::Alias> getRootDirectory(void) const;
	void putRootDirectory(const std::shared_ptr<FileSystem::Alias>& value);

	// SetLocalDescriptor
	//
	// Sets a local descriptor table entry
	// todo: how to handle x64
	void SetLocalDescriptor(uapi::user_desc32* u_info);
	void SetLocalDescriptor(uapi::user_desc64* u_info);

	// SignalAction
	//
	// Gets/sets the action associated with a signal
	__declspec(property(get=getSignalAction, put=putSignalAction)) uapi::sigaction SignalAction[];
	uapi::sigaction getSignalAction(int signal) const;
	void putSignalAction(int signal, uapi::sigaction action);

	// TerminationSignal
	//
	// Gets/sets the signal to send to the parent on termination
	__declspec(property(get=getTerminationSignal, put=putTerminationSignal)) int TerminationSignal;
	int getTerminationSignal(void) const;
	void putTerminationSignal(int value);

	// Thread
	//
	// Gets a Thread instance by virtual thread identifier
	__declspec(property(get=getThread)) std::shared_ptr<Thread> Thread[];
	std::shared_ptr<::Thread> getThread(uapi::pid_t tid);
	
	// WorkingDirectory
	//
	// Gets/sets the process working directory alias
	__declspec(property(get=getWorkingDirectory, put=putWorkingDirectory)) std::shared_ptr<FileSystem::Alias> WorkingDirectory;
	std::shared_ptr<FileSystem::Alias> getWorkingDirectory(void) const;
	void putWorkingDirectory(const std::shared_ptr<FileSystem::Alias>& value);

private:

	Process(const Process&)=delete;
	Process& operator=(const Process&)=delete;

	// ldt_lock_t
	//
	// Synchronization object for the local descriptor table
	using ldt_lock_t = Concurrency::reader_writer_lock;

	// process_map_t
	//
	// Collection of contained Process instances, keyed on the process identifier
	using process_map_t = std::map<uapi::pid_t, std::shared_ptr<Process>>;

	// process_map_lock_t
	//
	// Synchronization object for process_map_t
	using process_map_lock_t = Concurrency::reader_writer_lock;

	// pending_thread_map_t
	//
	// Collection of pending Thread instances, keyed on native thread identifier
	using pending_thread_map_t = std::map<DWORD, std::unique_ptr<TaskState>>;

	// thread_map_t
	//
	// Collection of contained Thread instances, keyed on the thread identifier
	using thread_map_t = std::map<uapi::pid_t, std::shared_ptr<::Thread>>;

	// thread_map_lock_t
	//
	// Synchronization object for thread_map_t
	using thread_map_lock_t = Concurrency::reader_writer_lock;

	// Instance Constructors
	//
	Process(const std::shared_ptr<VirtualMachine>& vm, ::Architecture architecture, uapi::pid_t pid, const std::shared_ptr<Process>& parent, 
		std::unique_ptr<NativeProcess>&& host, std::unique_ptr<TaskState>&& task, std::unique_ptr<ProcessMemory>&& memory, const void* ldt, 
		Bitmap&& ldtslots, const void* programbreak, int termsignal, const std::shared_ptr<FileSystem::Alias>& rootdir, 
		const std::shared_ptr<FileSystem::Alias>& workingdir);

	Process(const std::shared_ptr<VirtualMachine>& vm, ::Architecture architecture, uapi::pid_t pid, const std::shared_ptr<Process>& parent, 
		std::unique_ptr<NativeProcess>&& host, std::unique_ptr<TaskState>&& task, std::unique_ptr<ProcessMemory>&& memory, const void* ldt, 
		Bitmap&& ldtslots, const void* programbreak, const std::shared_ptr<ProcessHandles>& handles, const std::shared_ptr<SignalActions>& sigactions, 
		int termsignal, const std::shared_ptr<FileSystem::Alias>& rootdir, 	const std::shared_ptr<FileSystem::Alias>& workingdir);

	friend class std::_Ref_count_obj<Process>;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// ClearThreads
	//
	// Removes all threads from the process
	void ClearThreads(void);

	// Clone<Architecture>
	//
	// Clones this process into a new child process
	template<::Architecture architecture>
	std::shared_ptr<Process> Clone(uapi::pid_t pid, int flags, std::unique_ptr<TaskState>&& task);

	// CollectWaitables
	//
	// Generates a collection of waitable objects for use with WaitChild()
	std::vector<std::shared_ptr<Waitable>> CollectWaitables(uapi::idtype_t type, uapi::pid_t id, int options);

	// CreateStack
	//
	// Allocates a new stack in the process
	static const void* CreateStack(const std::shared_ptr<VirtualMachine>& vm, const std::unique_ptr<ProcessMemory>& memory);

	// Execute<Architecture>
	//
	// Replaces the process with a new executable image
	template<::Architecture>
	void Execute(const std::unique_ptr<Executable>& executable);

	// FromExecutable<Architecture>
	//
	// Creates a new process instance from an executable image
	template<::Architecture architecture>
	static std::shared_ptr<Process> FromExecutable(const std::shared_ptr<VirtualMachine>& vm, const std::shared_ptr<::ProcessGroup>& pgroup, uapi::pid_t pid, 
		const std::shared_ptr<Process>& parent, const std::unique_ptr<Executable>& executable);

	// NotifyParent
	//
	// Notifies the parent process that a state change has occurred
	void NotifyParent(Waitable::State state, int32_t status);

	// ResumeInternal
	//
	// Resumes the process from a suspended state
	void ResumeInternal(void) const;

	// SuspendInternal
	//
	// Suspends the process
	void SuspendInternal(void) const;

	//-------------------------------------------------------------------------
	// Member Variables

	const std::shared_ptr<VirtualMachine>	m_vm;				// Virtual machine instance
	std::weak_ptr<::ProcessGroup>			m_pgroup;			// Parent ProcessGroup instance
	const ::Architecture					m_architecture;		// Process architecture
	std::unique_ptr<NativeProcess>			m_host;				// Native process instance
	const uapi::pid_t						m_pid;				// Process identifier

	// Parent and Children
	//
	std::weak_ptr<Process>				m_parent;			// Weak reference to parent
	process_map_t						m_children;			// Collection of child processes
	process_map_lock_t					m_childlock;		// Synchronization object
	ScalarCondition<bool>				m_nochildren;		// "No Children" condition variable

	// Memory
	//
	std::unique_ptr<ProcessMemory>		m_memory;			// Virtual address space
	const void*							m_programbreak;		// Current program break

	// Local Descriptor Table
	//
	const void*							m_ldt;				// Local descriptor table
	Bitmap								m_ldtslots;			// LDT allocation bitmap
	ldt_lock_t							m_ldtlock;			// Synchronization object

	// File System
	//
	std::shared_ptr<FileSystem::Alias>	m_rootdir;			// Current root directory
	std::shared_ptr<FileSystem::Alias>	m_workingdir;		// Current working directory
	std::shared_ptr<ProcessHandles>		m_handles;			// File system handles
	std::atomic<uapi::mode_t>			m_umask = 0022;		// Process file creation mask

	// Signals
	//
	std::shared_ptr<SignalActions>		m_sigactions;		// Signal actions
	std::atomic<int>					m_termsignal;		// Termination signal

	// Pending Threads
	//
	pending_thread_map_t				m_pendingthreads;	// Pending threads
	std::condition_variable				m_attached;			// Condition signaling an attach
	std::mutex							m_attachlock;		// Synchronization object

	// Threads
	//
	const void*							m_threadproc;		// Native thread entry point
	thread_map_t						m_threads;			// Collection of threads
	thread_map_lock_t					m_threadslock;		// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESS_H_
