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
#include <concurrent_unordered_map.h>
#include <memory>
#include <linux/elf.h>
#include "ElfArguments.h"
#include "ElfClass.h"
#include "ElfImage.h"
#include "Exception.h"
#include "HeapBuffer.h"
#include "Host.h"
#include "IndexPool.h"
#include "LinuxException.h"
#include "Random.h"
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

	// Create (static)
	//
	// Creates a new process instance via an external Windows host binary
	template <ElfClass _class>
	static std::shared_ptr<Process> Create(const std::shared_ptr<VirtualMachine>& vm, const FileSystem::HandlePtr& handle,
		const uapi::char_t** argv, const uapi::char_t** envp, const tchar_t* hostpath, const tchar_t* hostargs);

	// GetHandle
	//
	// Accesses a file system handle referenced by the process
	FileSystem::HandlePtr GetHandle(int index);

	// RemoveHandle
	//
	// Removes a file system handle from the process
	void RemoveHandle(int index);

	// Resume
	//
	// Resumes the process from a suspended state
	void Resume(void) { _ASSERTE(m_host); m_host->Resume(); }

	// Suspend
	//
	// Suspends the process
	void Suspend(void) { _ASSERTE(m_host); m_host->Suspend(); }

	// Terminate
	//
	// Terminates the process
	void Terminate(int exitcode) { _ASSERTE(m_host); m_host->Terminate(-exitcode); }

	//-------------------------------------------------------------------------
	// Properties

	// EntryPoint
	//
	// Gets the entry point address of the hosted process
	__declspec(property(get=getEntryPoint)) const void* EntryPoint;
	const void* getEntryPoint(void) const { return m_startinfo.EntryPoint; }

	// HostProcessId
	//
	// Gets the host process identifier
	__declspec(property(get=getHostProcessId)) DWORD HostProcessId;
	DWORD getHostProcessId(void) const { return m_host->ProcessId; }

	// ProgramBreak
	//
	// Gets the initial program break for the process
	__declspec(property(get=getProgramBreak)) const void* ProgramBreak;
	const void* getProgramBreak(void) const { return m_startinfo.ProgramBreak; }

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

private:

	Process(const Process&)=delete;
	Process& operator=(const Process&)=delete;

	// Forward Declarations
	//
	struct StartupInfo;

	// Instance Constructor
	//
	Process(std::unique_ptr<Host>&& host, StartupInfo&& startinfo) : m_host(std::move(host)), m_startinfo(startinfo) {}
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
		const void*		EntryPoint;				// Execution entry point
		const void*		ProgramBreak;			// Pointer to the program break;
		const void*		StackImage;				// Pointer to the stack image
		size_t			StackImageLength;		// Length of the stack image
	};

	// SystemInfo
	//
	// Used to initialize a static SYSTEM_INFO structure
	struct SystemInfo : public SYSTEM_INFO
	{
		SystemInfo() { GetNativeSystemInfo(static_cast<SYSTEM_INFO*>(this)); }
	};

	// handle_map_t
	//
	// Collection of file system handles, keyed on the index (file descriptor)
	using handle_map_t = Concurrency::concurrent_unordered_map<int, FileSystem::HandlePtr>;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// CheckHostProcessClass (static)
	//
	// Verifies that the created host process type matches what is expected
	template <ElfClass _class>
	static void CheckHostProcessClass(HANDLE process);

	//-------------------------------------------------------------------------
	// Member Variables

	std::unique_ptr<Host>	m_host;				// Hosted windows process
	const StartupInfo		m_startinfo;		// Hosted process start information

	handle_map_t			m_handles;			// Process file system handles
	IndexPool<int>			m_indexpool { MIN_HANDLE_INDEX };

	static SystemInfo		s_sysinfo;			// System information
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESS_H_
