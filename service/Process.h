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
#include <memory>
#include <unordered_map>
#include <concrt.h>
#include <linux/ldt.h>
#include <linux/stat.h>
#include "Architecture.h"
#include "Bitmap.h"
#include "Executable.h"
#include "FileSystem.h"
#include "LinuxException.h"
#include "NativeHandle.h"
#include "ProcessHandles.h"
#include "NativeProcess.h"
#include "ProcessMemory.h"
#include "SignalActions.h"
#include "TaskState.h"
#include "Thread.h"
#include "VirtualMachine.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Process
//
// Process represents a virtual machine process/thread group instance

class Process
{
public:

	// Destructor
	//
	~Process();

	//-------------------------------------------------------------------------
	// Member Functions

	// AddHandle
	//
	// Adds a file system handle to the process
	int AddHandle(const FileSystem::HandlePtr& handle);
	int AddHandle(int fd, const FileSystem::HandlePtr& handle);

	// ProtectMemory
	//
	// Sets the memory protection flags for an address space region
	void ProtectMemory(const void* address, size_t length, int prot) const;

	// Spawn (static)
	//
	// Spawns a new process instance
	static std::shared_ptr<Process> Spawn(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t pid, const char_t* filename, const char_t* const* argv,
		const char_t* const* envp, const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir);

	// ReadMemory
	//
	// Reads data from the process virtual address space
	size_t ReadMemory(const void* address, void* buffer, size_t length) const;

	// RemoveHandle
	//
	// Removes a file system handle from the process
	void RemoveHandle(int fd);

	// SetProgramBreak
	//
	// Sets the program break address to increase or decrease data segment length
	const void* SetProgramBreak(const void* address);

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

	// Zombie
	//
	// Gets a flag indicating if this process is a 'zombie' or not
	__declspec(property(get=getZombie)) bool Zombie;
	bool getZombie(void) const;

private:

	Process(const Process&)=delete;
	Process& operator=(const Process&)=delete;

	// ldt_lock_t
	//
	// Synchronization object for the local descriptor table
	using ldt_lock_t = Concurrency::reader_writer_lock;

	// thread_map_t
	//
	// Collection of contained Thread instances, keyed on the thread identifier
	using thread_map_t = std::unordered_map<uapi::pid_t, std::shared_ptr<::Thread>>;

	// thread_map_lock_t
	//
	// Synchronization object for thread_map_t
	using thread_map_lock_t = Concurrency::reader_writer_lock;

	// Instance Constructors
	//
	Process(const std::shared_ptr<VirtualMachine>& vm, ::Architecture architecture, uapi::pid_t pid, const std::shared_ptr<NativeHandle>& process,
		DWORD processid, std::unique_ptr<ProcessMemory>&& memory, const void* ldt, const void* programbreak, const std::shared_ptr<::Thread>& mainthread,
		const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir);

	Process(const std::shared_ptr<VirtualMachine>& vm, ::Architecture architecture, uapi::pid_t pid, const std::shared_ptr<NativeHandle>& process,
		DWORD processid, std::unique_ptr<ProcessMemory>&& memory, const void* ldt, const void* programbreak, const std::shared_ptr<ProcessHandles>& handles, 
		const std::shared_ptr<SignalActions>& sigactions, const std::shared_ptr<::Thread>& mainthread, const std::shared_ptr<FileSystem::Alias>& rootdir, 
		const std::shared_ptr<FileSystem::Alias>& workingdir);

	friend class std::_Ref_count_obj<Process>;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// CreateThreadStack
	//
	// Creates the stack memory for a thread
	static const void* CreateThreadStack(const std::shared_ptr<VirtualMachine>& vm, const std::unique_ptr<ProcessMemory>& memory);

	// FromExecutable<Architecture>
	//
	// Creates a new process instance from an Executable
	template<::Architecture architecture>
	static std::shared_ptr<Process> FromExecutable(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t pid, const std::unique_ptr<Executable>& executable);

	//-------------------------------------------------------------------------
	// Member Variables

	std::shared_ptr<VirtualMachine>		m_vm;				// Virtual machine instance
	const ::Architecture				m_architecture;		// Process architecture
	const uapi::pid_t					m_pid;				// Process identifier
	std::shared_ptr<NativeHandle>		m_process;			// Native process handle
	const DWORD							m_processid;		// Native process identifier

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

	// Threads
	//
	thread_map_t						m_threads;			// Collection of threads
	thread_map_lock_t					m_threadslock;		// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESS_H_
