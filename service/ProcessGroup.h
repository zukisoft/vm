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

#ifndef __PROCESSGROUP_H_
#define __PROCESSGROUP_H_
#pragma once

#include <map>
#include <memory>
#include <concrt.h>
#include <linux/types.h>
#include "Process.h"
#include "ProcessModel.h"

#pragma warning(push, 4)

// Forward Declarations
//
class Session;

//-----------------------------------------------------------------------------
// ProcessGroup
//
// Implements a process group, which is a collection of processes that can be 
// managed as a single entity

class ProcessGroup : public ProcessModel::Child<ProcessGroup>
{
public:

	// Destructor
	//
	virtual ~ProcessGroup();

	//-------------------------------------------------------------------------
	// Member Functions

	// AttachProcess
	//
	// Attaches an existing Process to this ProcessGroup
	void AttachProcess(const std::shared_ptr<::Process>& process);

	// FromExecutable (static)
	//
	// Creates a new ProcessGroup from an Executable instance
	static std::shared_ptr<ProcessGroup> FromExecutable(const std::shared_ptr<VirtualMachine>& vm, const std::shared_ptr<::Session>& session, 
		uapi::pid_t pgid, const std::unique_ptr<Executable>& executable);

	// ReleaseProcess
	//
	// Removes a process from this process group
	void ReleaseProcess(uapi::pid_t pid);

	//-------------------------------------------------------------------------
	// Properties

	// Process
	//
	// Gets a Process instance by identifier
	__declspec(property(get=getProcess)) std::shared_ptr<::Process> Process[];
	std::shared_ptr<::Process> getProcess(uapi::pid_t pid);

	// ProcessGroupId
	//
	// Gets the virtual process group identifier
	__declspec(property(get=getProcessGroupId)) uapi::pid_t ProcessGroupId;
	uapi::pid_t getProcessGroupId(void) const;

	// Session
	//
	// Gets a reference to the containing session instance
	__declspec(property(get=getSession)) std::shared_ptr<::Session> Session;
	std::shared_ptr<::Session> getSession(void) const;

private:

	ProcessGroup(const ProcessGroup&)=delete;
	ProcessGroup& operator=(const ProcessGroup&)=delete;

	// process_map_t
	//
	// Collection of owned Process instances
	using process_map_t = std::map<uapi::pid_t, std::shared_ptr<::Process>>;

	// process_map_lock_t
	//
	// Synchronization object for process_map_t
	using process_map_lock_t = Concurrency::reader_writer_lock;

	// Instance Constructor
	//
	ProcessGroup(const std::shared_ptr<VirtualMachine>& vm, const std::shared_ptr<::Session>& session, uapi::pid_t pgid);
	friend class std::_Ref_count_obj<ProcessGroup>;

	//-------------------------------------------------------------------------
	// Member Variables

	const std::weak_ptr<VirtualMachine>	m_vm;				// Parent vm instance
	const std::weak_ptr<::Session>		m_session;			// Parent session instance
	const uapi::pid_t					m_pgid;				// Process group identifier
	process_map_t						m_processes;		// Owned processes
	process_map_lock_t					m_processeslock;	// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESSGROUP_H_
