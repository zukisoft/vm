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

#ifndef __SCHEDULABLE_H_
#define __SCHEDULABLE_H_
#pragma once

#include <mutex>
#include "Win32Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Schedulable
//
// Abstraction of a schedulable unit of execution (processes, threads); provides
// a common interface for manipulating execution state

class Schedulable
{
public:

	// ExecutionState
	//
	// Strongly typed enumeration defining the execution state
	enum class ExecutionState {

		Stopped		= 0,		// Execution is suspended
		Running,				// Execution is running
		Terminated,				// Execution has terminated
	};

	// Destructor
	//
	virtual ~Schedulable();

	//-------------------------------------------------------------------------
	// Member Functions

	// Resume
	//
	// Resumes execution of the object
	virtual void Resume(void) = 0;

	// Start
	//
	// Starts execution of the object
	virtual void Start(void) = 0;

	// Suspend
	//
	// Suspends execution of the object
	virtual void Suspend(void) = 0;

	// Terminate
	//
	// Terminates execution of the object
	virtual void Terminate(int exitcode) = 0;

	//-------------------------------------------------------------------------
	// Properties

	// ExitCode
	//
	// Gets the current exit code, also resets the event handle
	__declspec(property(get=getExitCode)) int ExitCode;
	int getExitCode(void);

	// State
	//
	// Gets the current execution state
	__declspec(property(get=getState)) ExecutionState State;
	ExecutionState getState(void);

	// StateChanged
	//
	// Exposes the event handle that can be waited on for state changes
	__declspec(property(get=getStateChanged)) HANDLE StateChanged;
	HANDLE getStateChanged(void) const;

protected:

	// Instance Constructor
	//
	explicit Schedulable(ExecutionState state);

	//-------------------------------------------------------------------------
	// Protected Member Functions

	// MakeExitCode (static)
	//
	// Generates an exit code that can be used for the object
	static int MakeExitCode(int status, int signal, bool coredump);

	// Resumed
	//
	// Indicates that execution has resumed
	void Resumed(void);

	// Started
	//
	// Indicates that execution has started
	void Started(void);

	// Suspended
	//
	// Indicates that execution has been suspended
	void Suspended(void);

	// Terminated
	//
	// Indicates that execution has terminated
	virtual void Terminated(int exitcode);


private:

	Schedulable(const Schedulable&)=delete;
	Schedulable& operator=(const Schedulable&)=delete;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// ChangeState
	//
	// Changes the state of the object and signals the event
	void ChangeState(ExecutionState newstate, bool fireevent, int exitcode);

	//-------------------------------------------------------------------------
	// Member Variables

	ExecutionState		m_state;			// Current execution state
	std::mutex			m_statelock;		// Synchronization object
	HANDLE				m_statechanged;		// Waitable event object
	int					m_exitcode;			// Current object exit code
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SCHEDULABLE_H_
