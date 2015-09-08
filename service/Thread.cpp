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
#include "Thread.h"

#include "Pid.h"
#include "Process.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Thread Constructor (private)
//
// Arguments:
//
//	tid		- Thread identifier to assign to the thread
//	process	- Parent Process instance

Thread::Thread(pid_t tid, process_t process) : m_tid(std::move(tid)), m_process(std::move(process))
{
}

//-----------------------------------------------------------------------------
// Thread Destructor

Thread::~Thread()
{
	RemoveProcessThread(m_process, this);
}

//-----------------------------------------------------------------------------
// Thread::Create (static)
//
// Creates a new thread instance
//
// Arguments:
//
//	tid			- Thread identifier to assign to the thread
//	process		- Process that the thread will become a member of

std::shared_ptr<Thread> Thread::Create(std::shared_ptr<Pid> tid, std::shared_ptr<class Process> process)
{
	// Create the Thread instance
	auto thread = std::make_shared<Thread>(tid, process);

	// The parent container link has to be established after the shared_ptr has been constructed
	AddProcessThread(process, thread);

	return thread;
}

//-----------------------------------------------------------------------------
// Thread::getProcess
//
// Gets a reference to the parent Process instance

std::shared_ptr<class Process> Thread::getProcess(void) const
{
	return m_process;
}

//-----------------------------------------------------------------------------
// Thread::getThreadId
//
// Gets the thread identifier

std::shared_ptr<Pid> Thread::getThreadId(void) const
{
	return m_tid;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
