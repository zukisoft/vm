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
// Abstract base class used by Process and Thread to provide a common interface
// for starting/suspending/resuming/terminating them.  Couldn't think of a better
// name at the time.

class Schedulable
{
public:

	// State
	//
	// Strongly typed enumeration defining the object state
	enum class State {

		Stopped		= 0,		// Object is suspended
		Running,				// Object is running
		Terminating,			// Object is terminating
		Terminated,				// Object has terminated
	};

	// Destructor
	//
	virtual ~Schedulable();

	//-------------------------------------------------------------------------
	// Member Functions

	// Resume
	//
	// Resumes the schedulable object
	virtual void Resume(void) = 0;

	// Start
	//
	// Starts the schedulable object
	virtual void Start(void) = 0;

	// Suspend
	//
	// Suspends the schedulable object
	virtual void Suspend(void) = 0;

	// Terminate
	//
	// Terminates the schedulable object
	virtual void Terminate(int exitcode) = 0;

	//-------------------------------------------------------------------------
	// Properties

	// CurrentState
	//
	// Gets the current state of the schedulable object
	__declspec(property(get=getCurrentState)) State CurrentState;
	State getCurrentState(void);

	// ExitCode
	//
	// Gets the exit code set for the schedulable object
	__declspec(property(get=getExitCode)) int ExitCode;
	int getExitCode(void);

	// StateChanged
	//
	// Exposes the event handle that can be waited on for state changes
	__declspec(property(get=getStateChanged)) HANDLE StateChanged;
	HANDLE getStateChanged(void) const;

protected:

	// Instance Constructor
	//
	explicit Schedulable(State state);

	//-------------------------------------------------------------------------
	// Protected Member Functions

	// MakeExitCode (static)
	//
	// Generates an exit code that can be used for the object
	static int MakeExitCode(int status, int signal, bool coredump);

	// Resumed
	//
	// Indicates that the schedulable object has resumed
	void Resumed(void);

	// Started
	//
	// Indicates that the schedulable object has started
	void Started(void);

	// Suspended
	//
	// Indicates that the schedulable object has been suspended
	void Suspended(void);

	// Terminated
	//
	// Indicates that the schedulable object has terminated
	virtual void Terminated(int exitcode);


private:

	Schedulable(const Schedulable&)=delete;
	Schedulable& operator=(const Schedulable&)=delete;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// ChangeState
	//
	// Changes the state of the object and signals the event
	void ChangeState(State newstate, bool fireevent, int exitcode);

	//-------------------------------------------------------------------------
	// Member Variables

	State				m_state;			// Current object state
	std::mutex			m_statelock;		// Synchronization object
	HANDLE				m_statechanged;		// Waitable event object
	int					m_exitcode;			// Schedulable object exit code
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SCHEDULABLE_H_
