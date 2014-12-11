//-----------------------------------------------------------------------------
// Copyright (c) 2014 Michael G. Brehm
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

#include <array>
#include <concrt.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <linux/elf.h>
#include <linux/mman.h>
#include <linux/sched.h>
#include <linux/stat.h>
#include "ElfArguments.h"
#include "ElfClass.h"
#include "ElfImage.h"
#include "Exception.h"
#include "HeapBuffer.h"
#include "Host.h"
#include "IndexPool.h"
#include "LinuxException.h"
#include "MemorySection.h"
#include "Random.h"
#include "SystemInformation.h"
#include "VirtualMachine.h"

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
	std::shared_ptr<Process> Clone(const std::shared_ptr<VirtualMachine>& vm, const tchar_t* hostpath, const tchar_t* hostargs, uint32_t flags);

	// Create (static)
	//
	// Creates a new process instance via an external Windows host binary
	template <ElfClass _class>
	static std::shared_ptr<Process> Create(const std::shared_ptr<VirtualMachine>& vm, const FileSystem::AliasPtr& rootdir, const FileSystem::AliasPtr& workingdir,
		const FileSystem::HandlePtr& handle, const uapi::char_t** argv, const uapi::char_t** envp, const tchar_t* hostpath, const tchar_t* hostargs);

	// GetHandle
	//
	// Accesses a file system handle referenced by the process
	FileSystem::HandlePtr GetHandle(int index);

	// MapMemory
	//
	// Creates a memory mapping for the process
	void* MapMemory(size_t length, int prot, int flags) { return MapMemory(nullptr, length, prot, flags, -1, 0); }
	void* MapMemory(void* address, size_t length, int prot, int flags) { return MapMemory(address, length, prot, flags, -1, 0); }
	void* MapMemory(void* address, size_t length, int prot, int flags, int fd, uapi::loff_t offset);
	// TODO: will need overloads for shared memory when I get there

	// ProtectMemory
	//
	// Sets memory protection flags for a region
	void ProtectMemory(void* address, size_t length, int prot);

	// ReadMemory
	//
	// Reads directory from the process memory space, will abort on a fault
	size_t ReadMemory(const void* address, void* buffer, size_t length);

	// RemoveHandle
	//
	// Removes a file system handle from the process
	void RemoveHandle(int index);

	// Resume
	//
	// Resumes the process from a suspended state
	void Resume(void) { _ASSERTE(m_host); m_host->Resume(); }

	// SetProgramBreak
	//
	// Sets the program break address to increase or decrease data segment length
	void* SetProgramBreak(void* address);

	// Suspend
	//
	// Suspends the process
	void Suspend(void) { _ASSERTE(m_host); m_host->Suspend(); }

	// Terminate
	//
	// Terminates the process
	void Terminate(int exitcode) { _ASSERTE(m_host); m_host->Terminate(-exitcode); }

	// UnmapMemory
	//
	// Releases a memory mapping from the process
	void UnmapMemory(void* address, size_t length);

	//-------------------------------------------------------------------------
	// Properties

	// EntryPoint
	//
	// Gets the entry point address of the hosted process
	__declspec(property(get=getEntryPoint)) const void* EntryPoint;
	const void* getEntryPoint(void) const { return m_startinfo.EntryPoint; }

	// FileCreationModeMask
	//
	// Gets/sets the process UMASK for default file system permissions
	__declspec(property(get=getFileCreationModeMask, put=putFileCreationModeMask)) uapi::mode_t FileCreationModeMask;
	uapi::mode_t getFileCreationModeMask(void) const { return m_umask; }
	void putFileCreationModeMask(uapi::mode_t value) { m_umask = (value & LINUX_S_IRWXUGO); }

	// HostProcessId
	//
	// Gets the host process identifier
	__declspec(property(get=getHostProcessId)) DWORD HostProcessId;
	DWORD getHostProcessId(void) const { return m_host->ProcessId; }

	// ProcessId
	//
	// Gets the virtual machine process identifier
	__declspec(property(get=getProcessId)) int ProcessId;
	int getProcessId(void) const { return m_processid; }

	// StackImage
	//
	// Gets the location of the stack image in the hosted process
	__declspec(property(get=getStackImage)) const void* StackImage;
	const void* getStackImage(void) const { return m_startinfo.StackImage; }

	// StackImageLength
	//
	// Gets the length of the stack image in the hosted process
	__declspec(property(get=getStackImageLength)) size_t StackImageLength;
	size_t getStackImageLength(void) const { return m_startinfo.StackImageLength; }

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

	// Forward Declarations
	//
	struct StartupInfo;

	// Instance Constructor
	//
	Process(std::unique_ptr<Host>&& host, const FileSystem::AliasPtr& rootdir, const FileSystem::AliasPtr& workingdir, StartupInfo&& startinfo, std::vector<std::unique_ptr<MemorySection>>&& sections);
	friend class std::_Ref_count_obj<Process>;

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// MIN_HANDLE_INDEX
	//
	// Minimum allowable file system handle index (file descriptor)
	static const int MIN_HANDLE_INDEX = 3;

	// StartupInfo
	//
	// Information generated when the host process was created that is
	// required for it to know how to get itself up and running
	struct StartupInfo
	{
		void*		EntryPoint;				// Execution entry point
		void*		ProgramBreak;			// Pointer to the program break;
		void*		StackImage;				// Pointer to the stack image
		size_t		StackImageLength;		// Length of the stack image
	};

	// handle_map_t
	//
	// Collection of file system handles, keyed on the index (file descriptor)
	using handle_map_t = std::unordered_map<int, FileSystem::HandlePtr>;

	// section_map_hash_t
	//
	// Hash function for the section_map_t collection
	struct section_map_hash_t 
	{ 
		// Shift out the lower 16 bits of the key (base address), they should always be zero due to allocation granularity
		size_t operator() (void* const element) const { return uintptr_t(element) >> 16; }
	};

	// section_map_t
	//
	// Collection of std::unique_ptr<MemorySection> objects
	using section_map_t = std::unordered_map<void*, std::unique_ptr<MemorySection>, section_map_hash_t>;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// AllocateMemory
	//
	// Allocates and commits memory in the process virtual address space
	void* AllocateMemory(size_t length, uint32_t protection) { return AllocateMemory(nullptr, length, protection); }
	void* AllocateMemory(void* address, size_t length, uint32_t protection);

	// CheckHostProcessClass (static)
	//
	// Verifies that the created host process type matches what is expected
	template <ElfClass _class>
	static void CheckHostProcessClass(HANDLE process);

	// ReleaseMemory
	//
	// Decommits and releases memory from the process virtual address space
	void ReleaseMemory(void* address, size_t length);

	//-------------------------------------------------------------------------
	// Member Variables

	std::unique_ptr<Host>	m_host;				// Hosted windows process
	StartupInfo				m_startinfo;		// Hosted process start information

	////

	int						m_processid = 1;
	void*					m_tidaddress = nullptr;

	// MEMORY MANAGEMENT
	//
	void*							m_break;		// Current program break
	Concurrency::reader_writer_lock	m_sectionlock;	// Section collection lock
	section_map_t					m_sections;		// Allocated memory sections

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
