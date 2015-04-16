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
//	vm			- Parent VirtualMachine instance
//	sid			- Session identifier

Session::Session(const std::shared_ptr<::VirtualMachine>& vm, uapi::pid_t sid) : m_vm(vm), m_sid(sid) 
{
}

//-----------------------------------------------------------------------------
// Session::FromExecutable (static)
//
// Creates a new Session instance from an Executable
//
// Arguments:
//
//	vm			- Parent VirtualMachine instance
//	sid			- Session identifier
//	ns			- Namespace instance
//	executable	- Executable instance to use to seed the session

std::shared_ptr<Session> Session::FromExecutable(const std::shared_ptr<::VirtualMachine>& vm, uapi::pid_t sid,
	const std::unique_ptr<Executable>& executable)
{
	// Create and initialize a new session instance with a new process group
	auto session = std::make_shared<Session>(vm, sid);
	session->m_pgroups.emplace(std::make_pair(sid, ProcessGroup::FromExecutable(session, sid, executable)));

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
// Session::getVirtualMachine
//
// Gets a reference to the parent virtual machine instance

std::shared_ptr<::VirtualMachine> Session::getVirtualMachine(void) const
{
	return m_vm.lock();
}

//-----------------------------------------------------------------------------

#pragma warning(pop)