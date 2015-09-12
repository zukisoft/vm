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

#ifndef __THREAD_H_
#define __THREAD_H_
#pragma once

#include <memory>

#pragma warning(push, 4)

// Forward Declarations
//
class Pid;
class Process;

//-----------------------------------------------------------------------------
// Thread
//
// Implements a thread of execution within a process/thread group

class Thread
{
public:

	// Destructor
	//
	~Thread();

	//-------------------------------------------------------------------------
	// Member Functions

	// Create (static)
	//
	// Creates a new Thread instance
	static std::shared_ptr<Thread> Create(std::shared_ptr<Pid> tid, std::shared_ptr<class Process> process);

	//-------------------------------------------------------------------------
	// Properties

	// Process
	//
	// Gets a reference to the parent process instance
	__declspec(property(get=getProcess)) std::shared_ptr<class Process> Process;
	std::shared_ptr<class Process> getProcess(void) const;

	// ThreadId
	//
	// Gets the thread identifier
	__declspec(property(get=getThreadId)) std::shared_ptr<Pid> ThreadId;
	std::shared_ptr<Pid> getThreadId(void) const;

private:

	Thread(Thread const&)=delete;
	Thread& operator=(Thread const&)=delete;

	// pid_t
	//
	// Pid shared pointer
	using pid_t = std::shared_ptr<Pid>;

	// process_t
	//
	// Process shared pointer
	using process_t = std::shared_ptr<class Process>;

	// Instance Constructor
	//
	Thread(pid_t tid, process_t process);
	friend class std::_Ref_count_obj<Thread>;

	//-------------------------------------------------------------------------
	// Member Variables

	pid_t const					m_tid;			// Thread identifier
	process_t const 			m_process;		// Parent process instance
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __THREAD_H_
