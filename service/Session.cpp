//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"}; to deal
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

#include "LinuxException.h"
#include "Pid.h"
#include "Process.h"
#include "ProcessGroup.h"
#include "VirtualMachine.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// AddSessionProcess
//
// Adds a process into a session
//
// Arguments:
//
//	session		- Session instance to operate against
//	process		- Process instance to be added

std::shared_ptr<Session> AddSessionProcess(std::shared_ptr<Session> session, std::shared_ptr<Process> process)
{
	sync::critical_section::scoped_lock cs{ session->m_cs };
	if(!session->m_processes.emplace(process.get(), process).second) throw LinuxException{ LINUX_ENOMEM };

	return session;
}

//-----------------------------------------------------------------------------
// AddSessionProcessGroup
//
// Adds a process group into a session
//
// Arguments:
//
//	session		- Session instance to operate against
//	pgroup		- Process group instance to be added

std::shared_ptr<Session> AddSessionProcessGroup(std::shared_ptr<Session> session, std::shared_ptr<ProcessGroup> pgroup)
{
	sync::critical_section::scoped_lock cs{ session->m_cs };
	if(!session->m_pgroups.emplace(pgroup.get(), pgroup).second) throw LinuxException{ LINUX_ENOMEM };

	return session;
}

//-----------------------------------------------------------------------------
// RemoveSessionProcess
//
// Removes a process from a session
//
// Arguments:
//
//	session		- Session instance to operate against
//	process		- Process instance to be removed

void RemoveSessionProcess(std::shared_ptr<Session> session, Process const* process)
{
	sync::critical_section::scoped_lock cs{ session->m_cs };
	session->m_processes.erase(process);
}

//-----------------------------------------------------------------------------
// RemoveSessionProcessGroup
//
// Removes a process group from a session
//
// Arguments:
//
//	session		- Session instance to operate against
//	pgroup		- ProcessGroup instance to be removed

void RemoveSessionProcessGroup(std::shared_ptr<Session> session, ProcessGroup const* pgroup)
{
	sync::critical_section::scoped_lock cs{ session->m_cs };
	session->m_pgroups.erase(pgroup);
}

//-----------------------------------------------------------------------------
// SwapSessionProcess
//
// Moves a process from one session into another session
//
// Arguments:
//
//	source		- Source session
//	dest		- Destination session
//	process		- Process instance to be swapped

std::shared_ptr<Session> SwapSessionProcess(std::shared_ptr<Session> source, std::shared_ptr<Session> dest, Process const* process)
{
	sync::critical_section::scoped_lock csfrom{ source->m_cs };
	sync::critical_section::scoped_lock csto{ dest->m_cs };

	// Move the weak_ptr to the process from the source session into the destination session, watch that
	// unodered_map::at() will throw std::out_of_range exception if the key is not found, convert to ESRCH
	try {

		if(!dest->m_processes.emplace(process, std::move(source->m_processes.at(process))).second) throw LinuxException{ LINUX_ENOMEM };
		source->m_processes.erase(process);
	}
	catch(std::out_of_range&) { throw LinuxException{ LINUX_ESRCH }; }

	return dest;
}

//-----------------------------------------------------------------------------
// Session Constructor (private)
//
// Arguments:
//
//	sid		- Session identifier
//	vm		- Parent VirtualMachine instance

Session::Session(pid_t sid, vm_t vm) : m_sid(std::move(sid)), m_vm(std::move(vm))
{
}

//-----------------------------------------------------------------------------
// Session Destructor

Session::~Session()
{
	RemoveVirtualMachineSession(m_vm, this);
}

//-----------------------------------------------------------------------------
// Session::Create (static)
//
// Creates a session instance
//
// Arguments:
//
//	sid			- Pid instance to assign to the session
//	vm			- VirtualMachine that this session will become a member of

std::shared_ptr<Session> Session::Create(std::shared_ptr<Pid> sid, std::shared_ptr<class VirtualMachine> vm)
{
	// Create the Session instance
	auto session = std::make_shared<Session>(std::move(sid), vm);

	// The parent container link has to be established after the shared_ptr has been constructed
	AddVirtualMachineSession(vm, session);

	return session;
}

//-----------------------------------------------------------------------------
// Session::getSessionId
//
// Gets the session identifier

std::shared_ptr<Pid> Session::getSessionId(void) const
{
	return m_sid;
}

//-----------------------------------------------------------------------------
// Session::getVirtualMachine
//
// Gets a reference to the parent VirtualMachine instance

std::shared_ptr<class VirtualMachine> Session::getVirtualMachine(void) const
{
	return m_vm;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
