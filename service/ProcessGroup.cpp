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
#include "Session.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// ProcessGroup Constructor (private)
//
// Arguments:
//
//	vm				- Parent virtual machine instance
//	session			- Parent session instance
//	pgid			- Process group identifier

ProcessGroup::ProcessGroup(const std::shared_ptr<_VmOld>& vm, const std::shared_ptr<::Session>& session, uapi::pid_t pgid) 
	: m_vm(vm), m_session(session), m_pgid(pgid), ProcessModel::Child<ProcessGroup>(pgid)
{
}

//-----------------------------------------------------------------------------
// ProcessGroup Destructor

ProcessGroup::~ProcessGroup()
{
	auto vm = m_vm.lock();			// Parent virtual machine instance

	// There should not be any processes left in this process group
	_ASSERTE(m_processes.size() == 0);

	// Release the PIDs for any processes that still exist in the collection
	for(const auto& iterator : m_processes)
		if((iterator.first != m_pgid) && vm) vm->ReleasePID(iterator.first);
}

//-----------------------------------------------------------------------------
// ProcessGroup::AttachProcess
//
// Attaches an existing Process instance to this process group
//
// Arguments:
//
//	process		- Process to add to this group

void ProcessGroup::AttachProcess(const std::shared_ptr<::Process>& process)
{
	process_map_lock_t::scoped_lock_write writer(m_processeslock);

	// TODO: TEMPORARY; NEEDS EXCEPTION HANDLING
	m_processes.insert(std::make_pair(process->ProcessId, process));
}

//-----------------------------------------------------------------------------
// ProcessGroup::FromExecutable (static)
//
// Creates a new ProcessGroup instance from an Executable
//
// Arguments:
//
//	vm			- Parent _VmOld instance
//	session		- Parent Session instance
//	pgid		- Process group identifier
//	executable	- Executable instance to use to seed the session

std::shared_ptr<ProcessGroup> ProcessGroup::FromExecutable(const std::shared_ptr<_VmOld>& vm, const std::shared_ptr<::Session>& session, 
	uapi::pid_t pgid, const std::unique_ptr<Executable>& executable)
{
	// Create and initialize a new process group instance with a new process
	auto pgroup = std::make_shared<ProcessGroup>(vm, session, pgid);
	pgroup->m_processes.emplace(std::make_pair(pgid, Process::FromExecutable(vm, pgroup, pgid, executable)));

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
// ProcessGroup::ReleaseProcess
//
// Removes a process from this process group
//
// Arguments:
//
//	pid			- Process identifier

void ProcessGroup::ReleaseProcess(uapi::pid_t pid)
{
	auto vm = m_vm.lock();				// Parent virtual machine instance
	auto session = m_session.lock();	// Parent session instance

	process_map_lock_t::scoped_lock_write writer(m_processeslock);

	// Remove the specified process group from the collection
	if(m_processes.erase(pid) == 0) throw LinuxException(LINUX_ESRCH);

	// Release the process identifier if it wasn't the group leader
	if((pid != m_pgid) && vm) vm->ReleasePID(pid);

	// If this was the last process, release the process group as well
	if((m_processes.size() == 0) && session) session->ReleaseProcessGroup(m_pgid);
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
