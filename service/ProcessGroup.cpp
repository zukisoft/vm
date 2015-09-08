//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
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
#include "ProcessGroup.h"

#include "LinuxException.h"
#include "Pid.h"
#include "Process.h"
#include "Session.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// AddProcessGroupProcess
//
// Adds a process into a process group
//
// Arguments:
//
//	pgroup		- ProcessGroup instance to operate against
//	process		- Process instance to be added

std::shared_ptr<ProcessGroup> AddProcessGroupProcess(std::shared_ptr<ProcessGroup> pgroup, std::shared_ptr<Process> process)
{
	sync::critical_section::scoped_lock cs{ pgroup->m_cs };
	if(!pgroup->m_processes.emplace(process.get(), process).second) throw LinuxException{ LINUX_ENOMEM };

	return pgroup;
}

//-----------------------------------------------------------------------------
// RemoveProcessGroupProcess
//
// Removes a process from a process group
//
// Arguments:
//
//	pgroup		- ProcessGroup instance to operate against
//	process		- Process instance to be removed

void RemoveProcessGroupProcess(std::shared_ptr<ProcessGroup> pgroup, const Process* process)
{
	sync::critical_section::scoped_lock cs{ pgroup->m_cs };
	pgroup->m_processes.erase(process);
}

//-----------------------------------------------------------------------------
// SwapProcessGroupProcess
//
// Moves a process from one process group into another process group
//
// Arguments:
//
//	source		- Source process group
//	dest		- Destination process group
//	process		- Process instance to be swapped

std::shared_ptr<ProcessGroup> SwapProcessGroupProcess(std::shared_ptr<ProcessGroup> source, std::shared_ptr<ProcessGroup> dest, const Process* process)
{
	sync::critical_section::scoped_lock csfrom{ source->m_cs };
	sync::critical_section::scoped_lock csto{ dest->m_cs };

	// Move the weak_ptr to the process from the source process group into the destination process group,
	// watch that unodered_map::at() will throw std::out_of_range exception if the key is not found
	try {

		if(!dest->m_processes.emplace(process, std::move(source->m_processes.at(process))).second) throw LinuxException{ LINUX_ENOMEM };
		source->m_processes.erase(process);
	}
	catch(std::out_of_range&) { throw LinuxException{ LINUX_ESRCH }; }

	return dest;
}

//-----------------------------------------------------------------------------
// ProcessGroup Constructor (private)
//
// Arguments:
//
//	pgid		- Process group identifier
//	session		- Parent session instance

ProcessGroup::ProcessGroup(pid_t pgid, session_t session) : m_pgid(std::move(pgid)), m_session(std::move(session))
{
}

//-----------------------------------------------------------------------------
// ProcessGroup Destructor

ProcessGroup::~ProcessGroup()
{
	RemoveSessionProcessGroup(m_session, this);
}

//-----------------------------------------------------------------------------
// ProcessGroup::Create (static)
//
// Creates a new process group
//
// Arguments:
//
//	pgid		- Pid instance to assign to the process group
//	session		- Session in which the process group will be a member

std::shared_ptr<ProcessGroup> ProcessGroup::Create(std::shared_ptr<Pid> pgid, std::shared_ptr<class Session> session)
{
	// Create the ProcessGroup instance
	auto pgroup = std::make_shared<ProcessGroup>(std::move(pgid), session);

	// The parent container link has to be established after the shared_ptr has been constructed
	AddSessionProcessGroup(session, pgroup);

	return pgroup;
}

//-----------------------------------------------------------------------------
// ProcessGroup::getProcessGroupId
//
// Gets the process group identifier

std::shared_ptr<Pid> ProcessGroup::getProcessGroupId(void) const
{
	return m_pgid;
}

//-----------------------------------------------------------------------------
// ProcessGroup::getSession
//
// Gets a reference to the parent Session instance

std::shared_ptr<class Session> ProcessGroup::getSession(void) const
{
	return m_session;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
