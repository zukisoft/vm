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
#include <linux/elf.h>
#include <linux/ldt.h>
#include <linux/mman.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/stat.h>
#include "elf_traits.h"
#include "Architecture.h"
#include "Bitmap.h"
#include "ElfArguments.h"
#include "ElfImage.h"
#include "Executable.h"
#include "Exception.h"
#include "HeapBuffer.h"
#include "Host.h"
#include "LinuxException.h"
#include "NtApi.h"
#include "ProcessHandles.h"
#include "Random.h"
#include "SignalActions.h"
#include "SystemInformation.h"
#include "TaskState.h"
#include "Thread.h"
#include "VirtualMachine.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Process
//
// Process represents a single hosted process instance.
//
// TODO: needs lot more words

class Process
{
public:

	// Destructor
	//
	~Process()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// AddHandle
	//
	// Adds a file system handle to the process
	int AddHandle(const FileSystem::HandlePtr& handle) { return m_handles->Add(handle); }
	int AddHandle(int fd, const FileSystem::HandlePtr& handle) { return m_handles->Add(fd, handle); }

	// Clone
	//
	// Clones the process into a new child process
	std::shared_ptr<Process> Clone(const std::shared_ptr<VirtualMachine>& vm, uint32_t flags, void* taskstate, size_t taskstatelen);

	// Create (static)
	//
	// Creates a new process instance via an external Windows host binary
	template <Architecture architecture>
	static std::shared_ptr<Process> Create(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t pid, const FileSystem::AliasPtr& rootdir, const FileSystem::AliasPtr& workingdir,
		const FileSystem::HandlePtr& handle, const uapi::char_t** argv, const uapi::char_t** envp, const tchar_t* hostpath, const tchar_t* hostargs);

	// Execute
	//
	// Executes a new process by replacing the current one
	void Execute(const std::shared_ptr<VirtualMachine>& vm, const char_t* filename, const char_t* const& argv, const char_t* const& envp);

	// FindNativeThread
	//
	// Locates a thread instance based on its native thread id
	std::shared_ptr<Thread> FindNativeThread(DWORD nativetid);

	// FindThread
	//
	// Locates a thread instance based on it's tid
	std::shared_ptr<Thread> FindThread(uapi::pid_t tid);

	// GetHandle
	//
	// Accesses a file system handle referenced by the process
	FileSystem::HandlePtr GetHandle(int index) { return m_handles->operator[](index); }

	// GetInitialTaskState
	//
	// Acquires the task state information structure used for the process, the contents of which
	// varies depending on if the process is 32 or 64-bit, is a new or cloned process, etc.
	void GetInitialTaskState(void* taskstate, size_t length);

	// MapMemory
	//
	// Creates a memory mapping for the process
	const void* MapMemory(size_t length, int prot, int flags) { return MapMemory(nullptr, length, prot, flags, -1, 0); }
	const void* MapMemory(const void* address, size_t length, int prot, int flags) { return MapMemory(address, length, prot, flags, -1, 0); }
	const void* MapMemory(const void* address, size_t length, int prot, int flags, int fd, uapi::loff_t offset);
	// TODO: will need overloads for shared memory when I get there

	// ProtectMemory
	//
	// Sets memory protection flags for a region
	void ProtectMemory(const void* address, size_t length, int prot) { return m_host->ProtectMemory(address, length, uapi::LinuxProtToWindowsPageFlags(prot)); }

	// ReadMemory
	//
	// Reads directly from the process memory space, will abort on a fault
	size_t ReadMemory(const void* address, void* buffer, size_t length) { return m_host->ReadMemory(address, buffer, length); }

	// RemoveHandle
	//
	// Removes a file system handle from the process
	void RemoveHandle(int index) { m_handles->Remove(index); }

	// SetLocalDescriptor
	//
	// Sets a local descriptor table entry
	// TODO: WILL THIS APPLY TO x64?
	void SetLocalDescriptor(uapi::user_desc32* u_info);

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
	// Sends a signal to the process
	void Signal(int signal);
	//void Signal(uapi::pid_t tgid, uapi::pid_t tid, int signal);

	// Start
	//
	// Launches the process
	void Start(void);

	// UnmapMemory
	//
	// Releases a memory mapping from the process
	void UnmapMemory(void* address, size_t length);

	// WaitChild
	//
	// TESTING
	uapi::pid_t WaitChild_TEST(uapi::pid_t pid, int* status);
	uapi::pid_t RegisterThread_TEST(DWORD nativeid);

	// WriteMemory
	//
	// Writes directly into the process memory space, will abort on a fault
	size_t WriteMemory(const void* address, const void* buffer, size_t length) { return m_host->WriteMemory(address, buffer, length); }

	//-------------------------------------------------------------------------
	// Properties

	// Class
	//
	// Gets the class (x86/x86_64) of the process
	__declspec(property(get=getArchitecture)) ::Architecture Architecture;
	::Architecture getArchitecture(void) const { return m_architecture; }

	// FileCreationModeMask
	//
	// Gets/sets the process UMASK for default file system permissions
	__declspec(property(get=getFileCreationModeMask, put=putFileCreationModeMask)) uapi::mode_t FileCreationModeMask;
	uapi::mode_t getFileCreationModeMask(void) const { return m_umask; }
	void putFileCreationModeMask(uapi::mode_t value) { m_umask = (value & LINUX_S_IRWXUGO); }

	// LocalDescriptorTable
	//
	// Gets the address of the process' local descriptor table
	__declspec(property(get=getLocalDescriptorTable)) const void* LocalDescriptorTable;
	const void* getLocalDescriptorTable(void) const;

	// NativeHandle
	//
	// Gets the native process handle
	__declspec(property(get=getNativeHandle)) HANDLE NativeHandle;
	HANDLE getNativeHandle(void) const;

	// NativeProcessId
	//
	// Gets the native process identifier
	__declspec(property(get=getNativeProcessId)) DWORD NativeProcessId;
	DWORD getNativeProcessId(void) const;

	// NativeThreadProc
	//
	// Gets/sets the native process thread entry point
	__declspec(property(get=getNativeThreadProc, put=putNativeThreadProc)) void* NativeThreadProc;
	void* getNativeThreadProc(void) const;
	void putNativeThreadProc(void* value);

	// ParentProcessId
	//
	// Gets the process identifier of the parent process
	__declspec(property(get=getParentProcessId)) uapi::pid_t ParentProcessId;
	uapi::pid_t getParentProcessId(void) const;
	
	// ProcessId
	//
	// Gets the virtual machine process identifier
	__declspec(property(get=getProcessId)) uapi::pid_t ProcessId;
	uapi::pid_t getProcessId(void) const { return m_pid; }

	// TidAddress
	//
	// TODO: This is used for set_tid_address; needs to have a futex associated with 
	// it as well, stubbing this out so that sys_set_tid_address can be implemented
	__declspec(property(get=getTidAddress, put=putTidAddress)) void* TidAddress;
	void* getTidAddress(void) const { return m_tidaddress; }
	void putTidAddress(void* value) { m_tidaddress = value; }

	// RootDirectory
	//
	// Gets the alias of the process' root directory
	__declspec(property(get=getRootDirectory, put=putRootDirectory)) FileSystem::AliasPtr RootDirectory;
	FileSystem::AliasPtr getRootDirectory(void) { return m_rootdir; }
	void putRootDirectory(const FileSystem::AliasPtr& value) { m_rootdir = value; }

	// WorkingDirectory
	//
	// Gets the alias of the process' current working directory
	__declspec(property(get=getWorkingDirectory, put=putWorkingDirectory)) FileSystem::AliasPtr WorkingDirectory;
	FileSystem::AliasPtr getWorkingDirectory(void) { return m_workingdir; }
	void putWorkingDirectory(const FileSystem::AliasPtr& value) { m_workingdir = value; }

	// Zombie
	//
	// Gets flag indicating if this process is a 'zombie' or not
	__declspec(property(get=getZombie)) bool Zombie;
	bool getZombie(void) const;

private:

	Process(const Process&)=delete;
	Process& operator=(const Process&)=delete;

	// Instance Constructor
	//
	Process(::Architecture architecture, std::unique_ptr<Host>&& host, std::shared_ptr<Thread>&& thread, uapi::pid_t pid, const FileSystem::AliasPtr& rootdir, const FileSystem::AliasPtr& workingdir, 
		std::unique_ptr<TaskState>&& taskstate, const void* ldt, Bitmap&& ldtmap, const std::shared_ptr<ProcessHandles>& handles, const std::shared_ptr<SignalActions>& sigactions, const void* programbreak);
	friend class std::_Ref_count_obj<Process>;

	// ldt_lock_t
	//
	// Local descriptor table collection synchronization object
	using ldt_lock_t = Concurrency::reader_writer_lock;

	using thread_map_t = std::unordered_map<uapi::pid_t, std::shared_ptr<Thread>>;
	using thread_map_lock_t = Concurrency::reader_writer_lock;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// CheckHostArchitecture (static)
	//
	// Verifies that the created host process type matches what is expected
	template <::Architecture architecture>
	static void CheckHostArchitecture(HANDLE process);

	//-------------------------------------------------------------------------
	// Member Variables

	std::unique_ptr<Host>		m_host;			// Hosted windows process
	std::unique_ptr<TaskState>	m_taskstate;	// Initial task state information

	// TODO: move implementation in from Host?
	//HANDLE						m_nativeprocess;		// Native process handle
	//DWORD						m_nativepid;			// Native process identifier
	void*						m_nativethreadproc;		// Native ThreadProc

	const ::Architecture			m_architecture;
	////

	const uapi::pid_t					m_pid;				// Process identifier
	std::weak_ptr<Process>				m_parent;			// Parent process
	DWORD m_threadid_TEST;

	// TESTING CHILDREN
	// NEEDS SYNCHRONIZATION OBJECT
	std::unordered_map<int, std::weak_ptr<Process>> m_children;

	
	thread_map_t				m_threads;
	thread_map_lock_t			m_threadslock;

	void*					m_tidaddress = nullptr;

	// VIRTUAL MEMORY
	//
	const void*						m_programbreak;		// Current program break

	// LOCAL DESCRIPTOR TABLE
	//
	const void*						m_ldt;				// Local descriptor table
	Bitmap							m_ldtslots;			// LDT allocation bitmap
	ldt_lock_t						m_ldtlock;			// LDT synchronization object

	// SIGNALS
	//
	std::shared_ptr<SignalActions>	m_sigactions;		// Signal actions

	// FILE SYSTEM
	//
	std::shared_ptr<ProcessHandles>		m_handles;			// File system handles
	std::shared_ptr<FileSystem::Alias>	m_rootdir;			// Process root directory
	std::shared_ptr<FileSystem::Alias>	m_workingdir;		// Process working directory
	std::atomic<uapi::mode_t>			m_umask = 0022;		// Default UMASK value
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESS_H_
