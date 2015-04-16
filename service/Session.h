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

#ifndef __SESSION_H_
#define __SESSION_H_
#pragma once

#include <map>
#include <memory>
#include <concrt.h>
#include <linux/types.h>
#include "Executable.h"
#include "Process.h"
#include "ProcessGroup.h"
#include "VirtualMachine.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Session
//
// Implements a session, which is a collection of process groups that are 
// associated with a controlling terminal (stdin/stdout).

class Session
{
public:

	// Destructor
	//
	~Session();

	//-------------------------------------------------------------------------
	// Member Functions

	// FromExecutable (static)
	//
	// Creates a new Session from an Executable instance
	static std::shared_ptr<Session> FromExecutable(const std::shared_ptr<::VirtualMachine>& vm, uapi::pid_t sid,
		const std::unique_ptr<Executable>& executable);

	// ReleaseProcessGroup
	//
	// Removes a ProcessGroup from this session instance
	void ReleaseProcessGroup(uapi::pid_t pgid);

	//-------------------------------------------------------------------------
	// Properties

	// ProcessGroup
	//
	// Gets a ProcessGroup instance by process group identifier
	__declspec(property(get=getProcessGroup)) std::shared_ptr<::ProcessGroup> ProcessGroup[];
	std::shared_ptr<::ProcessGroup> getProcessGroup(uapi::pid_t pgid);

	// SessionId
	//
	// Gets the session identifier
	__declspec(property(get=getSessionId)) uapi::pid_t SessionId;
	uapi::pid_t getSessionId(void) const;

	// VirtualMachine
	//
	// Gets the parent virtual machine instance
	__declspec(property(get=getVirtualMachine)) std::shared_ptr<::VirtualMachine> VirtualMachine;
	std::shared_ptr<::VirtualMachine> getVirtualMachine(void) const;

private:

	Session(const Session&)=delete;
	Session& operator=(const Session&)=delete;

	// pgroup_map_t
	//
	// Collection of owned ProcessGroup instances
	using pgroup_map_t = std::map<uapi::pid_t, std::shared_ptr<::ProcessGroup>>;

	// pgroup_map_lock_t
	//
	// Synchronization object for pgroup_map_t
	using pgroup_map_lock_t = Concurrency::reader_writer_lock;

	// Instance Constructor
	//
	Session(const std::shared_ptr<::VirtualMachine>& vm, uapi::pid_t sid);
	friend class std::_Ref_count_obj<Session>;

	//-------------------------------------------------------------------------
	// Member Variables

	std::weak_ptr<::VirtualMachine>	m_vm;				// Parent VirtualMachine
	const uapi::pid_t				m_sid;				// Session identifier
	pgroup_map_t					m_pgroups;			// Owned process groups
	pgroup_map_lock_t				m_pgroupslock;		// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SESSION_H_
