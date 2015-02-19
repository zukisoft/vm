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

#ifndef __NEWPROCESS_H_
#define __NEWPROCESS_H_
#pragma once

#include <memory>
#include "elf_traits.h"
#include "Architecture.h"
#include "ElfImage.h"
#include "Executable.h"
#include "FileSystem.h"
#include "LinuxException.h"
#include "NativeHandle.h"
#include "ProcessHandles.h"
#include "ProcessHost.h"
#include "ProcessMemory.h"
#include "Thread.h"
#include "VirtualMachine.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Process
//
// Process represents a virtual machine process/thread group instance
//
//	POINTERS:		SHARED
//	EXCEPTIONS:		LINUXEXCEPTION

class NewProcess
{
public:

	// Destructor
	//
	~NewProcess();

	//-------------------------------------------------------------------------
	// Member Functions

	// Spawn (static)
	//
	// Spawns a new process instance
	static std::shared_ptr<NewProcess> Spawn(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t pid, const char_t* filename, const char_t* const* argv,
		const char_t* const* envp, const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir);

	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the process architecture code
	__declspec(property(get=getArchitecture)) ::Architecture Architecture;
	::Architecture getArchitecture(void) const;

	// ProcessId
	//
	// Gets the virtual process identifier
	__declspec(property(get=getProcessId)) uapi::pid_t ProcessId;
	uapi::pid_t getProcessId(void) const;

	// RootDirectory
	//
	// Gets/sets the process root directory alias
	__declspec(property(get=getRootDirectory, put=putRootDirectory)) std::shared_ptr<FileSystem::Alias> RootDirectory;
	std::shared_ptr<FileSystem::Alias> getRootDirectory(void) const;
	void putRootDirectory(const std::shared_ptr<FileSystem::Alias>& value);

	// WorkingDirectory
	//
	// Gets/sets the process working directory alias
	__declspec(property(get=getWorkingDirectory, put=putWorkingDirectory)) std::shared_ptr<FileSystem::Alias> WorkingDirectory;
	std::shared_ptr<FileSystem::Alias> getWorkingDirectory(void) const;
	void putWorkingDirectory(const std::shared_ptr<FileSystem::Alias>& value);

private:

	NewProcess(const NewProcess&)=delete;
	NewProcess& operator=(const NewProcess&)=delete;

	// Instance Constructors
	//
	NewProcess(const std::shared_ptr<VirtualMachine>& vm, ::Architecture architecture, const std::shared_ptr<NativeHandle>& process, uapi::pid_t pid, 
		const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir, std::unique_ptr<ProcessMemory>&& memory);
	friend class std::_Ref_count_obj<NewProcess>;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// FromExecutable<Architecture>
	//
	// Creates a new process instance from an Executable
	template<::Architecture architecture>
	static std::shared_ptr<NewProcess> FromExecutable(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t pid, const std::unique_ptr<Executable>& executable);

	//-------------------------------------------------------------------------
	// Member Variables

	std::shared_ptr<VirtualMachine>		m_vm;				// Virtual machine instance
	const ::Architecture				m_architecture;		// Process architecture
	std::shared_ptr<NativeHandle>		m_process;			// Native process handle
	const uapi::pid_t					m_pid;				// Process identifier

	// Memory
	//
	std::unique_ptr<ProcessMemory>		m_memory;			// Virtual address space

	// File System
	//
	std::shared_ptr<FileSystem::Alias>	m_rootdir;
	std::shared_ptr<FileSystem::Alias>	m_workingdir;

	// Threads
	//
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __NEWPROCESS_H_
