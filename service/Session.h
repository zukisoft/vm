//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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

#include <memory>
#include <unordered_map>

#pragma warning(push, 4)

// Forward Declarations
//
class Pid;
class Process;
class ProcessGroup;
class VirtualMachine;

//-----------------------------------------------------------------------------
// Session
//
// Implements a session, which is a collection of process groups that are 
// associated with a controlling terminal (stdin/stdout)

class Session
{
public:

	// Destructor
	//
	~Session();

	//-------------------------------------------------------------------------
	// Friend Functions

	// AddSessionProcess
	//
	// Adds a process to a session
	friend std::shared_ptr<Session> AddSessionProcess(std::shared_ptr<Session> sesson, std::shared_ptr<Process> process);

	// AddSessionProcessGroup
	//
	// Adds a process group to a session
	friend std::shared_ptr<Session> AddSessionProcessGroup(std::shared_ptr<Session> sesson, std::shared_ptr<ProcessGroup> pgroup);

	// RemoveSessionProcess
	//
	// Removes a process from a session
	friend void RemoveSessionProcess(std::shared_ptr<Session> session, Process const* process);

	// RemoveSessionProcessGroup
	//
	// Removes a process group from the collection
	friend void RemoveSessionProcessGroup(std::shared_ptr<Session> session, ProcessGroup const* pgroup);

	// SwapSessionProcess
	//
	// Swaps a process instance from one session to another
	friend std::shared_ptr<Session> SwapSessionProcess(std::shared_ptr<Session> source, std::shared_ptr<Session> dest, Process const* process);

	//-------------------------------------------------------------------------
	// Member Functions

	// Create (static)
	//
	// Creates a new session instance
	static std::shared_ptr<Session> Create(std::shared_ptr<Pid> sid, std::shared_ptr<VirtualMachine> vm);

	//-------------------------------------------------------------------------
	// Properties

	// SessionId
	//
	// Gets the session identifier
	__declspec(property(get=getSessionId)) std::shared_ptr<Pid> SessionId;
	std::shared_ptr<Pid> getSessionId(void) const;

	// VirtualMachine
	//
	// Gets a reference to the parent VirtualMachine instance
	__declspec(property(get=getVirtualMachine)) std::shared_ptr<class VirtualMachine> VirtualMachine;
	std::shared_ptr<class VirtualMachine> getVirtualMachine(void) const;

private:

	Session(Session const&)=delete;
	Session& operator=(Session const&)=delete;

	// pgroup_map_t
	//
	// Collection of process group instances
	using pgroup_map_t = std::unordered_map<ProcessGroup const*, std::weak_ptr<ProcessGroup>>;

	// pid_t
	//
	// Pid shared pointer
	using pid_t = std::shared_ptr<Pid>;

	// process_map_t
	//
	// Collection of process instances
	using process_map_t = std::unordered_map<Process const*, std::weak_ptr<Process>>;

	// vm_t
	//
	// VirtualMachine shared pointer
	using vm_t = std::shared_ptr<class VirtualMachine>;

	// Instance Constructor
	//
	Session(pid_t sid, vm_t vm);
	friend class std::_Ref_count_obj<Session>;

	//-------------------------------------------------------------------------
	// Member Variables

	pid_t const						m_sid;			// Session identifier
	vm_t const						m_vm;			// VirtualMachine instance
	pgroup_map_t					m_pgroups;		// Collection of process groups
	process_map_t					m_processes;	// Collection of processes
	mutable sync::critical_section	m_cs;			// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SESSION_H_
