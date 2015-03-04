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

#ifndef __SIGNALACTIONS_H_
#define __SIGNALACTIONS_H_
#pragma once

#include <concurrent_unordered_map.h>
#include <memory>
#include <linux/signal.h>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// SignalActions
//
// Implements a collection of signal actions that can be duplicated or shared
// among multiple process instances

class SignalActions
{
public:

	// Destructor
	//
	~SignalActions()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Create (static)
	//
	// Creates a new default signal handler collection
	static std::shared_ptr<SignalActions> Create(void);

	// Duplicate (static)
	//
	// Duplicates the handle collection into a new instance
	static std::shared_ptr<SignalActions> Duplicate(const std::shared_ptr<SignalActions>& existing);

	// Get
	//
	// Retrieves the action structure defined for a signal
	uapi::sigaction Get(int signal);

	// Reset
	//
	// Resets the collection of signal actions
	void Reset(void);
	
	// Set
	//
	// Adds or updates the action structure for a signal
	void Set(int signal, uapi::sigaction action);
	void Set(int signal, const uapi::sigaction* action, uapi::sigaction* oldaction);

private:

	SignalActions(const SignalActions&)=delete;
	SignalActions& operator=(const SignalActions&)=delete;

	// action_t
	//
	// Private derivation of sigaction to provide constructor support
	struct action_t : public uapi::sigaction {

		// Default Constructor
		//
		action_t()
		{
			sa_handler = LINUX_SIG_DFL;
			sa_flags = 0;
			sa_restorer = nullptr;
			sa_mask = 0;
		}

		// Copy Constructor
		//
		action_t(const uapi::sigaction& rhs)
		{
			sa_handler = rhs.sa_handler;
			sa_flags = rhs.sa_flags;
			sa_restorer = rhs.sa_restorer;
			sa_mask = rhs.sa_mask;
		}
	};

	// action_map_t
	//
	// Collection of signal actions, keyed on the signal code.  This implementation
	// lends itself well to a lock-free map<> since values are never removed/erased
	using action_map_t = Concurrency::concurrent_unordered_map<int, action_t>;

	// Instance Constructors
	//
	explicit SignalActions(action_map_t&& actions) : m_actions(std::move(actions)) {}
	friend class std::_Ref_count_obj<SignalActions>;

	//-------------------------------------------------------------------------
	// Member Variables

	action_map_t				m_actions;			// Contained collection
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SIGNALACTIONS_H_
