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
#include "Session.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Session Constructor (private)
//
// Arguments:
//
//	sid			- Session identifier
//	ns			- Namespace instance for the session

Session::Session(uapi::pid_t sid, const std::shared_ptr<Namespace>& ns) : m_sid(sid), m_ns(ns) {}

//-----------------------------------------------------------------------------
// Session::Create (static)
//
// Creates a new Session instance
//
// Arguments:
//
//	sid			- Session identifier
//	ns			- Namespace instance for the session

std::shared_ptr<Session> Session::Create(uapi::pid_t sid, const std::shared_ptr<Namespace>& ns)
{
	// Create an empty session instance and the required initial process group
	auto session = std::make_shared<Session>(sid, ns);
	auto pgroup = ProcessGroup::Create(session, sid, ns);
	
	// Add the process group to the session's collection before returning it
	pgroup_map_lock_t::scoped_lock writer(session->m_pgroupslock);
	session->m_pgroups.insert(std::make_pair(sid, pgroup));

	return session;
}

//-----------------------------------------------------------------------------
// Session::getProcessGroup
//
// Gets a contained ProcessGroup instance via it's PGID

std::shared_ptr<::ProcessGroup> Session::getProcessGroup(uapi::pid_t pgid)
{
	pgroup_map_lock_t::scoped_lock_read reader(m_pgroupslock);

	// Attempt to locate the ProcessGroup instance associated with this PGID
	const auto& iterator = m_pgroups.find(pgid);
	if(iterator == m_pgroups.end()) throw LinuxException(LINUX_ESRCH);

	return iterator->second;
}

//-----------------------------------------------------------------------------
// Session::getSessionId
//
// Gets the session identifier

uapi::pid_t Session::getSessionId(void) const
{
	return m_sid;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
