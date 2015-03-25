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

#include <condition_variable>
#include <list>
#include <mutex>
#include <linux/siginfo.h>
#include <linux/signal.h>
#include <linux/wait.h>
#include "Win32Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Waitable
//
// Base class for a waitable object (process/thread) that provides the mechanism
// that allows the object to be waited upon for a state change
//
class Waitable
{
public:

	// State
	//
	// Indicates the type of state change that has occurred
	enum class State {

		Exited			= 1,		// Child has exited normally
		Killed			= 2,		// Child was killed by a signal
		Dumped			= 3,		// Child was killed and dumped
		Trapped			= 4,		// Traced child was trapped
		Stopped			= 5,		// Child was stopped by a signal
		Continued		= 6,		// Child was continued by SIGCONT
	};

	// Destructor
	//
	virtual ~Waitable()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Wait
	//
	// Waits for a waitable instance to become signaled
	static std::shared_ptr<Waitable> Wait(const std::vector<std::shared_ptr<Waitable>>& objects, int options, uapi::siginfo* siginfo);

protected:

	// Instance Constructor
	//
	Waitable();

	//-------------------------------------------------------------------------
	// Protected Member Functions

	// NotifyStateChange
	//
	// Signals that a change in waitable object state has occurred
	void NotifyStateChange(uapi::pid_t pid, State state, int32_t status); 

private:

	Waitable(const Waitable&)=delete;
	Waitable& operator=(const Waitable&)=delete;
	
	// waiter_t
	//
	// Structure used to register a wait operation with a Waitable instance
	struct waiter_t {

		// Instance Constructor
		//
		waiter_t(std::condition_variable& _signal, std::mutex& _lock, int _options,
			const std::shared_ptr<Waitable>& _object, uapi::siginfo* _siginfo, std::shared_ptr<Waitable>& _result) :
			signal(_signal), lock(_lock), options(_options), object(_object), siginfo(_siginfo), result(_result) {}

		// Fields
		//
		std::condition_variable&			signal;			// Condition variable to signal
		std::mutex&							lock;			// Condition variable lock object
		int const							options;		// Wait operation mask and flags
		const std::shared_ptr<Waitable>&	object;			// Wait operation context object
		uapi::siginfo*						siginfo;		// Resultant signal information
		std::shared_ptr<Waitable>&			result;			// Resultant signaled object

		waiter_t(const waiter_t&)=delete;
		waiter_t& operator=(const waiter_t&)=delete;
	};

	//-------------------------------------------------------------------------
	// Private Member Functions

	// MaskAcceptsState (static)
	//
	// Determines if a wait options mask accepts a specific State code
	static bool MaskAcceptsState(int mask, State state);

	//-------------------------------------------------------------------------
	// Member Variables
	
	std::list<waiter_t>			m_waiters;		// Objects waiting for a state change
	uapi::siginfo				m_pending;		// Unprocessed state change signal
	std::mutex					m_lock;			// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __WAITABLE_H_
