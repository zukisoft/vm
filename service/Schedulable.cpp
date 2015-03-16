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
#include "Schedulable.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Schedulable Constructor (protected)
//
// Arguments:
//
//	state		- Initial execution state to assign

Schedulable::Schedulable(ExecutionState state) : m_state(state), m_exitcode(0)
{
	// Create a Win32 auto reset event to signal state changes
	m_statechanged = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if(!m_statechanged) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// Schedulable Destructor

Schedulable::~Schedulable()
{
	CloseHandle(m_statechanged);
}

//-----------------------------------------------------------------------------
// Schedulable::ChangeState (private)
//
// Changes the state of the object and signals the event
//
// Arguments:
//
//	newstate	- New execution state
//	fireevent	- Flag to fire the state changed event
//	exitcode	- Exit code to be assigned

void Schedulable::ChangeState(ExecutionState newstate, bool fireevent, int exitcode)
{
	std::lock_guard<std::mutex> critsec(m_statelock);

	// If the new state differs from previous, change and optionally signal
	if(newstate != m_state) {

		m_state = newstate;
		if((fireevent) && (!SetEvent(m_statechanged))) throw Win32Exception();
	}

	m_exitcode = exitcode;				// Update the exit code
}

//-----------------------------------------------------------------------------
// Schedulable::getExitCode
//
// Gets the exit code for the unit of execution

int Schedulable::getExitCode(void)
{
	std::lock_guard<std::mutex> critsec(m_statelock);
	return m_exitcode;
}

//-----------------------------------------------------------------------------
// Schedulable::MakeExitCode (static, protected)
//
// Generates an exit code for a unit of execution
//
// Arguments:
//
//	status		- Object return value
//	signal		- Signal number to embed in the exit code
//	coredump	- Flag if the exit caused a core dump

int Schedulable::MakeExitCode(int status, int signal, bool coredump)
{
	// Create the packed exit code for the task, which is a 16-bit value that
	// contains the actual exit code in the upper 8 bits and flags in the lower 8
	return ((status & 0xFF) << 8) | (signal & 0xFF) | (coredump ? 0x80 : 0);
}

//-----------------------------------------------------------------------------
// Schedulable::Resumed (protected)
//
// Indicates that the object has resumed execution
//
// Arguments:
//
//	NONE

void Schedulable::Resumed(void)
{
	// Change the state to running with an exit code of 0xFFFF
	ChangeState(ExecutionState::Running, true, 0xFFFF);
}

//-----------------------------------------------------------------------------
// Schedulable::Started (protected)
//
// Indicates that the object has started execution
//
// Arguments:
//
//	NONE

void Schedulable::Started(void)
{
	// Change the state to running without signaling the event
	ChangeState(ExecutionState::Running, false, 0x0000);
}

//-----------------------------------------------------------------------------
// Schedulable::getState
//
// Gets the current state of the schedulable object

Schedulable::ExecutionState Schedulable::getState(void)
{
	std::lock_guard<std::mutex> critsec(m_statelock);
	return m_state;
}

//-----------------------------------------------------------------------------
// Schedulable::getStateChanged
//
// Gets the Win32 event object handle used to signal state changes

HANDLE Schedulable::getStateChanged(void) const
{
	return m_statechanged;
}

//-----------------------------------------------------------------------------
// Schedulable::Suspended (protected)
//
// Indicates that execution has been suspended
//
// Arguments:
//
//	NONE

void Schedulable::Suspended(void)
{
	// Change the state to suspended with an exit code of 0x007F
	ChangeState(ExecutionState::Stopped, true, 0x007F);
}

//-----------------------------------------------------------------------------
// Schedulable::Terminated (protected)
//
// Indicates that execution has terminated
//
// Arguments:
//
//	exitcode		- Exit code for the task

void Schedulable::Terminated(int exitcode)
{
	// Change the state to suspended with an exit code of 0x007F
	ChangeState(ExecutionState::Terminated, true, exitcode);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
