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
#include "Executable.h"
#include "Process.h"
#include "ProcessGroup.h"
#include "ProcessModel.h"
#include "_VmOld.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Session
//
// Implements a session, which is a collection of process groups that are 
// associated with a controlling terminal (stdin/stdout).

class Session : public ProcessModel::Parent<::ProcessGroup>, public std::enable_shared_from_this<Session>
{
public:

	// Destructor
	//
	virtual ~Session();

	//-------------------------------------------------------------------------
	// Member Functions

	// FromExecutable (static)
	//
	// Creates a new Session from an Executable instance
	static std::shared_ptr<Session> FromExecutable(const std::shared_ptr<::_VmOld>& vm, uapi::pid_t sid,
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

	// _VmOld
	//
	// Gets the parent virtual machine instance
	__declspec(property(get=get_VmOld)) std::shared_ptr<::_VmOld> _VmOld;
	std::shared_ptr<::_VmOld> get_VmOld(void) const;

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
	using pgroup_map_lock_t = sync::reader_writer_lock;

	// Instance Constructor
	//
	Session(const std::shared_ptr<::_VmOld>& vm, uapi::pid_t sid);
	friend class std::_Ref_count_obj<Session>;

	//-------------------------------------------------------------------------
	// ProcessModel::Parent<> Implementation

	virtual std::shared_ptr<ProcessModel::Parent<::ProcessGroup>> getSharedPointer(void)
	{
		// todo: move me into CPP file
		return std::static_pointer_cast<ProcessModel::Parent<::ProcessGroup>>(shared_from_this());
	}

	//-------------------------------------------------------------------------
	// Member Variables

	const std::weak_ptr<::_VmOld>	m_vm;			// Parent _VmOld
	const uapi::pid_t						m_sid;			// Session identifier
	pgroup_map_t							m_pgroups;		// Owned process groups
	pgroup_map_lock_t						m_pgroupslock;	// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SESSION_H_
