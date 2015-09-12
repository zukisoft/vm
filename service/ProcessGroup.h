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

#include <memory>
#include <unordered_map>

#pragma warning(push, 4)

// Forward Declarations
//
class Pid;
class Process;
class Session;

//-----------------------------------------------------------------------------
// ProcessGroup
//
// Implements a process group, which is a collection of processes that can be 
// managed as a single entity

class ProcessGroup
{
public:

	// Destructor
	//
	~ProcessGroup();

	//-------------------------------------------------------------------------
	// Friend Functions

	// AddProcessGroupProcess
	//
	// Adds a process to a process group
	friend std::shared_ptr<ProcessGroup> AddProcessGroupProcess(std::shared_ptr<ProcessGroup> pgroup, std::shared_ptr<Process> process);

	// RemoveProcessGroupProcess
	//
	// Removes a process from the collection
	friend void RemoveProcessGroupProcess(std::shared_ptr<ProcessGroup> pgroup, Process const* process);

	// SwapProcessGroupProcess
	//
	// Swaps a process instance from one process group to another
	friend std::shared_ptr<ProcessGroup> SwapProcessGroupProcess(std::shared_ptr<ProcessGroup> source, std::shared_ptr<ProcessGroup> dest, Process const* process);

	//-------------------------------------------------------------------------
	// Member Functions

	// Create (static)
	//
	// Creates a new process group instance
	static std::shared_ptr<ProcessGroup> Create(std::shared_ptr<Pid> pgid, std::shared_ptr<class Session> session);

	//-------------------------------------------------------------------------
	// Properties

	// ProcessGroupId
	//
	// Gets the process group identifier
	__declspec(property(get=getProcessGroupId)) std::shared_ptr<Pid> ProcessGroupId;
	std::shared_ptr<Pid> getProcessGroupId(void) const;

	// Session
	//
	// Gets the session to which this process group belongs
	__declspec(property(get=getSession)) std::shared_ptr<class Session> Session;
	std::shared_ptr<class Session> getSession(void) const;

private:

	ProcessGroup(ProcessGroup const&)=delete;
	ProcessGroup& operator=(ProcessGroup const&)=delete;

	// pid_t
	//
	// Pid shared pointer
	using pid_t = std::shared_ptr<Pid>;

	// process_map_t
	//
	// Collection of process instances
	using process_map_t = std::unordered_map<Process const*, std::weak_ptr<Process>>;

	// session_t
	//
	// Session shared pointer
	using session_t = std::shared_ptr<class Session>;

	// Instance Constructor
	//
	ProcessGroup(pid_t pgid, session_t session);
	friend class std::_Ref_count_obj<ProcessGroup>;

	//-------------------------------------------------------------------------
	// Member Variables

	pid_t const						m_pgid;			// Process group identifier
	session_t const					m_session;		// Parent session instance
	process_map_t					m_processes;	// Collection of processes
	mutable sync::critical_section	m_cs;			// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESSGROUP_H_
