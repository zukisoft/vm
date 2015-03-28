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
#include "SignalActions.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// SignalActions::Create (static)
//
// Creates a new empty signal action collection
//
// Arguments:
//
//	NONE

std::shared_ptr<SignalActions> SignalActions::Create(void)
{
	// Create a new SignalActions instance with an empty collection
	return std::make_shared<SignalActions>(std::move(action_map_t()));
}

//-----------------------------------------------------------------------------
// SignalActions::Duplicate (static)
//
// Duplicates an existing signal action collection
//
// Arguments:
//
//	existing		- The existing collection instance to be duplicated

std::shared_ptr<SignalActions> SignalActions::Duplicate(const std::shared_ptr<SignalActions>& existing)
{
	// Create a new SignalActions instance with a duplicate of the member collection
	return std::make_shared<SignalActions>(std::move(action_map_t(existing->m_actions)));
}

//-----------------------------------------------------------------------------
// SignalActions::Get
//
// Retrieves the stored action for the specified signal
//
// Arguments:
//
//	signal			- Signal for which to retrieve the action

uapi::sigaction SignalActions::Get(int signal)
{
	return m_actions[signal];			// Return existing value or a default
}

//-----------------------------------------------------------------------------
// SignalActions::Reset
//
// Resets the signal actions collection
//
// Arguments:
//
//	NONE

void SignalActions::Reset(void)
{
	// Iterate over all of the contained actions and set anything that isn't
	// currently being ignored back to a default action
	for(auto& iterator : m_actions)
		if(iterator.second.sa_handler != LINUX_SIG_IGN) iterator.second = action_t();
}

//-----------------------------------------------------------------------------
// SignalActions::Set
//
// Sets the action structure for a specified signal
//
// Arguments:
//
//	signal			- Signal for which to set the action
//	action			- Action to be assigned to the signal

void SignalActions::Set(int signal, uapi::sigaction action)
{
	m_actions[signal] = action;			// Add or update existing value
}

//-----------------------------------------------------------------------------
// SignalActions::Set
//
// Convenience version; sets the action structure for a specified signal and
// optionally retrieves the previous structure information
//
// Arguments:
//
//	signal			- Signal for which to set the action
//	action			- Action to be assigned to the signal
//	oldaction		- Optionally retrieves the previous signal action

void SignalActions::Set(int signal, const uapi::sigaction* action, uapi::sigaction* oldaction)
{
	// Locate the specified signal in the collection
	auto iterator = m_actions.find(signal);
	
	if(iterator == m_actions.end()) {

		// The signal does not already exist, return a default structure and insert it
		if(oldaction) *oldaction = action_t();
		if(action) m_actions.insert(std::make_pair(signal, *action));
	}

	else {

		// The signal action has already been defined; return as previous and replace it
		if(oldaction) *oldaction = iterator->second;
		if(action) iterator->second = *action;
	}
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
