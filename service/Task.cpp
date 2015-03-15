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
#include "Task.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Task Constructor (protected)
//
// Arguments:
//
//	state		- Initial object state to assign

Task::Task(State state) : m_state(state), m_exitcode(0)
{
	// Create a Win32 auto reset event to signal state changes
	m_statechanged = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if(!m_statechanged) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// Task Destructor

Task::~Task()
{
	CloseHandle(m_statechanged);
}

//-----------------------------------------------------------------------------
// Task::ChangeState (private)
//
// Changes the state of the task and signals the event
//
// Arguments:
//
//	newstate	- New task state
//	fireevent	- Flag to fire the state changed event
//	exitcode	- Exit code to be assigned

void Task::ChangeState(State newstate, bool fireevent, int exitcode)
{
	std::lock_guard<std::mutex> critsec(m_statelock);

	// If the new state differs from previous, change and optionally signal
	if(newstate != m_state) {

		m_state = newstate;
		if((fireevent) && (!SetEvent(m_statechanged))) throw Win32Exception();
	}

	// Always update the exit code
	m_exitcode = exitcode;
}

//-----------------------------------------------------------------------------
// Task::getCurrentState
//
// Gets the current state of the task

Task::State Task::getCurrentState(void)
{
	std::lock_guard<std::mutex> critsec(m_statelock);
	return m_state;
}

//-----------------------------------------------------------------------------
// Task::getExitCode
//
// Gets the exit code for the task

int Task::getExitCode(void)
{
	std::lock_guard<std::mutex> critsec(m_statelock);
	return m_exitcode;
}

//-----------------------------------------------------------------------------
// Task::MakeExitCode (static, protected)
//
// Generates an exit code for a task
//
// Arguments:
//
//	status		- Object return value
//	signal		- Signal number to embed in the exit code
//	coredump	- Flag if the exit caused a core dump

int Task::MakeExitCode(int status, int signal, bool coredump)
{
	// Create the packed exit code for the process, which is a 16-bit value that
	// contains the actual exit code in the upper 8 bits and flags in the lower 8
	return ((status & 0xFF) << 8) | (signal & 0xFF) | (coredump ? 0x80 : 0);
}

//-----------------------------------------------------------------------------
// Task::Resumed (protected)
//
// Indicates that the task has resumed from suspension
//
// Arguments:
//
//	NONE

void Task::Resumed(void)
{
	// Change the state to running with an exit code of 0xFFFF
	ChangeState(State::Running, true, 0xFFFF);
}

//-----------------------------------------------------------------------------
// Task::Started (protected)
//
// Indicates that the task has started
//
// Arguments:
//
//	NONE

void Task::Started(void)
{
	// Change the state to running without signaling the event
	ChangeState(State::Running, false, 0x0000);
}

//-----------------------------------------------------------------------------
// Task::getStateChanged
//
// Gets the Win32 event object handle used to signal state changes

HANDLE Task::getStateChanged(void) const
{
	return m_statechanged;
}

//-----------------------------------------------------------------------------
// Task::Suspended (protected)
//
// Indicates that the task has been suspended
//
// Arguments:
//
//	NONE

void Task::Suspended(void)
{
	// Change the state to suspended with an exit code of 0x007F
	ChangeState(State::Stopped, true, 0x007F);
}

//-----------------------------------------------------------------------------
// Task::Terminated (protected)
//
// Indicates that the task has terminated
//
// Arguments:
//
//	exitcode		- Exit code for the task

void Task::Terminated(int exitcode)
{
	// Change the state to suspended with an exit code of 0x007F
	ChangeState(State::Terminated, true, exitcode);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
