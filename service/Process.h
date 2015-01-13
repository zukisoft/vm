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

#include <concrt.h>
#include <memory>
#include <unordered_map>
#include <linux/elf.h>
#include <linux/mman.h>
#include <linux/sched.h>
#include <linux/stat.h>
#include "elf_traits.h"
#include "ElfArguments.h"
#include "ElfImage.h"
#include "Exception.h"
#include "HeapBuffer.h"
#include "Host.h"
#include "IndexPool.h"
#include "LinuxException.h"
#include "NtApi.h"
#include "ProcessClass.h"
#include "Random.h"
#include "SystemInformation.h"
#include "TaskState.h"
#include "VirtualMachine.h"
// todo: the above includes need to be scrubbed

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

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
	int AddHandle(const FileSystem::HandlePtr& handle);
	int AddHandle(int fd, const FileSystem::HandlePtr& handle);

	// Clone
	//
	// Clones the process into a new child process
	std::shared_ptr<Process> Clone(const std::shared_ptr<VirtualMachine>& vm, uint32_t flags, void* taskstate, size_t taskstatelen);

	// Create (static)
	//
	// Creates a new process instance via an external Windows host binary
	template <ProcessClass _class>
	static std::shared_ptr<Process> Create(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t pid, const FileSystem::AliasPtr& rootdir, const FileSystem::AliasPtr& workingdir,
		const FileSystem::HandlePtr& handle, const uapi::char_t** argv, const uapi::char_t** envp, const tchar_t* hostpath, const tchar_t* hostargs);

	// GetHandle
	//
	// Accesses a file system handle referenced by the process
	FileSystem::HandlePtr GetHandle(int index);

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
	void RemoveHandle(int index);

	// Resume
	//
	// Resumes the process from a suspended state
	void Resume(void);

	// SetProgramBreak
	//
	// Sets the program break address to increase or decrease data segment length
	const void* SetProgramBreak(const void* address);

	// Spawn (static)
	//
	// Creates a new process instance
	static std::shared_ptr<Process> Spawn(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t pid, const uapi::char_t* filename, const uapi::char_t** argv, const uapi::char_t** envp);

	// Suspend
	//
	// Suspends the process
	void Suspend(void);

	// UnmapMemory
	//
	// Releases a memory mapping from the process
	void UnmapMemory(void* address, size_t length);

	// WriteMemory
	//
	// Writes directly into the process memory space, will abort on a fault
	size_t WriteMemory(const void* address, const void* buffer, size_t length) { return m_host->WriteMemory(address, buffer, length); }

	//-------------------------------------------------------------------------
	// Properties

	// Class
	//
	// Gets the class (x86/x86_64) of the process
	__declspec(property(get=getClass)) ProcessClass Class;
	ProcessClass getClass(void) const { return m_class; }

	// FileCreationModeMask
	//
	// Gets/sets the process UMASK for default file system permissions
	__declspec(property(get=getFileCreationModeMask, put=putFileCreationModeMask)) uapi::mode_t FileCreationModeMask;
	uapi::mode_t getFileCreationModeMask(void) const { return m_umask; }
	void putFileCreationModeMask(uapi::mode_t value) { m_umask = (value & LINUX_S_IRWXUGO); }

	// HostProcessId
	//
	// Gets the host process identifier
	// TODO: I don't like having this, get rid of it
	__declspec(property(get=getHostProcessId)) DWORD HostProcessId;
	DWORD getHostProcessId(void) const { return m_host->ProcessId; }

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

private:

	Process(const Process&)=delete;
	Process& operator=(const Process&)=delete;

	// Instance Constructor
	//
	Process(ProcessClass _class, std::unique_ptr<Host>&& host, uapi::pid_t pid, const FileSystem::AliasPtr& rootdir, const FileSystem::AliasPtr& workingdir, 
		std::unique_ptr<TaskState>&& taskstate, const void* programbreak);
	friend class std::_Ref_count_obj<Process>;

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// MIN_HANDLE_INDEX
	//
	// Minimum allowable file system handle index (file descriptor)
	static const int MIN_HANDLE_INDEX = 3;

	// handle_map_t
	//
	// Collection of file system handles, keyed on the index (file descriptor)
	using handle_map_t = std::unordered_map<int, FileSystem::HandlePtr>;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// CheckHostProcessClass (static)
	//
	// Verifies that the created host process type matches what is expected
	template <ProcessClass _class>
	static void CheckHostProcessClass(HANDLE process);

	// Terminate
	//
	// Terminates the process
	void Terminate(int exitcode) { m_host->Terminate(exitcode); }

	//-------------------------------------------------------------------------
	// Member Variables

	std::unique_ptr<Host>		m_host;			// Hosted windows process
	std::unique_ptr<TaskState>	m_taskstate;	// Initial task state information

	const ProcessClass			m_class;
	////

	const uapi::pid_t					m_pid;				// Process identifier
	std::weak_ptr<Process>				m_parent;			// Parent process

	void*					m_tidaddress = nullptr;

	// VIRTUAL MEMORY MANAGEMENT
	//
	const void*						m_programbreak;		// Current program break


	// LDT section?

	// HANDLE MANAGEMENT
	//
	Concurrency::reader_writer_lock	m_handlelock;	// Handle collection lock
	handle_map_t					m_handles;		// Process file system handles
	IndexPool<int>					m_indexpool { MIN_HANDLE_INDEX };

	// FILE SYSTEM MANAGEMENT
	//
	FileSystem::AliasPtr		m_rootdir;			// Process root directory
	FileSystem::AliasPtr		m_workingdir;		// Process working directory
	std::atomic<uapi::mode_t>	m_umask = 0022;		// Default UMASK value
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESS_H_
