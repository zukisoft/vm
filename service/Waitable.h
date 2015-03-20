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

#ifndef __WAITABLE_H_
#define __WAITABLE_H_
#pragma once

#include <concurrent_queue.h>
#include <mutex>
#include "Win32Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Waitable
//
// Base class for a waitable object (processes, threads)

class Waitable
{
public:

	// State
	//
	// Defines the waitable object state
	enum class State {

		NotSignaled		= 0,	// Object is not signaled
		Suspended,				// Object has been suspended
		Resumed,				// Object has been resumed
		Terminated,				// Object has been terminated
	};

	// Destructor
	//
	virtual ~Waitable();

	//-------------------------------------------------------------------------
	// Member Functions

	// PeekWaitableState
	//
	// Peeks the current state without resetting the signal
	State PeekWaitableState(void);

	// PopWaitableState
	//
	// Pops the current state and resets the signal
	State PopWaitableState(void);

	//-------------------------------------------------------------------------
	// Properties

	// StatusCode (abstract)
	//
	// Gets the status code for the waitable object
	__declspec(property(get=getStatusCode)) int StatusCode;
	virtual int getStatusCode(void) = 0;

	// WaitableStateChanged
	//
	// Exposes the event handle that can be waited on for state changes
	__declspec(property(get=getWaitableStateChanged)) HANDLE WaitableStateChanged;
	HANDLE getWaitableStateChanged(void) const;

protected:

	// Instance Constructor
	//
	Waitable();

	//-------------------------------------------------------------------------
	// Protected Member Functions

	// SetWaitableState
	//
	// Invoked to signal that a waitable state change occurred
	void SetWaitableState(State state);

private:

	Waitable(const Waitable&)=delete;
	Waitable& operator=(const Waitable&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	State				m_state;			// Current waitable state
	std::mutex			m_statelock;		// Synchronization object
	HANDLE				m_statechanged;		// Waitable event object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __WAITABLE_H_
