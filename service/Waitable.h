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

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Waitable
//
// Base class for a waitable object that provides the mechanism that allows an
// object to be waited upon for a state change.
//
// TODO: Consider moving this into Process, no other virtual machine objects
// should need this functionality.

class Waitable
{
public:

	// Destructor
	//
	virtual ~Waitable()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Wait
	//
	// Waits for a waitable instance to become signaled
	static std::shared_ptr<Waitable> Wait(std::shared_ptr<Waitable> object, int options, uapi::siginfo* siginfo);
	static std::shared_ptr<Waitable> Wait(std::vector<std::shared_ptr<Waitable>> const& objects, int options, uapi::siginfo* siginfo);

protected:

	// StateChange
	//
	// Indicates the type of state change that has occurred
	enum class StateChange 
	{
		Exited			= 1,		// Child has exited normally
		Killed			= 2,		// Child was killed by a signal
		Dumped			= 3,		// Child was killed and dumped
		Trapped			= 4,		// Traced child was trapped
		Stopped			= 5,		// Child was stopped by a signal
		Continued		= 6,		// Child was continued by SIGCONT
	};

	// Instance Constructor
	//
	Waitable();

	//-------------------------------------------------------------------------
	// Protected Member Functions

	// NotifyStateChange
	//
	// Signals that a change in waitable object state has occurred
	void NotifyStateChange(uapi::pid_t pid, StateChange newstate, int32_t status); 

private:

	Waitable(Waitable const&)=delete;
	Waitable& operator=(Waitable const&)=delete;
	
	// waiter_t
	//
	// Structure used to register a wait operation with a Waitable instance
	struct waiter_t 
	{
		std::condition_variable&			signal;			// Condition variable to signal
		std::mutex&							lock;			// Condition variable lock object
		int const							options;		// Wait operation mask and flags
		std::shared_ptr<Waitable> const&	object;			// Wait operation context object
		uapi::siginfo*						siginfo;		// Resultant signal information
		std::shared_ptr<Waitable>&			result;			// Resultant signaled object
	};

	//-------------------------------------------------------------------------
	// Private Member Functions

	// MaskAcceptsStateChange (static)
	//
	// Determines if a wait options mask accepts a specific StateChange code
	static bool MaskAcceptsStateChange(int mask, StateChange newstate);

	//-------------------------------------------------------------------------
	// Member Variables
	
	std::list<waiter_t>			m_waiters;		// Objects waiting for a state change
	uapi::siginfo				m_pending;		// Unprocessed state change signal
	std::mutex					m_lock;			// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __WAITABLE_H_
