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

#include "stdafx.h"
#include "ProcessGroup.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// ProcessGroup Constructor (private)
//
// Arguments:
//
//	session			- Parent session instance
//	pgid			- Process group identifier
//	ns				- Namespace instance for the process group

ProcessGroup::ProcessGroup(const std::shared_ptr<::Session>& session, uapi::pid_t pgid, const std::shared_ptr<Namespace>& ns) 
	: m_session(session), m_pgid(pgid), m_ns(ns) {}

//-----------------------------------------------------------------------------
// ProcessGroup::Create (static)
//
// Creates a new ProcessGroup instance
//
// Arguments:
//
//	session		- Parent Session instance
//	pgid		- Process group indentifier
//	ns			- Namespace instance

std::shared_ptr<ProcessGroup> ProcessGroup::Create(const std::shared_ptr<::Session>& session, 
	uapi::pid_t pgid, const std::shared_ptr<Namespace>& ns)
{
	// Create an empty process group instance and the required initial process
	auto pgroup = std::make_shared<ProcessGroup>(session, pgid, ns);
	auto process = nullptr;/// TODO: Process::Spawn(blah)
	
	// Add the process to process group's collection before returning it
	process_map_lock_t::scoped_lock writer(pgroup->m_processeslock);
	// TODO: pgroup->m_processes.insert(std::make_pair(pgid, blah));

	return pgroup;	
}

//-----------------------------------------------------------------------------
// Session::getProcess
//
// Gets a contained Process instance via it's PID

std::shared_ptr<::Process> ProcessGroup::getProcess(uapi::pid_t pid)
{
	process_map_lock_t::scoped_lock_read reader(m_processeslock);

	// Attempt to locate the Process instance associated with this PID
	const auto& iterator = m_processes.find(pid);
	if(iterator == m_processes.end()) throw LinuxException(LINUX_ESRCH);

	return iterator->second;
}

//-----------------------------------------------------------------------------
// ProcessGroup::getProcessGroupId
//
// Gets the process group identifier

uapi::pid_t ProcessGroup::getProcessGroupId(void) const
{
	return m_pgid;
}

//-----------------------------------------------------------------------------
// ProcessGroup::getSession
//
// Gets a reference to the containing session instance

std::shared_ptr<::Session> ProcessGroup::getSession(void) const
{
	return m_session.lock();
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
